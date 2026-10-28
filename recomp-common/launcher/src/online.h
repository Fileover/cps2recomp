// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
#pragma once
// Online play (launcher::Online): the addresses another player can reach this machine at, for the Host row, and
// the lobby's list of open matches.

#include <string>
#include <vector>

namespace launcher {

struct LocalAddress {
    std::string address; // "203.0.113.5"
    std::string kind;    // "home network", "Tailscale", "VPN" or "" (a public address)
};

// This machine's IPv4 addresses (not the loopback), those on a home network first
std::vector<LocalAddress> local_addresses();

// A match waiting in the lobby (launcher::Online::lobby): its code, its host's name, and how long it has waited
struct OpenMatch {
    std::string code;
    std::string name;
    int waiting = 0; // (seconds)
};
// The lobby's list, fetched from `url` with the system's curl (blocks: for another thread); false if there was no
// answer
bool fetch_lobby(const std::string& url, std::vector<OpenMatch>& matches);

}  // namespace launcher
