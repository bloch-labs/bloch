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

#include "bloch/http/http_client.hpp"

#include <utility>

namespace bloch::http {

    namespace {
        Response notImplemented(std::string url) {
            Response resp;
            resp.status = 501;
            resp.headers.emplace("X-Bloch-HTTP", "stub");
            resp.body = "HTTP client not yet implemented for " + std::move(url);
            return resp;
        }
    }  // namespace

    Response Client::get(std::string_view url, int timeout_ms) const {
        (void)timeout_ms;
        return notImplemented(std::string(url));
    }

    Response Client::post(std::string_view url, const std::string& body, Headers headers,
                          int timeout_ms) const {
        (void)body;
        (void)headers;
        (void)timeout_ms;
        return notImplemented(std::string(url));
    }

}  // namespace bloch::http
