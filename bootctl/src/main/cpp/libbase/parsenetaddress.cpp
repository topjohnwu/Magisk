/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "android-base/parsenetaddress.h"

#include <algorithm>

#include "android-base/stringprintf.h"
#include "android-base/strings.h"

namespace android {
namespace base {

bool ParseNetAddress(const std::string& address, std::string* host, int* port,
                     std::string* canonical_address, std::string* error) {
  host->clear();

  bool ipv6 = true;
  bool saw_port = false;
  size_t colons = std::count(address.begin(), address.end(), ':');
  size_t dots = std::count(address.begin(), address.end(), '.');
  std::string port_str;
  if (address[0] == '[') {
    // [::1]:123
    if (address.rfind("]:") == std::string::npos) {
      *error = StringPrintf("bad IPv6 address '%s'", address.c_str());
      return false;
    }
    *host = address.substr(1, (address.find("]:") - 1));
    port_str = address.substr(address.rfind("]:") + 2);
    saw_port = true;
  } else if (dots == 0 && colons >= 2 && colons <= 7) {
    // ::1
    *host = address;
  } else if (colons <= 1) {
    // 1.2.3.4 or some.accidental.domain.com
    ipv6 = false;
    std::vector<std::string> pieces = Split(address, ":");
    *host = pieces[0];
    if (pieces.size() > 1) {
      port_str = pieces[1];
      saw_port = true;
    }
  }

  if (host->empty()) {
    *error = StringPrintf("no host in '%s'", address.c_str());
    return false;
  }

  if (saw_port) {
    if (sscanf(port_str.c_str(), "%d", port) != 1 || *port <= 0 ||
        *port > 65535) {
      *error = StringPrintf("bad port number '%s' in '%s'", port_str.c_str(),
                            address.c_str());
      return false;
    }
  }

  if (canonical_address != nullptr) {
    *canonical_address =
        StringPrintf(ipv6 ? "[%s]:%d" : "%s:%d", host->c_str(), *port);
  }

  return true;
}

}  // namespace base
}  // namespace android
