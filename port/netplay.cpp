// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// Online play: the socket, the handshake, and rollback over the network (netplay.h)
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "netplay.h"

#include "game_info.h"
#include "../runtime/game.h"
#include "port.h"

#include <zlib.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <thread>

using namespace cps2recomp;

namespace port {

namespace {
    uint64_t NowMs() {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    // The packets: "C2NP", what kind, then the kind's fields, little-endian
    constexpr uint8_t kMagic[4] = {'C', '2', 'N', 'P'};
    constexpr uint16_t kProtocol = 1;
    enum Kind : uint8_t {
        kHello = 1,   // joining -> host: the protocol, the game and its build
        kWelcome = 2, // host -> joining: the delay, the board settings, the features that change play
        kRefuse = 3,  // either: why not (and the match is over)
        kControls = 4, // either: controls, and the rest of what each frame says (Netplay::SendInputs)
        kBye = 5,     // either: this side has left
    };

    struct Writer {
        std::vector<uint8_t> out;
        explicit Writer(Kind kind) {
            out.assign(kMagic, kMagic + 4);
            out.push_back(kind);
        }
        void U8(uint8_t v) { out.push_back(v); }
        void U16(uint16_t v) { U8(static_cast<uint8_t>(v)); U8(static_cast<uint8_t>(v >> 8)); }
        void U32(uint32_t v) { U16(static_cast<uint16_t>(v)); U16(static_cast<uint16_t>(v >> 16)); }
        void Text(const std::string &s) {
            U16(static_cast<uint16_t>(s.size()));
            out.insert(out.end(), s.begin(), s.end());
        }
        void Bytes(const void *p, size_t n) {
            U16(static_cast<uint16_t>(n));
            out.insert(out.end(), static_cast<const uint8_t *>(p), static_cast<const uint8_t *>(p) + n);
        }
    };
    struct Reader {
        const uint8_t *at, *end;
        bool ok = true;
        Reader(const uint8_t *data, size_t size) : at(data + 5), end(data + size) {}
        bool Has(size_t n) {
            ok = ok && static_cast<size_t>(end - at) >= n;
            return ok;
        }
        uint8_t U8() { return Has(1) ? *at++ : 0; }
        uint16_t U16() { const uint16_t lo = U8(); return static_cast<uint16_t>(lo | U8() << 8); }
        uint32_t U32() { const uint32_t lo = U16(); return lo | static_cast<uint32_t>(U16()) << 16; }
        std::string Text() {
            const size_t n = U16();
            if (!Has(n)) {
                return "";
            }
            std::string s(reinterpret_cast<const char *>(at), n);
            at += n;
            return s;
        }
    };

    // Which features the joining side takes from the host: those that change how the game plays. Each player's
    // own: how the picture is shown and the sound. Off for both: the rest (the alternative modes, training, the
    // viewers: kept apart from the save state, or not worked out for two machines yet), and run-ahead (rollback
    // does what it does).
    enum class Share { Host, Own, Off, Keep };
    Share Shared(const Feature &feature) {
        const std::string category = feature.category, name = feature.name;
        if (name == "run_ahead" || name == "custom_colours") {
            return Share::Off; // (custom colours: each player's own library, which the other side has not)
        }
        if (name.rfind("widescreen", 0) == 0 || category == "Game" || category == "Board" || category == "Cheats") {
            return Share::Host;
        }
        if (category == "Display" || category == "Sound") {
            return Share::Own;
        }
        if (feature.flags & kFeatureOwn) {
            return Share::Keep; // (a record or a choice kept for a mode: left as it is, and the mode is off)
        }
        return Share::Off;
    }

    // What both sides must have the same of: the game, and the program as this build reads it
    uint32_t BuildHash(const Port &port) {
        uLong h = crc32(0, nullptr, 0);
        h = crc32(h, port.roms.opcodes.data(), static_cast<uInt>(port.roms.opcodes.size()));
        h = crc32(h, port.roms.soundProgram.data(), static_cast<uInt>(port.roms.soundProgram.size()));
        return static_cast<uint32_t>(h);
    }
} // namespace

// The socket

#ifdef _WIN32
using SocketHandle = SOCKET;
#else
using SocketHandle = int;
#define closesocket close
#endif

UdpSocket::~UdpSocket() {
    if (handle != -1) {
        closesocket(static_cast<SocketHandle>(handle));
    }
}

bool UdpSocket::Open(int port, std::string &error) {
#ifdef _WIN32
    static bool started = false;
    if (!started) {
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            error = "cannot start Windows sockets";
            return false;
        }
        started = true;
    }
#endif
    // (IPv6 with IPv4 too, where the system has it; or IPv4)
    SocketHandle s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    six = true;
    if (s != static_cast<SocketHandle>(-1)) {
        int off = 0;
        setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char *>(&off), sizeof(off));
        sockaddr_in6 any{};
        any.sin6_family = AF_INET6;
        any.sin6_port = htons(static_cast<uint16_t>(port));
        any.sin6_addr = in6addr_any;
        if (bind(s, reinterpret_cast<sockaddr *>(&any), sizeof(any)) != 0) {
            closesocket(s);
            s = static_cast<SocketHandle>(-1);
        }
    }
    if (s == static_cast<SocketHandle>(-1)) {
        six = false;
        s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        sockaddr_in any{};
        any.sin_family = AF_INET;
        any.sin_port = htons(static_cast<uint16_t>(port));
        any.sin_addr.s_addr = htonl(INADDR_ANY);
        if (s == static_cast<SocketHandle>(-1) || bind(s, reinterpret_cast<sockaddr *>(&any), sizeof(any)) != 0) {
            error = "cannot use UDP port " + std::to_string(port) + " (is another copy of the game using it?)";
            if (s != static_cast<SocketHandle>(-1)) {
                closesocket(s);
            }
            return false;
        }
    }
#ifdef _WIN32
    u_long nonBlocking = 1;
    ioctlsocket(s, FIONBIO, &nonBlocking);
#else
    fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
#endif
    handle = static_cast<intptr_t>(s);
    return true;
}

bool UdpSocket::Resolve(const std::string &host, int port, NetAddress &address, std::string &error) {
    addrinfo hints{}, *found = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &found) != 0 || !found) {
        error = "cannot find " + host;
        return false;
    }
    // (an IPv4 address as IPv6 sees it, for the socket that is both)
    const addrinfo *use = found;
    if (use->ai_family == AF_INET) {
        sockaddr_in6 mapped{};
        mapped.sin6_family = AF_INET6;
        mapped.sin6_port = reinterpret_cast<const sockaddr_in *>(use->ai_addr)->sin_port;
        mapped.sin6_addr.s6_addr[10] = 0xFF;
        mapped.sin6_addr.s6_addr[11] = 0xFF;
        memcpy(&mapped.sin6_addr.s6_addr[12], &reinterpret_cast<const sockaddr_in *>(use->ai_addr)->sin_addr, 4);
        memcpy(address.storage.data(), &mapped, sizeof(mapped));
        address.length = sizeof(mapped);
        memcpy(address.four.data(), use->ai_addr, sizeof(sockaddr_in));
        address.hasFour = true;
    } else {
        memcpy(address.storage.data(), use->ai_addr, use->ai_addrlen);
        address.length = static_cast<int>(use->ai_addrlen);
    }
    freeaddrinfo(found);
    return true;
}

void UdpSocket::Send(const NetAddress &to, const uint8_t *data, size_t size) {
    const bool asFour = !six && to.hasFour;
    sendto(static_cast<SocketHandle>(handle), reinterpret_cast<const char *>(data), static_cast<int>(size), 0,
           reinterpret_cast<const sockaddr *>(asFour ? to.four.data() : to.storage.data()),
           static_cast<socklen_t>(asFour ? sizeof(sockaddr_in) : to.length));
}

int UdpSocket::Receive(uint8_t *data, size_t size, NetAddress &from) {
    sockaddr_storage address{};
    socklen_t length = sizeof(address);
    const int got = static_cast<int>(recvfrom(static_cast<SocketHandle>(handle), reinterpret_cast<char *>(data), static_cast<int>(size), 0,
                                              reinterpret_cast<sockaddr *>(&address), &length));
    if (got < 0) {
        return -1;
    }
    from = NetAddress{};
    memcpy(from.storage.data(), &address, std::min<size_t>(length, sizeof(from.storage)));
    from.length = static_cast<int>(length);
    return got;
}

// The session

void Netplay::Send(const std::vector<uint8_t> &packet) {
    if (settings.fakeLoss && static_cast<int>((random = random * 6364136223846793005ull + 1442695040888963407ull) >> 33) % 100 < settings.fakeLoss) {
        return;
    }
    if (settings.fakeLag || settings.fakeJitter) {
        random = random * 6364136223846793005ull + 1442695040888963407ull;
        const int jitter = settings.fakeJitter ? static_cast<int>((random >> 33) % (2 * settings.fakeJitter + 1)) - settings.fakeJitter : 0;
        late.push_back({NowMs() + static_cast<uint64_t>(std::max(0, settings.fakeLag + jitter)), packet});
        return;
    }
    socket.Send(peer, packet.data(), packet.size());
}

bool Netplay::Connect(Port &port, std::string &error) {
    if (!socket.Open(settings.hosting ? settings.port : 0, error)) {
        return false;
    }
    local = settings.hosting ? 0 : 1;
    remote = 1 - local;
    const uint32_t build = BuildHash(port);
    std::vector<uint8_t> packet(2048);
    NetAddress from;
    // The features as this match has them: the joining side takes the host's (Share::Host); both turn off the rest
    const auto settle = [&](const std::map<std::string, int> &host) {
        for (const Feature &feature : TheGame().features) {
            switch (Shared(feature)) {
            case Share::Host:
                if (!settings.hosting) {
                    const auto it = host.find(feature.name);
                    SetFeatureForNow(feature.name, it == host.end() ? 0 : it->second);
                }
                break;
            case Share::Off: SetFeatureForNow(feature.name, 0); break;
            default: break;
            }
        }
    };
    const uint64_t began = NowMs();
    // (the first `delay` frames: nobody's controls)
    const auto start = [&](int frames) {
        delay = std::clamp(frames, 0, kMostAhead);
        localUpTo = remoteUpTo = static_cast<uint64_t>(delay);
    };
    if (settings.hosting) {
        start(settings.delay);
        fprintf(stderr, "online: waiting for the other player on UDP port %d\n", settings.port);
        std::map<std::string, int> none;
        settle(none);
        Writer welcome(kWelcome);
        welcome.U16(kProtocol);
        welcome.U8(static_cast<uint8_t>(delay));
        welcome.Bytes(port.machine->eeprom.words, sizeof(port.machine->eeprom.words));
        std::string features;
        for (const Feature &feature : TheGame().features) {
            if (Shared(feature) == Share::Host && FeatureSetting(feature.name)) {
                features += std::string(feature.name) + "=" + std::to_string(FeatureSetting(feature.name)) + ",";
            }
        }
        welcome.Text(features);
        bool found = false;
        uint64_t sent = 0;
        while (true) {
            const int got = socket.Receive(packet.data(), packet.size(), from);
            if (got >= 5 && !memcmp(packet.data(), kMagic, 4)) {
                Reader in(packet.data(), static_cast<size_t>(got));
                if (packet[4] == kHello && (!found || from == peer)) {
                    const uint16_t protocol = in.U16();
                    const std::string game = in.Text();
                    const uint32_t theirs = in.U32();
                    std::string why;
                    if (protocol != kProtocol) {
                        why = "The two copies of the game are different versions: update both.";
                    } else if (game != GAME_ID) {
                        why = "The other player started " + game + ", this side " GAME_ID ".";
                    } else if (theirs != build) {
                        why = "The two copies of the game are different builds: use the same release on both.";
                    }
                    if (!why.empty()) {
                        Writer refuse(kRefuse);
                        refuse.Text(why);
                        socket.Send(from, refuse.out.data(), refuse.out.size());
                        fprintf(stderr, "online: turned away a player: %s\n", why.c_str());
                        continue;
                    }
                    if (!found) {
                        peer = from;
                        found = true;
                        fprintf(stderr, "online: the other player is here\n");
                    }
                    Send(welcome.out);
                    sent = NowMs();
                } else if (found && from == peer && packet[4] == kControls) {
                    Take(packet.data(), static_cast<size_t>(got)); // (they have the welcome: the match is on)
                    break;
                } else if (found && from == peer && packet[4] == kBye) {
                    found = false; // (they left before it began)
                }
            } else if (got < 0) {
                if (found && NowMs() - sent > 250) {
                    Send(welcome.out);
                    sent = NowMs();
                }
                Pump();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
    } else {
        // The host's address, and the welcome: the hello again every quarter second until it comes (for 30 seconds)
        std::string host = settings.address;
        int hostPort = settings.port;
        const size_t colon = host.rfind(':');
        if (colon != std::string::npos && host.find(':') == colon) { // (ADDRESS:PORT; an IPv6 address alone has several)
            hostPort = atoi(host.c_str() + colon + 1);
            host = host.substr(0, colon);
        } else if (host.size() > 2 && host[0] == '[') { // ([IPv6]:PORT)
            const size_t close = host.find(']');
            if (close != std::string::npos && close + 1 < host.size() && host[close + 1] == ':') {
                hostPort = atoi(host.c_str() + close + 2);
            }
            host = host.substr(1, close == std::string::npos ? std::string::npos : close - 1);
        }
        if (!UdpSocket::Resolve(host, hostPort, peer, error)) {
            return false;
        }
        fprintf(stderr, "online: joining %s, UDP port %d\n", host.c_str(), hostPort);
        Writer hello(kHello);
        hello.U16(kProtocol);
        hello.Text(GAME_ID);
        hello.U32(build);
        uint64_t sent = 0;
        bool welcomed = false;
        while (!welcomed) {
            if (NowMs() - began > 30000) {
                error = "The host did not answer (is it waiting, and is UDP port " + std::to_string(hostPort) + " open to you?).";
                return false;
            }
            if (NowMs() - sent > 250) {
                Send(hello.out);
                sent = NowMs();
            }
            Pump();
            const int got = socket.Receive(packet.data(), packet.size(), from);
            if (got < 5 || memcmp(packet.data(), kMagic, 4)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }
            Reader in(packet.data(), static_cast<size_t>(got));
            if (packet[4] == kRefuse) {
                error = in.Text();
                return false;
            }
            if (packet[4] != kWelcome) {
                continue;
            }
            peer = from;
            if (in.U16() != kProtocol) {
                error = "The two copies of the game are different versions: update both.";
                return false;
            }
            start(in.U8());
            const size_t words = in.U16();
            if (!in.Has(words) || words != sizeof(port.machine->eeprom.words)) {
                error = "The host's welcome did not make sense.";
                return false;
            }
            memcpy(port.machine->eeprom.words, in.at, words); // (the host's board settings, for this match only)
            in.at += words;
            std::map<std::string, int> host;
            const std::string features = in.Text();
            for (size_t at = 0; at < features.size();) {
                const size_t comma = std::min(features.find(',', at), features.size());
                const std::string item = features.substr(at, comma - at);
                const size_t eq = item.find('=');
                if (eq != std::string::npos) {
                    host[item.substr(0, eq)] = atoi(item.c_str() + eq + 1);
                }
                at = comma + 1;
            }
            settle(host);
            welcomed = true;
        }
    }
    lastHeard = NowMs();
    fprintf(stderr, "online: the match is on: you are player %d (%d frame%s of delay)\n", local + 1, delay, delay == 1 ? "" : "s");
    return true;
}

void Netplay::SendInputs() {
    Writer out(kControls);
    out.U32(static_cast<uint32_t>(current));
    out.U32(static_cast<uint32_t>(remoteUpTo));
    const uint64_t first = std::max<uint64_t>(remoteAck, localUpTo > 64 ? localUpTo - 64 : 0);
    out.U32(static_cast<uint32_t>(first));
    out.U8(static_cast<uint8_t>(localUpTo - first));
    for (uint64_t at = first; at < localUpTo; at++) {
        out.U16(localInput[at % kInputs]);
    }
    out.U16(static_cast<uint16_t>(static_cast<int16_t>(std::lround(std::clamp(ahead, -100.0, 100.0) * 100))));
    out.U32(static_cast<uint32_t>(lastHashFrame));
    out.U32(lastHash);
    const uint64_t now = NowMs();
    out.U32(static_cast<uint32_t>(now));
    out.U32(echo);
    out.U32(static_cast<uint32_t>(echoAt ? now - echoAt : 0));
    Send(out.out);
}

void Netplay::Take(const uint8_t *data, size_t size) {
    Reader in(data, size);
    if (data[4] == kBye || data[4] == kRefuse) {
        if (ended.empty()) { // (the first reason is the one: a desync is followed by a goodbye)
            ended = data[4] == kBye ? "The other player left." : in.Text();
            failed = data[4] == kRefuse;
        }
        return;
    }
    if (data[4] != kControls) {
        return;
    }
    const uint64_t frame = in.U32(), ack = in.U32(), first = in.U32();
    const unsigned count = in.U8();
    uint16_t inputs[256];
    for (unsigned n = 0; n < count; n++) {
        inputs[n] = in.U16();
    }
    const double theirs = static_cast<int16_t>(in.U16()) / 100.0;
    const uint64_t hashFrame = in.U32();
    const uint32_t hash = in.U32();
    const uint32_t sentAt = in.U32(), echoed = in.U32(), held = in.U32();
    if (!in.ok) {
        return;
    }
    lastHeard = NowMs();
    remoteFrame = std::max(remoteFrame, frame);
    remoteAck = std::max(remoteAck, ack);
    theirAhead = theirs;
    for (uint64_t at = remoteUpTo; at < first + count; at++) {
        if (at >= first && at < confirmed + kInputs) {
            remoteInput[at % kInputs] = inputs[at - first];
            remoteUpTo = at + 1;
        }
    }
    if (hashFrame) {
        theirHashes[hashFrame] = hash;
    }
    echo = sentAt;
    echoAt = NowMs();
    if (echoed) {
        const double rtt = static_cast<double>(static_cast<uint32_t>(static_cast<uint32_t>(NowMs()) - echoed - held));
        if (rtt < 10000) {
            ping = ping ? ping * 0.9 + rtt * 0.1 : rtt;
        }
    }
}

void Netplay::Pump() {
    const uint64_t now = NowMs();
    while (!late.empty() && late.front().due <= now) {
        socket.Send(peer, late.front().data.data(), late.front().data.size());
        late.pop_front();
    }
}

bool Netplay::Ready(uint64_t frame) {
    Pump();
    uint8_t packet[2048];
    NetAddress from;
    int got;
    while ((got = socket.Receive(packet, sizeof(packet), from)) >= 0) {
        if (got >= 5 && !memcmp(packet, kMagic, 4) && from == peer) {
            if (packet[4] == kWelcome && local == 1) {
                continue; // (the host had not heard yet: our controls tell it)
            }
            Take(packet, static_cast<size_t>(got));
        }
    }
    Compare();
    if (ended.empty() && NowMs() - lastHeard > 5000) {
        ended = "Lost the connection to the other player.";
        failed = true;
    }
    if (!ended.empty()) {
        return false;
    }
    current = frame;
    // Too far ahead of the other side's controls: wait for them
    if (frame >= remoteUpTo + kMostAhead) {
        waits++;
        SendInputs();
        return false;
    }
    // Ahead of the other side in time (each side's own idea of how far ahead it is, compared): a tick waited now
    // and then, until even
    if (frame % 60 == 0 && frame != waitedAt && waitTicks == 0) {
        const double by = (ahead - theirAhead) / 2;
        waitTicks = by >= 1 ? std::min(static_cast<int>(by), 8) : 0;
    }
    if (waitTicks > 0 && frame != waitedAt) {
        waitedAt = frame;
        waitTicks--;
        waits++;
        SendInputs();
        return false;
    }
    return true;
}

void Netplay::Run(Port &port, uint64_t at, bool drawing) {
    Kept &k = kept[at % kKept];
    uint16_t players[Machine::kMaxPlayers] = {};
    players[local] = localInput[at % kInputs];
    // (the other side's, or guessed as the last that came)
    k.used = at < remoteUpTo ? remoteInput[at % kInputs] : remoteUpTo ? remoteInput[(remoteUpTo - 1) % kInputs] : 0;
    players[remote] = k.used;
    port.machine->drawing = drawing;
    port.Step(players);
    port.machine->drawing = true;
    if ((at + 1) % kHashEvery == 0 || port.hashFile) {
        k.hash = port.StateHash();
    }
}

void Netplay::Frame(Port &port, uint16_t controls) {
    current = port.frame;
    const uint64_t frame = port.frame;
    localInput[(frame + delay) % kInputs] = controls;
    localUpTo = frame + delay + 1;
    // How far ahead of the other side this side is: the frame it is on, over what the other side said last
    ahead = ahead * 0.95 + (static_cast<double>(frame) - static_cast<double>(remoteFrame)) * 0.05;
    SendInputs();
    // A wrong guess since the last frame that had the real controls: back to it, and on to here again, unseen and
    // unheard (the sound of those frames has been heard)
    const uint64_t upTo = std::min(remoteUpTo, frame);
    uint64_t wrong = upTo;
    for (uint64_t at = confirmed; at < upTo; at++) {
        if (kept[at % kKept].used != remoteInput[at % kInputs]) {
            wrong = at;
            break;
        }
    }
    if (wrong < upTo) {
        const size_t sound = port.machine->audio.size();
        port.PutBackState(kept[wrong % kKept].state);
        for (uint64_t at = wrong; at < frame; at++) {
            if (at != wrong) {
                port.KeepState(kept[at % kKept].state);
            }
            Run(port, at, false);
        }
        port.machine->audio.resize(sound);
        rollbacks++;
        rerun += frame - wrong;
        mostRerun = std::max(mostRerun, static_cast<int>(frame - wrong));
    }
    // The frames that now have the real controls: their hashes, to compare (and to the hash log)
    for (uint64_t at = confirmed; at < upTo; at++) {
        const Kept &k = kept[at % kKept];
        if (port.hashFile) {
            fprintf(port.hashFile, "%llu %08x\n", static_cast<unsigned long long>(at + 1), k.hash);
        }
        if ((at + 1) % kHashEvery == 0) {
            ourHashes[at + 1] = k.hash;
            lastHashFrame = at + 1;
            lastHash = k.hash;
        }
    }
    confirmed = upTo;
    Compare();
    // The frame itself
    port.KeepState(kept[frame % kKept].state);
    Run(port, frame, true);
    Report();
}

void Netplay::Compare() {
    for (auto it = ourHashes.begin(); it != ourHashes.end();) {
        const auto theirs = theirHashes.find(it->first);
        if (theirs != theirHashes.end()) {
            if (theirs->second != it->second && ended.empty()) {
                ended = "The two games came apart (a desync at frame " + std::to_string(it->first) + "): the match is stopped.";
                failed = true;
                Writer refuse(kRefuse);
                refuse.Text(ended);
                Send(refuse.out);
                Pump();
            }
            it = ourHashes.erase(it);
        } else if (it->first + kHashEvery * 20 < confirmed) {
            it = ourHashes.erase(it); // (theirs never came: lost)
        } else {
            ++it;
        }
    }
    while (!theirHashes.empty() && theirHashes.begin()->first + kHashEvery * 20 < confirmed) {
        theirHashes.erase(theirHashes.begin());
    }
}

void Netplay::Leave() {
    Writer bye(kBye);
    for (int n = 0; n < 3; n++) {
        socket.Send(peer, bye.out.data(), bye.out.size());
    }
}

void Netplay::Report(bool last) {
    const uint64_t now = NowMs();
    if (!last && now - lastReport < 10000) {
        return;
    }
    if (!lastReport && !last) {
        lastReport = now;
        return;
    }
    lastReport = now;
    fprintf(stderr, "online: frame %llu, ping %.0f ms, %llu rollbacks (%llu since the last report), %llu frames run again, at most %d at once; %llu ticks waited\n",
            static_cast<unsigned long long>(current), ping, static_cast<unsigned long long>(rollbacks),
            static_cast<unsigned long long>(rollbacks - reportedRollbacks), static_cast<unsigned long long>(rerun), mostRerun,
            static_cast<unsigned long long>(waits));
    reportedRollbacks = rollbacks;
}

} // namespace port
