// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// Online play (issue #70): two copies of the game, one on each player's machine, each running the whole game with
// both players' controls. Each side sends the other its own player's controls, frame by frame, over UDP. A frame
// does not wait for the other side's: until they come they are guessed (as they last were), and when they come and
// a guess was wrong, the game is put back to that frame and the frames since are run again with the real ones
// (rollback, as --rollback-test tries it: port.h, RollbackTest). Neither side is ahead of the other's controls by
// more than kMostAhead frames: past that, it waits.
//
// --host [PORT] waits for the other player; --join ADDRESS[:PORT] goes to them. The host is player 1. Before the
// first frame the two make sure they are the same game, and the joining side takes the host's board settings
// (the settings memory) and the features that change play (Game, Board and Cheats, and widescreen); the rest of
// the alternative modes, training, run-ahead and the like stay off. Every so often each sends the other a hash of
// the game as it was at a frame both have the real controls for; if the two differ, the games have come apart
// (a desync), and the match stops, saying so.
#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <vector>

namespace port {

struct Port;

// A UDP socket, IPv6 and IPv4 both; the other side's address
struct NetAddress {
    std::array<uint8_t, 128> storage{}; // (a sockaddr_storage)
    int length = 0;
    // (an IPv4 address: as IPv4 too, for a socket that has no IPv6)
    std::array<uint8_t, 16> four{};
    bool hasFour = false;
    bool operator==(const NetAddress &other) const {
        return length == other.length && storage == other.storage;
    }
};
class UdpSocket {
public:
    ~UdpSocket();
    bool Open(int port, std::string &error); // (0: any port)
    static bool Resolve(const std::string &host, int port, NetAddress &address, std::string &error);
    void Send(const NetAddress &to, const uint8_t *data, size_t size);
    // A packet that has come (its size), or -1 for none yet
    int Receive(uint8_t *data, size_t size, NetAddress &from);

private:
    intptr_t handle = -1;
    bool six = false; // (IPv6, and IPv4 through it)
};

class Netplay {
public:
    static constexpr int kDefaultPort = 7845;
    static constexpr int kMostAhead = 8;   // frames run on guessed controls at most
    static constexpr int kKept = 16;       // states kept (more than the most a rollback goes back)
    static constexpr int kInputs = 128;    // controls kept, each side's
    static constexpr int kHashEvery = 60;  // frames between the hashes compared
    static constexpr int kDefaultDelay = 2; // frames a player's own controls are held back (fewer rollbacks)

    // What --host, --join and the --net-* options asked for
    struct Settings {
        bool hosting = false;
        std::string address; // (joining: the host's)
        int port = kDefaultPort;
        int delay = kDefaultDelay;
        // (tests: each packet sent late by this many ms, give or take `jitter`, and this many in a hundred lost)
        int fakeLag = 0, fakeJitter = 0, fakeLoss = 0;
    };
    explicit Netplay(const Settings &settings) : settings(settings) {}
    // The socket, the other side found and agreed with (blocks until then; false and `error` if it did not go)
    bool Connect(Port &port, std::string &error);
    // ... or a step at a time, for a window that shows the wait: Begin, then Waiting each tick until it is not 0 (1:
    // the match is on; -1: it did not go, as `error` says)
    bool Begin(Port &port, std::string &error);
    int Waiting(Port &port, std::string &error);
    std::string waiting; // what is being waited for ("Waiting for the other player...")
    // This side's player (0: player 1)
    int Local() const {
        return local;
    }
    // Each tick: what has come in, sent again what has not been taken; false if the next frame must wait (the
    // other side's controls are too far behind, or this side is ahead of the other in time)
    bool Ready(uint64_t frame);
    // The frame, with this side's player's controls: rollback first if a guess was wrong, then the frame itself
    void Frame(Port &port, uint16_t controls);
    // Why the match is over ("" while it goes on); and whether that is a failure (a desync, the other side lost)
    std::string ended;
    bool failed = false;
    // The match is over on this side: the other side is told
    void Leave();
    // A line on how it is going (ping, rollbacks), every few seconds, to stderr
    void Report(bool last = false);

private:
    Settings settings;
    UdpSocket socket;
    // (the handshake: the hello or the welcome, sent again until answered)
    std::vector<uint8_t> greeting;
    uint32_t build = 0;
    uint64_t began = 0, sent = 0;
    bool found = false; // (hosting: the other player has said hello)
    int joinPort = 0;
    void StartAt(int frames);
    NetAddress peer;
    int local = 0, remote = 1;
    int delay = kDefaultDelay;
    std::array<uint16_t, kInputs> localInput{}, remoteInput{};
    uint64_t localUpTo = 0;  // this side's controls are known for the frames before this
    uint64_t remoteUpTo = 0; // ... and the other side's (all of them before it)
    uint64_t remoteAck = 0;  // the other side has this side's controls for the frames before this
    uint64_t remoteFrame = 0; // the frame the other side was on, last heard
    struct Kept {
        std::vector<uint8_t> state; // the game before the frame
        uint16_t used = 0;          // the other side's controls it was run with
        uint32_t hash = 0;          // the game after it, last run (when hashed)
    };
    std::array<Kept, kKept> kept;
    uint64_t confirmed = 0;  // the frames before this were run with the other side's real controls
    uint64_t current = 0;    // the frame running now
    std::map<uint64_t, uint32_t> ourHashes, theirHashes;
    uint64_t lastHashFrame = 0; uint32_t lastHash = 0; // (ours, sent with every packet)
    // Keeping time with the other side: how far ahead of it this side is (frames, averaged), and its own idea
    double ahead = 0, theirAhead = 0;
    int waitTicks = 0;
    uint64_t waitedAt = UINT64_MAX;
    // (ping)
    uint32_t echo = 0;       // the other side's time on its last packet
    uint64_t echoAt = 0;     // ... and when it came, ours
    double ping = 0;
    uint64_t lastHeard = 0;  // (ms)
    // (for the report)
    uint64_t rollbacks = 0, rerun = 0, waits = 0, lastReport = 0, reportedRollbacks = 0;
    int mostRerun = 0;
    double worst = 0;  // the longest a frame took, its rollback with it (seconds)
    uint64_t over = 0; // ... and how many took longer than a frame's time

    // (tests: packets held back, and when they go)
    struct Late { uint64_t due; std::vector<uint8_t> data; };
    std::deque<Late> late;
    uint64_t random = 0x2545F4914F6CDD1Dull;

    void Send(const std::vector<uint8_t> &packet);
    void SendInputs();
    void Pump();
    void Take(const uint8_t *data, size_t size);
    void Run(Port &port, uint64_t at, bool drawing);
    void Compare();
};

} // namespace port
