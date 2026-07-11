<div align="center">

# GIGACHAD Loader

![C++](https://img.shields.io/badge/C++-17-9B59F5?style=for-the-badge&logo=cplusplus&logoColor=white)
![Win32](https://img.shields.io/badge/Win32-API-6E30A0?style=for-the-badge&logo=windows&logoColor=white)
![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2026-A855F7?style=for-the-badge&logo=visualstudio&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-8855DD?style=for-the-badge)
![Status](https://img.shields.io/badge/status-active-purple?style=for-the-badge)

<br>

<img src="https://img.shields.io/badge/-Single%20File-000000?style=flat-square&labelColor=1a1a1a&color=A855F7" />
<img src="https://img.shields.io/badge/-No%20External%20Libs-000000?style=flat-square&labelColor=1a1a1a&color=A855F7" />
<img src="https://img.shields.io/badge/-GDI%20Rendered-000000?style=flat-square&labelColor=1a1a1a&color=A855F7" />
<img src="https://img.shields.io/badge/-60%20FPS-000000?style=flat-square&labelColor=1a1a1a&color=A855F7" />

</div>

---

##  Overview

**GigaChad** is a fully custom-drawn Windows desktop GUI built directly on the raw **Win32 API** — no MFC, no WinForms, no third-party UI frameworks. Every pixel of the login screen, loading spinner, buttons, and animated snow overlay is hand-rendered with **GDI**, double-buffered for a smooth, flicker-free experience.

It's built as a **single `.cpp` file**, making it easy to drop into any fresh Visual Studio 2026 "Windows Desktop Application" project and compile immediately.

---

## Preview Flow

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────────┐
│   LOGIN     │ ──▶│   LOADING    │ ──▶│    MAIN     │ ──▶│   POPUP         │
│  user/pass  │     │  5s spinner │     │  5 buttons  │     │  "debug" msgbox │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────────┘
```

| Screen | What happens |
|---|---|
|  **Login** | Themed username/password fields, validated locally |
|  **Loading** | Custom-drawn rotating arc spinner, 5-second delay |
|  **Main Menu** | 5 owner-drawn purple/black buttons in a responsive grid |
|  **Action** | Short spinner → native `MessageBoxW` popup ("Hi") |
|  **Always-on** | Soft, alpha-blended falling snow layered over every screen |

---

##  Design & Theme

- **Palette:** near-black background (`#08060E`) with layered purple accents (`#A855F7`, `#5A3091`)
- **Typography:** Segoe UI throughout, weighted per hierarchy (bold titles, semibold buttons)
- **Snow:** real alpha-transparent radial-gradient sprites (not flat shapes), rendered via `AlphaBlend`, with parallax depth (dim = far/slow, bright = near/fast)
- **No flicker:** `WS_CLIPCHILDREN` + double-buffered GDI painting keep the animated background from ever fighting with the live controls

---

##  Tech Stack

| Layer | Technology |
|---|---|
| Language | C++17 |
| UI | Raw **Win32 API** (`user32`, `gdi32`, `comctl32`, `msimg32`) |
| Rendering | GDI, double buffering, `AlphaBlend` |
| Build | Visual Studio 2026, no CLR, no external SDKs |

---

##  Getting Started

1. Create a new **Windows Desktop Application** (C++, no CLR) project in Visual Studio 2026
2. Drop `GigaChad.cpp` in, replace the auto-generated stub
3. **Project Properties → Linker → System → Subsystem** → `Windows`
4. **Character Set** → `Unicode`
5. Build & run

**Default login:** `1234` / `1234` *(local check only — see below for how this could evolve into real license/key auth)*

---

##  Adding Key Authentication (Concept Overview)

Right now the login is a hardcoded local check. To turn this into a real **license/key authentication system**, the typical architecture looks like this:

```
┌────────────┐        HTTPS / TLS         ┌────────────────┐        ┌────────────┐
│   Client   │  ───────────────────────▶  │  Auth Server    │  ───▶  │  Database   │
│ (this app) │  key + HWID + timestamp     │ (validates key) │        │ (keys, HWID)│
└────────────┘  ◀───────────────────────  └────────────────┘        └────────────┘
                  signed token / JWT
```

**Key building blocks:**

- **Transport security** — all requests go over HTTPS/TLS (`WinHTTP` or `WinINet` on the Win32 side) so keys aren't sent in plaintext
- **Server-side validation** — the key is checked against a database (valid, expired, revoked, already bound) rather than trusted client-side
- **HWID binding** — the server ties a key to a hashed hardware identifier (e.g. disk serial + motherboard ID) on first use to prevent sharing
- **Signed session tokens** — on success, the server issues a short-lived signed token (JWT-style) that the client holds in memory for the session instead of re-sending the raw key
- **Expiry & heartbeat** — the client periodically re-validates the token with the server, so revoking a key takes effect without restarting the app
- **No client-side secrets** — validation logic, expiry dates, and key lists must never live in the client binary, since anything shipped to the user can eventually be read or patched

This is architecture-level guidance rather than a drop-in implementation — the actual server, database schema, and crypto choices depend on your stack (e.g. a small Node/Go/Rust backend + Postgres + JWT is a common, solid combination).

---

##  Project Structure

```
GigaChad/
└── GigaChad.cpp   ← everything: window, controls, rendering, snow, state machine
```

---

## 📬 Contact

<div align="center">

[![Telegram](https://img.shields.io/badge/Telegram-t.me%2Fwizz3ard-A855F7?style=for-the-badge&logo=telegram&logoColor=white)](https://t.me/wizz3ard)

</div>

---

<div align="center">

**Built with Win32 API, GDI, and way too much purple.**

</div>
