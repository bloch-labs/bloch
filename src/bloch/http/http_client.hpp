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

#pragma once

#include <map>
#include <string>
#include <string_view>

namespace bloch::http {

    using Headers = std::map<std::string, std::string>;

    struct Request {
        std::string method = "GET";
        std::string url;
        Headers headers;
        std::string body;
        int timeout_ms = 5000;
    };

    struct Response {
        int status = 0;
        Headers headers;
        std::string body;

        [[nodiscard]] bool ok() const { return status >= 200 && status < 300; }
    };

    class Client {
       public:
        Client() = default;

        // Basic verbs; synchronous for now. Future work: async + streaming.
        [[nodiscard]] Response get(std::string_view url, int timeout_ms = 5000) const;
        [[nodiscard]] Response post(std::string_view url, const std::string& body,
                                    Headers headers = {}, int timeout_ms = 5000) const;
    };

}  // namespace bloch::http
