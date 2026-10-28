// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
#include "online.h"

#include "process.h"

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#endif

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace launcher {

namespace {
// What an address is, by its range: private (a home network), 100.64.0.0/10 (Tailscale gives these; other carrier
// and VPN networks too), link-local and the loopback (left out: "-")
std::string Kind(uint32_t a, const std::string &name) {
    const uint8_t b0 = a >> 24, b1 = (a >> 16) & 0xFF;
    if (b0 == 127 || (b0 == 169 && b1 == 254) || a == 0) {
        return "-";
    }
    if (b0 == 100 && (b1 & 0xC0) == 64) {
        return name.find("tailscale") != std::string::npos || name.rfind("utun", 0) == 0 || name.find("Tailscale") != std::string::npos
                   ? "Tailscale"
                   : "VPN";
    }
    if (b0 == 10 || (b0 == 172 && (b1 & 0xF0) == 16) || (b0 == 192 && b1 == 168)) {
        return "home network";
    }
    return "";
}

// A network of this machine's own making (a virtual machine's or a container's): nobody else is on it
bool Virtual(const std::string &name) {
    for (const char *start : {"bridge", "docker", "br-", "veth", "virbr", "vmnet", "vboxnet", "vEthernet", "VirtualBox", "VMware", "Hyper-V"}) {
        if (name.rfind(start, 0) == 0) {
            return true;
        }
    }
    return false;
}

void Add(std::vector<LocalAddress> &list, uint32_t a, const std::string &name) {
    const std::string kind = Kind(a, name);
    if (kind == "-" || Virtual(name)) {
        return;
    }
    const std::string text = std::to_string(a >> 24) + "." + std::to_string((a >> 16) & 0xFF) + "." + std::to_string((a >> 8) & 0xFF) + "." +
                             std::to_string(a & 0xFF);
    if (std::none_of(list.begin(), list.end(), [&](const LocalAddress &l) { return l.address == text; })) {
        list.push_back({text, kind});
    }
}
}  // namespace

std::vector<LocalAddress> local_addresses() {
    std::vector<LocalAddress> list;
#ifdef _WIN32
    ULONG size = 16384;
    std::vector<uint8_t> buffer(size);
    auto *adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());
    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, nullptr, adapters, &size) ==
        ERROR_BUFFER_OVERFLOW) {
        buffer.resize(size);
        adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());
        if (GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, nullptr, adapters, &size) != NO_ERROR) {
            return list;
        }
    }
    for (auto *adapter = adapters; adapter; adapter = adapter->Next) {
        if (adapter->OperStatus != IfOperStatusUp) {
            continue;
        }
        std::string name;
        for (const wchar_t *c = adapter->Description; c && *c; c++) {
            name += static_cast<char>(*c < 128 ? *c : '?');
        }
        for (auto *unicast = adapter->FirstUnicastAddress; unicast; unicast = unicast->Next) {
            const auto *in = reinterpret_cast<const sockaddr_in *>(unicast->Address.lpSockaddr);
            Add(list, ntohl(in->sin_addr.s_addr), name);
        }
    }
#else
    ifaddrs *all = nullptr;
    if (getifaddrs(&all) != 0) {
        return list;
    }
    for (const ifaddrs *i = all; i; i = i->ifa_next) {
        if (i->ifa_addr && i->ifa_addr->sa_family == AF_INET && (i->ifa_flags & IFF_UP)) {
            Add(list, ntohl(reinterpret_cast<const sockaddr_in *>(i->ifa_addr)->sin_addr.s_addr), i->ifa_name ? i->ifa_name : "");
        }
    }
    freeifaddrs(all);
#endif
    std::stable_sort(list.begin(), list.end(), [](const LocalAddress &a, const LocalAddress &b) {
        const auto rank = [](const std::string &kind) { return kind == "home network" ? 0 : kind == "Tailscale" ? 1 : kind == "VPN" ? 2 : 3; };
        return rank(a.kind) < rank(b.kind);
    });
    return list;
}

bool fetch_lobby(const std::string& url, std::vector<OpenMatch>& matches) {
    std::string json;
    if (run_program(curl_program(), {"-sS", "--max-time", "8", url}, &json) != 0 || json.find("\"matches\"") == std::string::npos) {
        return false;
    }
    // {"matches":[{"code":"K7QM2X","name":"...","waiting":12}, ...]}: the service's answer (cps2recomp's online/)
    const auto text = [&](size_t from, const char* key, size_t end) {
        std::string out;
        const size_t at = json.find(std::string("\"") + key + "\":\"", from);
        if (at == std::string::npos || at > end) {
            return out;
        }
        for (size_t i = at + std::strlen(key) + 4; i < end && json[i] != '"'; i++) {
            if (json[i] == '\\' && i + 1 < end) {
                i++;
            }
            out += json[i];
        }
        return out;
    };
    matches.clear();
    for (size_t at = json.find('{', json.find('[')); at != std::string::npos;) {
        const size_t end = json.find('}', at);
        if (end == std::string::npos) {
            break;
        }
        OpenMatch match;
        match.game = text(at, "game", end);
        match.code = text(at, "code", end);
        match.name = text(at, "name", end);
        const size_t waiting = json.find("\"waiting\":", at);
        match.waiting = waiting != std::string::npos && waiting < end ? std::atoi(json.c_str() + waiting + 10) : 0;
        if (!match.code.empty()) {
            matches.push_back(match);
        }
        at = json.find('{', end);
    }
    return true;
}

}  // namespace launcher
