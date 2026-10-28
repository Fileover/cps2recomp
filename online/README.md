# The online service

Online play (`port/netplay.h`) is two copies of the game sending each other their players' controls directly. This
service only introduces them: a host asks it for a code; the other player gives the code, and each is told where the
other can be reached. Nothing of a match goes through it. It is a [Cloudflare Worker](https://developers.cloudflare.com/workers/)
with a [Durable Object](https://developers.cloudflare.com/durable-objects/) for each match (`src/index.js`), within
Cloudflare's free plan.

How the two meet:

1. Each game asks a public STUN server (Cloudflare's, or Google's) what address and port its UDP socket is seen from
   on the internet: the router's, for a machine behind one.
2. The host sends that and its addresses on its own networks to `POST /v1/host`, and is given a code (`K7Q-M2X`,
   six letters and digits; no 0, O, 1, I or L). It asks `GET /v1/host/CODE` every second and a half.
3. The other player sends the code and its own addresses to `POST /v1/join/CODE`, and is given the host's. The host's
   next question gives it theirs. A code lasts 30 minutes, and can be joined once; a code for another game, or for
   another build of the game, is turned away.
4. Each side sends to every address of the other's. The host's packets open its router to the other's, whose hellos
   then come through (UDP hole punching); the first address that answers is the match's.

A host that asks for it (`--net-list`, with `--net-name`) is also in the **lobby**: a Durable Object keeping every
game's listed matches whose host has asked about them in the last 10 seconds, until someone joins.
`GET /v1/lobby?protocol=N` lists them all, each with its game (the launcher's Online page); `GET /v1/lobby/GAME?protocol=N`
one game's (a game page's Online card).

Two routers that each give every destination a port of its own ("symmetric" NAT, as some mobile networks do) can't be
met this way; a relay would be needed, which the service doesn't have yet. Then the host's address, with its port
forwarded, or a VPN such as Tailscale, still works.

## Running it

```sh
cd online
npx wrangler login     # once: Cloudflare's tool, signed in to the account in a browser
npx wrangler deploy    # https://cps2recomp-online.<the account's subdomain>.workers.dev
npx wrangler dev       # or on this machine, at http://localhost:8787, for tests
```

The games ask `Netplay::kService` (`port/netplay.h`) unless started with `--net-service URL` (or `off`: no codes,
addresses only).
