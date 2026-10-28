// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The introduction service for online play (port/netplay.h, online/README.md). It only introduces: a host asks for a
// code and says where it can be reached (its addresses as the internet and its own network see them); the other
// player gives the code and where they can be reached, and is told the host's; the host, asking again, is told
// theirs. The two then reach each other directly; nothing of the match goes through here. A code lasts 30 minutes,
// or until it has been joined. Every match is a Durable Object of its own, named by its code.
//
//   POST /v1/host                {game, build, addresses}          -> {code, secret}
//   POST /v1/host                {code, secret, game, build, addresses} -> {code, secret}   (the same code again, for
//                                                                  the player who left to come back to: Netplay's reconnect)
//   GET  /v1/host/CODE?secret=S                                    -> {joined: false} | {joined: true, addresses}
//   POST /v1/join/CODE           {game, build, addresses}          -> {addresses} | 404 | 409 {error}
//
// `addresses` are "IP:PORT" strings (IPv4, or [IPv6]:PORT), at most 8: each side's own, as STUN and its network say them.

const kAlphabet = "23456789ABCDEFGHJKMNPQRSTUVWXYZ"; // (no 0 and O, 1 and I and L)
const kLifetime = 30 * 60 * 1000;

function json(body, status = 200) {
  return new Response(JSON.stringify(body), { status, headers: { "content-type": "application/json" } });
}

function newCode() {
  const bytes = crypto.getRandomValues(new Uint8Array(6));
  return Array.from(bytes, (b) => kAlphabet[b % kAlphabet.length]).join("");
}

// "K7Q-M2X", "k7qm2x": the code as kept
function cleanCode(text) {
  const code = text.toUpperCase().replace(/[^0-9A-Z]/g, "");
  return code.length === 6 && [...code].every((c) => kAlphabet.includes(c)) ? code : null;
}

// What a client says it can be reached at, checked, with where its request came from (the port its socket was
// seen at from the internet is in the addresses it sends: STUN; the request's own address is its IP only)
function cleanAddresses(list) {
  if (!Array.isArray(list)) {
    return [];
  }
  return list
    .filter((a) => typeof a === "string" && a.length <= 64 && /^(\d{1,3}(\.\d{1,3}){3}|\[[0-9a-fA-F:.]+\]):\d{1,5}$/.test(a))
    .slice(0, 8);
}

function cleanText(text) {
  return typeof text === "string" ? text.slice(0, 64) : "";
}

export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    const parts = url.pathname.split("/").filter(Boolean);
    if (parts[0] !== "v1") {
      // (someone who came here in a browser: to the project)
      return Response.redirect("https://github.com/tmcleroy/cps2recomp", 302);
    }
    if (request.method === "POST" && parts[1] === "host" && parts.length === 2) {
      const text = await request.text();
      let asked = null;
      try {
        asked = JSON.parse(text);
      } catch {
        return json({ error: "not JSON" }, 400);
      }
      // (the host's code again: open to be joined once more, if it is theirs, or free)
      const again = typeof asked.code === "string" ? cleanCode(asked.code) : null;
      if (again && typeof asked.secret === "string") {
        const match = env.MATCHES.get(env.MATCHES.idFromName(again));
        const answer = await match.fetch("https://match/host", { method: "POST", body: text });
        if (answer.ok) {
          return json({ code: again, secret: asked.secret });
        }
      }
      // (a code not in use: tried again on the rare clash)
      for (let tries = 0; tries < 5; tries++) {
        const code = newCode();
        const match = env.MATCHES.get(env.MATCHES.idFromName(code));
        const answer = await match.fetch("https://match/host", { method: "POST", body: JSON.stringify({ ...asked, code: undefined, secret: undefined }) });
        if (answer.status !== 409) {
          const body = await answer.json();
          return json(answer.ok ? { code, secret: body.secret } : body, answer.status);
        }
      }
      return json({ error: "no code free" }, 503);
    }
    const code = parts.length === 3 ? cleanCode(parts[2]) : null;
    if (!code) {
      return json({ error: "no such code" }, 404);
    }
    const match = env.MATCHES.get(env.MATCHES.idFromName(code));
    if (request.method === "GET" && parts[1] === "host") {
      return match.fetch(`https://match/poll?secret=${encodeURIComponent(url.searchParams.get("secret") || "")}`);
    }
    if (request.method === "POST" && parts[1] === "join") {
      return match.fetch("https://match/join", { method: "POST", body: await request.text() });
    }
    return json({ error: "not here" }, 404);
  },
};

export class Match {
  constructor(state) {
    this.state = state;
  }

  async fetch(request) {
    const url = new URL(request.url);
    const now = Date.now();
    let match = (await this.state.storage.get("match")) || null;
    if (match && now - match.made > kLifetime) {
      await this.state.storage.deleteAll();
      match = null;
    }
    if (url.pathname === "/host") {
      let body;
      try {
        body = JSON.parse(await request.text());
      } catch {
        return json({ error: "not JSON" }, 400);
      }
      // (taken, unless by this host asking for it again; free, it may be had with the secret the host keeps)
      if (match && !(typeof body.secret === "string" && body.secret === match.secret)) {
        return json({ error: "in use" }, 409);
      }
      const secret = typeof body.secret === "string" && /^[0-9a-f-]{36}$/.test(body.secret) ? body.secret : crypto.randomUUID();
      match = { made: now, secret, game: cleanText(body.game), build: cleanText(body.build), host: cleanAddresses(body.addresses), joiner: null };
      await this.state.storage.put("match", match);
      await this.state.storage.setAlarm(now + kLifetime);
      return json({ secret });
    }
    if (!match) {
      return json({ error: "no such code" }, 404);
    }
    if (url.pathname === "/poll") {
      if (url.searchParams.get("secret") !== match.secret) {
        return json({ error: "no such code" }, 404);
      }
      return json(match.joiner ? { joined: true, addresses: match.joiner } : { joined: false });
    }
    if (url.pathname === "/join") {
      let body;
      try {
        body = JSON.parse(await request.text());
      } catch {
        return json({ error: "not JSON" }, 400);
      }
      if (cleanText(body.game) !== match.game) {
        return json({ error: `That code is for another game (${match.game}).` }, 409);
      }
      if (cleanText(body.build) !== match.build) {
        return json({ error: "The two copies of the game are different builds: use the same release on both." }, 409);
      }
      if (match.joiner) {
        return json({ error: "Someone has joined that match already." }, 409);
      }
      match.joiner = cleanAddresses(body.addresses);
      await this.state.storage.put("match", match);
      return json({ addresses: match.host });
    }
    return json({ error: "not here" }, 404);
  }

  async alarm() {
    await this.state.storage.deleteAll();
  }
}
