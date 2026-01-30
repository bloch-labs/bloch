// Copyright 2025-2026 Akshay Pal (https://bloch-labs.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "bloch/update/update_manager.hpp"

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <openssl/evp.h>
#include "third_party/cpp-httplib/httplib.h"

namespace bloch::update {
    namespace {

        // REFACTOR: Wrap filesystem/HTTP/time dependencies behind interfaces
        // (Strategy) so we can unit-test update logic without hitting disk/network
        // and swap out curl/httplib or platform-specific cache paths cleanly.

        using Clock = std::chrono::system_clock;
        constexpr auto kUpdateWindow = std::chrono::hours(72);
        constexpr std::string_view kChangelogUrl =
            "https://github.com/bloch-labs/bloch/blob/master/CHANGELOG.md";

        struct UpdateCache {
            std::string latestVersion;
            Clock::time_point lastChecked{};
            Clock::time_point lastNotified{};
        };

        struct SemVer {
            int major{0};
            int minor{0};
            int patch{0};
            bool valid{false};
        };

        std::string userAgent(const std::string& currentVersion) {
            if (currentVersion.empty())
                return "bloch";
            return "bloch/" + currentVersion;
        }

        std::filesystem::path cacheFilePath() {
            if (const char* xdg = std::getenv("XDG_CACHE_HOME"); xdg && *xdg) {
                return std::filesystem::path(xdg) / "bloch" / "update_cache.txt";
            }
#ifdef _WIN32
            if (const char* local = std::getenv("LOCALAPPDATA"); local && *local) {
                return std::filesystem::path(local) / "Bloch" / "update_cache.txt";
            }
#endif
            if (const char* home = std::getenv("HOME"); home && *home) {
                return std::filesystem::path(home) / ".cache" / "bloch" / "update_cache.txt";
            }
            return std::filesystem::temp_directory_path() / "bloch-update-cache.txt";
        }

        UpdateCache emptyCache() {
            return UpdateCache{std::string(), Clock::time_point{}, Clock::time_point{}};
        }

        std::optional<UpdateCache> loadCache() {
            const auto path = cacheFilePath();
            std::ifstream in(path);
            if (!in)
                return std::nullopt;

            std::string checkedStr;
            std::string latest;
            std::string notifiedStr;
            if (!std::getline(in, checkedStr))
                return std::nullopt;
            if (!std::getline(in, latest))
                return std::nullopt;
            if (!std::getline(in, notifiedStr))
                notifiedStr = "0";

            try {
                auto checkedSec = std::stoll(checkedStr);
                auto notifiedSec = std::stoll(notifiedStr.empty() ? "0" : notifiedStr);
                UpdateCache cache;
                cache.latestVersion = latest;
                cache.lastChecked = Clock::time_point(std::chrono::seconds(checkedSec));
                cache.lastNotified = Clock::time_point(std::chrono::seconds(notifiedSec));
                return cache;
            } catch (...) {
                return std::nullopt;
            }
        }

        void saveCache(const UpdateCache& cache) {
            const auto path = cacheFilePath();
            std::error_code ec;
            std::filesystem::create_directories(path.parent_path(), ec);

            std::ofstream out(path, std::ios::trunc);
            if (!out)
                return;

            const auto toSeconds = [](Clock::time_point tp) {
                return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch())
                    .count();
            };

            out << toSeconds(cache.lastChecked) << "\n";
            out << cache.latestVersion << "\n";
            out << toSeconds(cache.lastNotified) << "\n";
        }

        SemVer parseSemVer(const std::string& version) {
            SemVer sem;
            if (version.empty())
                return sem;
            std::string v = version;
            if (!v.empty() && v.front() == 'v')
                v.erase(v.begin());

            size_t pos = 0;
            int idx = 0;
            while (pos < v.size() && idx < 3) {
                size_t start = pos;
                while (pos < v.size() && std::isdigit(static_cast<unsigned char>(v[pos]))) {
                    ++pos;
                }
                if (start == pos)
                    break;
                int value = std::stoi(v.substr(start, pos - start));
                if (idx == 0)
                    sem.major = value;
                else if (idx == 1)
                    sem.minor = value;
                else
                    sem.patch = value;
                sem.valid = true;
                ++idx;
                if (pos >= v.size() || v[pos] != '.')
                    break;
                ++pos;
            }
            return sem;
        }

        int compareSemVer(const SemVer& current, const SemVer& latest) {
            if (!current.valid || !latest.valid)
                return 0;
            if (current.major != latest.major)
                return current.major < latest.major ? -1 : 1;
            if (current.minor != latest.minor)
                return current.minor < latest.minor ? -1 : 1;
            if (current.patch != latest.patch)
                return current.patch < latest.patch ? -1 : 1;
            return 0;
        }

        std::string changeLabel(const SemVer& current, const SemVer& latest) {
            if (!current.valid || !latest.valid)
                return "new";
            if (latest.major > current.major)
                return "major";
            if (latest.minor > current.minor)
                return "minor";
            if (latest.patch > current.patch)
                return "patch";
            return "new";
        }

        bool hasExpired(const Clock::time_point& tp, Clock::time_point now) {
            return now - tp >= kUpdateWindow;
        }

        std::optional<std::string> extractTagName(const std::string& body) {
            const std::string needle = "\"tag_name\"";
            auto pos = body.find(needle);
            if (pos == std::string::npos)
                return std::nullopt;
            pos = body.find(':', pos);
            if (pos == std::string::npos)
                return std::nullopt;
            pos = body.find('"', pos);
            if (pos == std::string::npos)
                return std::nullopt;
            auto end = body.find('"', pos + 1);
            if (end == std::string::npos)
                return std::nullopt;
            return body.substr(pos + 1, end - pos - 1);
        }

        std::string describeError(httplib::Error err) {
            switch (err) {
                case httplib::Error::Success:
                    return "success";
                case httplib::Error::Unknown:
                    return "unknown error";
                case httplib::Error::Connection:
                    return "connection error";
                case httplib::Error::BindIPAddress:
                    return "bind error";
                case httplib::Error::Read:
                    return "read error";
                case httplib::Error::Write:
                    return "write error";
                case httplib::Error::ExceedRedirectCount:
                    return "too many redirects";
                case httplib::Error::Canceled:
                    return "request cancelled";
                case httplib::Error::SSLConnection:
                    return "ssl connection error";
                case httplib::Error::SSLLoadingCerts:
                    return "ssl certificate load error";
                case httplib::Error::SSLServerVerification:
                    return "ssl verification failed";
                case httplib::Error::UnsupportedMultipartBoundaryChars:
                    return "unsupported multipart boundary";
                default:
                    return "http client error";
            }
        }

        void configureClient(httplib::SSLClient& client) {
            client.set_follow_location(true);
            client.set_connection_timeout(2, 0);  // fast-fail on slow DNS/connect
            client.set_read_timeout(15, 0);
            client.set_write_timeout(15, 0);
        }

        std::optional<std::string> fetchLatestReleaseTag(const std::string& agent,
                                                         std::string& error) {
            httplib::SSLClient client("api.github.com");
            configureClient(client);
            httplib::Headers headers{
                {"User-Agent", agent},
                {"Accept", "application/vnd.github+json"},
            };
            auto res = client.Get("/repos/bloch-labs/bloch/releases/latest", headers);
            if (!res) {
                error = describeError(res.error());
                return std::nullopt;
            }
            if (res->status != 200) {
                error = "status " + std::to_string(res->status);
                return std::nullopt;
            }
            auto tag = extractTagName(res->body);
            if (!tag) {
                error = "missing tag_name";
                return std::nullopt;
            }
            return tag;
        }

        bool shouldSkipChecks() {
            return std::getenv("BLOCH_NO_UPDATE_CHECK") || std::getenv("CI") ||
                   std::getenv("BLOCH_OFFLINE");
        }

        bool maybePrintNotice(const std::string& latestVersion, const std::string& currentVersion,
                              Clock::time_point now, UpdateCache& cache) {
            if (latestVersion.empty())
                return false;
            if (!hasExpired(cache.lastNotified, now))
                return false;

            const auto current = parseSemVer(currentVersion);
            const auto latest = parseSemVer(latestVersion);
            if (!current.valid || !latest.valid)
                return false;
            if (compareSemVer(current, latest) >= 0)
                return false;

            const auto label = changeLabel(current, latest);
            std::cout << "There is a new " << label << " version of Bloch, " << latestVersion
                      << ". You currently have " << currentVersion
                      << ". To install the latest run bloch --update." << std::endl;
            cache.lastNotified = now;
            cache.latestVersion = latestVersion;
            return true;
        }

        std::filesystem::path resolveInstallPath(const std::string& argv0) {
#ifdef __linux__
            std::error_code ec;
            auto exePath = std::filesystem::read_symlink("/proc/self/exe", ec);
            if (!ec)
                return exePath;
#endif
            if (!argv0.empty()) {
                std::error_code ec;
                auto canonical = std::filesystem::canonical(argv0, ec);
                if (!ec)
                    return canonical;
                auto absolute = std::filesystem::absolute(argv0, ec);
                if (!ec)
                    return absolute;
            }
            if (const char* home = std::getenv("HOME"); home && *home) {
                return std::filesystem::path(home) / ".local" / "bin" / "bloch";
            }
            return std::filesystem::path("bloch");
        }

        bool downloadText(const std::string& host, const std::string& path,
                          const std::string& agent, std::string& out, std::string& error) {
            httplib::SSLClient client(host);
            configureClient(client);
            httplib::Headers headers{{"User-Agent", agent}};
            auto res = client.Get(path.c_str(), headers);
            if (!res) {
                error = describeError(res.error());
                return false;
            }
            if (res->status < 200 || res->status >= 300) {
                error = "status " + std::to_string(res->status);
                return false;
            }
            out = res->body;
            return true;
        }

        bool downloadFile(const std::string& host, const std::string& path,
                          const std::string& agent, const std::filesystem::path& dest,
                          std::string& error) {
            httplib::SSLClient client(host);
            configureClient(client);
            httplib::Headers headers{{"User-Agent", agent}};
            std::ofstream out(dest, std::ios::binary | std::ios::trunc);
            if (!out) {
                error = "cannot open download destination";
                return false;
            }

            auto res = client.Get(path.c_str(), headers, [&](const char* data, size_t len) {
                out.write(data, static_cast<std::streamsize>(len));
                return static_cast<bool>(out);
            });
            if (!out) {
                error = "failed while writing to disk";
                return false;
            }
            if (!res) {
                error = describeError(res.error());
                return false;
            }
            if (res->status < 200 || res->status >= 300) {
                error = "status " + std::to_string(res->status);
                return false;
            }
            return true;
        }

        std::string sha256File(const std::filesystem::path& file, std::string& error) {
            std::ifstream in(file, std::ios::binary);
            if (!in) {
                error = "unable to open " + file.string();
                return {};
            }

            std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(),
                                                                        EVP_MD_CTX_free);
            if (!ctx) {
                error = "unable to allocate digest context";
                return {};
            }
            if (EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1) {
                error = "unable to initialise digest context";
                return {};
            }

            char buffer[8192];
            while (in.good()) {
                in.read(buffer, sizeof(buffer));
                std::streamsize read = in.gcount();
                if (read > 0 &&
                    EVP_DigestUpdate(ctx.get(), buffer, static_cast<size_t>(read)) != 1) {
                    error = "failed hashing " + file.string();
                    return {};
                }
            }
            if (in.bad()) {
                error = "failed reading " + file.string();
                return {};
            }
            unsigned char hash[EVP_MAX_MD_SIZE];
            unsigned int hashLength = 0;
            if (EVP_DigestFinal_ex(ctx.get(), hash, &hashLength) != 1) {
                error = "failed finalising digest";
                return {};
            }
            std::ostringstream oss;
            oss << std::hex << std::setfill('0');
            for (unsigned int i = 0; i < hashLength; ++i) {
                oss << std::setw(2) << static_cast<int>(hash[i]);
            }
            return oss.str();
        }

        std::optional<std::string> parseChecksum(const std::string& content,
                                                 const std::string& assetName) {
            std::istringstream in(content);
            std::string line;
            while (std::getline(in, line)) {
                if (line.find(assetName) == std::string::npos)
                    continue;
                std::istringstream parts(line);
                std::string hash;
                if (parts >> hash)
                    return hash;
            }
            return std::nullopt;
        }

        bool extractArchive(const std::filesystem::path& archive, const std::filesystem::path& dest,
                            std::string& error) {
            std::error_code ec;
            std::filesystem::create_directories(dest, ec);
            std::ostringstream cmd;
            cmd << "tar -xzf \"" << archive.string() << "\" -C \"" << dest.string() << "\"";
            int rc = std::system(cmd.str().c_str());
            if (rc != 0) {
                error = "tar exited with code " + std::to_string(rc);
                return false;
            }
            return true;
        }

        std::optional<std::filesystem::path> findBinary(const std::filesystem::path& root) {
            std::error_code ec;
            for (auto it = std::filesystem::recursive_directory_iterator(root, ec);
                 it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
                if (ec)
                    break;
                if (it->is_regular_file() && it->path().filename() == "bloch") {
                    return it->path();
                }
            }
            return std::nullopt;
        }

        std::string osLabel() {
#if defined(__APPLE__)
            return "macOS";
#elif defined(_WIN32)
            return "Windows";
#else
            return "Linux";
#endif
        }

        std::string archLabel() {
#if defined(__x86_64__) || defined(_M_X64)
            return "X64";
#elif defined(__aarch64__) || defined(_M_ARM64)
            return "ARM64";
#else
            return {};
#endif
        }

        bool hasLatest(const std::string& currentVersion, const std::string& latestVersion) {
            const auto current = parseSemVer(currentVersion);
            const auto latest = parseSemVer(latestVersion);
            if (!current.valid || !latest.valid)
                return false;
            return compareSemVer(current, latest) >= 0;
        }

        struct TempDirGuard {
            std::filesystem::path path;
            explicit TempDirGuard(std::filesystem::path p) : path(std::move(p)) {}
            ~TempDirGuard() {
                if (path.empty())
                    return;
                std::error_code ec;
                std::filesystem::remove_all(path, ec);
            }
        };

    }  // namespace

    void checkForUpdatesIfDue(const std::string& currentVersion) {
        if (shouldSkipChecks())
            return;

        const auto now = Clock::now();
        auto cached = loadCache();
        UpdateCache cache = cached.value_or(emptyCache());

        if (cached && hasExpired(cache.lastChecked, now) == false) {
            if (!cache.latestVersion.empty() &&
                maybePrintNotice(cache.latestVersion, currentVersion, now, cache)) {
                saveCache(cache);
            }
            return;
        }

        // Use any cached version info before hitting the network.
        if (cached && !cache.latestVersion.empty()) {
            if (maybePrintNotice(cache.latestVersion, currentVersion, now, cache))
                saveCache(cache);
        }

        std::string err;
        auto latest = fetchLatestReleaseTag(userAgent(currentVersion), err);
        if (!latest)
            return;

        cache.latestVersion = *latest;
        cache.lastChecked = now;
        maybePrintNotice(*latest, currentVersion, now, cache);
        saveCache(cache);
    }

    bool performSelfUpdate(const std::string& currentVersion, const std::string& argv0) {
        std::string err;
        auto latest = fetchLatestReleaseTag(userAgent(currentVersion), err);
        if (!latest) {
            std::cerr << "Unable to resolve latest Bloch release from GitHub (" << err << ")."
                      << std::endl;
            return false;
        }
        if (hasLatest(currentVersion, *latest)) {
            std::cout << "You already have the latest Bloch release (" << *latest << ")."
                      << std::endl;
            return true;
        }

        const auto currentSem = parseSemVer(currentVersion);
        const auto latestSem = parseSemVer(*latest);
        if (currentSem.valid && latestSem.valid && latestSem.major > currentSem.major) {
            std::cout << "A major Bloch update is available (" << currentVersion << " -> "
                      << *latest << "). Review changes: " << kChangelogUrl
                      << "\nProceed with the update? [y/N]: " << std::flush;
            std::string answer;
            std::getline(std::cin, answer);
            if (!(answer == "y" || answer == "Y" || answer == "yes" || answer == "YES")) {
                std::cout << "Update cancelled." << std::endl;
                return false;
            }
        }

#ifdef _WIN32
        std::ostringstream cmd;
        cmd << "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
            << "$ErrorActionPreference='Stop';"
            << "$url='https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1';"
            << "$out=Join-Path $env:TEMP 'bloch-update.ps1';"
            << "Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $out;"
            << "$ver='" << *latest << "';"
            << "& $out -Version $ver;"
            << "\"";
        int rc = std::system(cmd.str().c_str());
        if (rc != 0) {
            std::cerr << "Failed to run Windows updater (exit code " << rc
                      << "). Try reinstalling via the latest PowerShell installer." << std::endl;
            return false;
        }

        UpdateCache cache = emptyCache();
        cache.latestVersion = *latest;
        cache.lastChecked = Clock::now();
        cache.lastNotified = cache.lastChecked;
        saveCache(cache);

        std::cout << "Bloch updated to " << *latest << " via the PowerShell installer script."
                  << std::endl;
        return true;
#else

        const auto os = osLabel();
        const auto arch = archLabel();
        if (os.empty() || arch.empty()) {
            std::cerr << "Unsupported platform for self-update (os=" << os << ", arch=" << arch
                      << ")." << std::endl;
            return false;
        }

        const auto assetName = "bloch-" + *latest + "-" + os + "-" + arch + ".tar.gz";
        const auto basePath = "/bloch-labs/bloch/releases/download/" + *latest;  // host: github.com

        const auto tempDir =
            std::filesystem::temp_directory_path() /
            ("bloch-update-" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                  Clock::now().time_since_epoch())
                                                  .count()));
        TempDirGuard cleanup(tempDir);
        std::error_code ec;
        std::filesystem::create_directories(tempDir, ec);
        const auto archivePath = tempDir / assetName;

        if (!downloadFile("github.com", basePath + "/" + assetName, userAgent(currentVersion),
                          archivePath, err)) {
            std::cerr << "Failed to download " << assetName << " (" << err << ")." << std::endl;
            return false;
        }

        std::string checksums;
        if (downloadText("github.com", basePath + "/checksums.txt", userAgent(currentVersion),
                         checksums, err)) {
            auto expected = parseChecksum(checksums, assetName);
            if (expected) {
                std::string hashErr;
                auto actual = sha256File(archivePath, hashErr);
                if (actual.empty()) {
                    std::cerr << "Failed to compute checksum: " << hashErr << std::endl;
                    return false;
                }
                if (*expected != actual) {
                    std::cerr << "Checksum mismatch for " << assetName << std::endl;
                    return false;
                }
            }
        }

        if (!extractArchive(archivePath, tempDir, err)) {
            std::cerr << "Failed to extract archive: " << err << std::endl;
            return false;
        }

        auto newBinary = findBinary(tempDir);
        if (!newBinary) {
            std::cerr << "Unable to locate the bloch binary inside the downloaded archive."
                      << std::endl;
            return false;
        }

        const auto installPath = resolveInstallPath(argv0);
        std::filesystem::create_directories(installPath.parent_path(), ec);
        std::filesystem::permissions(
            *newBinary,
            std::filesystem::perms::owner_read | std::filesystem::perms::owner_write |
                std::filesystem::perms::owner_exec | std::filesystem::perms::group_read |
                std::filesystem::perms::group_exec | std::filesystem::perms::others_read |
                std::filesystem::perms::others_exec,
            std::filesystem::perm_options::add, ec);

        std::filesystem::copy_file(*newBinary, installPath,
                                   std::filesystem::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cerr << "Failed to install the new binary to " << installPath << ": "
                      << ec.message() << std::endl;
            return false;
        }

        UpdateCache cache = emptyCache();
        cache.latestVersion = *latest;
        cache.lastChecked = Clock::now();
        cache.lastNotified = cache.lastChecked;
        saveCache(cache);

        std::cout << "Bloch updated to " << *latest << " at " << installPath << std::endl;
        return true;
#endif
    }

}  // namespace bloch::update
