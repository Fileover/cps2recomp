// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
#pragma once
// Online play (launcher::Online): the addresses another player can reach this machine at, for the Host row.

#include <string>
#include <vector>

namespace launcher {

struct LocalAddress {
    std::string address; // "192.168.1.5"
    std::string kind;    // "home network", "Tailscale", "VPN" or "" (a public address)
};

// This machine's IPv4 addresses (not the loopback), those on a home network first
std::vector<LocalAddress> local_addresses();

}  // namespace launcher
