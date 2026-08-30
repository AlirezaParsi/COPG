<div align="center">

<img src="https://raw.githubusercontent.com/AlirezaParsi/COPG/refs/heads/JSON/module/banner.png" width="560" alt="COPG banner" />

# 🎮 COPG

**The most advanced device &amp; CPU spoofer for Android — bypass restrictions and unlock premium graphics, higher FPS and exclusive features on most games and apps.**

<br>

[![Version](https://img.shields.io/badge/version-6.7.0-818cf8?style=for-the-badge)](https://github.com/AlirezaParsi/COPG/releases)
[![Zygisk](https://img.shields.io/badge/Zygisk-Compatible-34d399?style=for-the-badge)](https://github.com/topjohnwu/Magisk)
[![Android](https://img.shields.io/badge/Android-9.0%2B-3ddc84?style=for-the-badge&logo=android&logoColor=white)](https://www.android.com/)
[![Downloads](https://img.shields.io/github/downloads/AlirezaParsi/COPG/total?style=for-the-badge&color=f59e0b)](https://github.com/AlirezaParsi/COPG/releases)
[![License](https://img.shields.io/github/license/AlirezaParsi/COPG?style=for-the-badge&color=a78bfa)](LICENSE)

<a href="#-installation"><img src="https://img.shields.io/badge/⬇_Install-818cf8?style=for-the-badge" alt="Install" /></a>
<a href="#-webui"><img src="https://img.shields.io/badge/🖥_WebUI-1f2937?style=for-the-badge" alt="WebUI" /></a>
<a href="#-faq"><img src="https://img.shields.io/badge/❓_FAQ-1f2937?style=for-the-badge" alt="FAQ" /></a>
<a href="https://t.me/COPG_module"><img src="https://img.shields.io/badge/💬_Telegram-2CA5E0?style=for-the-badge" alt="Telegram" /></a>
<a href="#-support-copg"><img src="https://img.shields.io/badge/Support-f59e0b?style=for-the-badge&logo=bitcoin&logoColor=white" alt="Support" /></a>

</div>

---

## ✨ Why COPG?

COPG is a **Zygisk module** that makes most games and apps believe they are running on a different,
fully‑featured flagship device — unlocking the high‑FPS modes, HD graphics and premium tiers that
are otherwise gated to specific hardware. It pairs that with a **CPU spoofer**, a userspace
**comfort‑tweak controller**, and a beautiful on‑device **WebUI** to manage everything — all
**without rebooting**.

<table>
<tr>
<td width="50%" valign="top">

#### 🎯 Device Spoofing
Per‑app device profiles (brand, model, fingerprint, SDK, **baseband**, **per‑app serial** and 12
extra Build fields) so each game sees the exact flagship it rewards.

#### ⚙️ CPU Spoofing
Spoof the CPU to flagship‑class silicon for apps that gate features on the chipset.

#### 🎨 GPU Spoofing *(PRO · opt‑in)*
Spoof the **GPU** — OpenGL renderer/vendor &amp; Vulkan device name — per app for games that gate
graphics on the chip. **GPU spoofing is a PRO feature** (unlocked with a license). It's a **resident**
hook, so it sits behind a clear *use‑at‑your‑own‑risk* gate — **never enable it for anti‑cheat games.**

#### 🖥️ Refresh‑Rate Spoof *(free)*
Make an app read a **custom display refresh rate** (e.g. 120 / 144 / 165 Hz) — the current rate **and**
the full supported‑modes list — for device‑info apps and games that show or gate on the panel's Hz.
Resident hook behind a *use‑at‑your‑own‑risk* gate — never for anti‑cheat games.

#### 📡 IMEI &amp; Device ID *(PRO)*
Fake the **IMEI / device ID** — **per app**, or **device‑wide** with the new **Global IMEI** (hooks the
phone service so Settings, `*#06#` and every app read it; runs in a separate process apps can't scan, so
**no risk gate**).

#### 🔒 DRM / Widevine *(PRO)*
Report a higher **Widevine security level** (L1 / L2 / L3) and spoof the DRM **device &amp; system ID**
for apps that gate or display it.

#### 🌍 Timezone &amp; Language *(free)*
Give each app its own **timezone** and its own **language / region** (BCP‑47) — apps read and even
**render** in the fake locale. Applied the stealth, system‑side way, so nothing loads into the app.

#### ⏱️ Fake Uptime *(PRO)*
Make an app think the device has been running for **days** — shifts both the Java and native uptime
readers, useful against anti‑fraud / referral flows that distrust a freshly‑reset device.

#### 🌐 WebView User‑Agent *(PRO)*
Give any WebView‑based app its own **browser User‑Agent** — from a reusable named profile — for sites
and in‑app pages that gate content, layout or pricing on the browser identity.

#### 📸 Screenshot &amp; Storage Unlocks *(free)*
**Disable FLAG_SECURE** device‑wide to screenshot / screen‑record apps that block it (banking, chat,
wallets) — done in the system, invisible to the app · **No Storage Restrict** re‑enables picking
Android/data, obb, the SD‑card root and Download in the system file picker. Both toggle with no reboot.

#### 🎟️ Play Source &amp; PairIP Bypass *(PRO)*
Make an app look **Play‑installed** (installer → Google Play) and **block Google's PairIP license
check** so paid / locked apps open normally. Both run in the system, out of the app — nothing loads
into the app's process, so they're **not detectable**.

</td>
<td width="50%" valign="top">

#### 🧬 Prop Spoofing &amp; Android ID
Stealth **copy‑on‑write** prop spoof (fingerprint, build props &amp; more) *(PRO)* and per‑app
**Android ID** *(PRO)* — both are stealth: the module unloads before the game runs, so nothing of
COPG stays mapped in memory — safe even for anti‑cheat games. (CPU spoof, device profiles, Build/serial
fields and block‑CPU stay free.)

#### 📶 SIM / Carrier Spoofing *(PRO)*
Make an app read a different **network carrier** — name, operator code (MCC/MNC) &amp; country — per
app, even a **different carrier per SIM slot**. **Safe** mode is fully stealth (anti‑cheat safe);
**Aggressive** mode also covers the newer subscription API but is resident (opt‑in, never for
anti‑cheat games).

#### 🆔 Per‑App Advertising ID *(PRO)*
Give each app its **own Google Advertising ID** — automatic per‑app, or pin an exact UUID — for ad /
reward / referral / multi‑account apps where each install should look like a different device.

#### 🧩 Per‑App App Set ID *(PRO)*
Give each app its **own Google App Set ID** — the resettable fingerprint signal ad / analytics SDKs
read via Play Services — automatic per‑app, or pin an exact UUID, so each looks like a separate device.
It's a **resident** hook behind a *use‑at‑your‑own‑risk* gate — **never for anti‑cheat games.**

#### 🌐 Per‑App Proxy *(PRO)*
Route a single app through your own **SOCKS5 / HTTP / SOCKS4** proxy so its traffic exits from that IP
and country — no VPN, no interface the app can see, nothing runs inside the app (kernel‑level, ban‑safe).
Save proxies and assign one per app; pair with a matching SIM/region so device, SIM and IP all agree.

#### 📍 GPS Location Spoof *(PRO)*
Put an app **anywhere on the map** — pick a country/city from the built‑in list (135 places) or drop in
your own latitude/longitude. Covers the normal location APIs **and** Google's Fused/GMS provider; only
lat/lng change (accuracy/altitude stay real). Resident hook behind a *use‑at‑your‑own‑risk* gate —
never for anti‑cheat games.

#### 🛡️ Privacy Hides
**Hide VPN** — covers both the Java (network‑interface / capabilities) and native interface checks,
pairip‑safe *(free)* · **Mock‑Location hide** *(PRO)* · **Hide Developer Options + USB
debugging** (free) — pass the checks that banking &amp; privacy‑sensitive apps run.

#### 🎛️ Per‑App Comfort Tweaks
Auto **Do‑Not‑Disturb**, **disable auto‑brightness**, **keep screen on**, **stop logging** and a
per‑app **screen DPI** — applied only while a tagged game is active, then restored.

</td>
</tr>
</table>

> 🔁 **Add or remove devices, games &amp; apps without a reboot.** ✨ Fully customizable. 🌍 9‑language
> WebUI with Light / Dark / AMOLED themes.

---

## 🚀 Maximize Your Gaming

<div align="center">

| Game | Unlock |
|------|--------|
| **Call of Duty Mobile** | 120 FPS (BR / MP) |
| **PUBG Mobile / BGMI** | 120 FPS · Haptic Feedback |
| **Delta Force** | 120 FPS · HD Graphics |
| **Free Fire / Free Fire MAX** | 144 FPS |
| **Mobile Legends: Bang Bang** | 144 FPS |
| **Fortnite** | 120 FPS |
| **Asphalt 9** | 120 FPS |
| **Farlight 84** | Max Graphics |
| **Arena Breakout** | 90 / 120 FPS |
| **Honor of Kings** | High‑FPS mode |
| _…and 69+ more_ | Premium tiers unlocked |

</div>

#### 📱 App Enhancements
- **TikTok** — stream in full 1080p

---

## 🖼️ Screenshots

<div align="center">
<table>
  <tr>
    <td align="center" width="25%">
      <img src="https://github.com/AlirezaParsi/COPG/blob/screenshots/Screenshot_20260703-114658_WebUI%20X.png?raw=true" alt="Dashboard" />
      <br><sub><b>Dashboard</b> · System info</sub>
    </td>
    <td align="center" width="25%">
      <img src="https://github.com/AlirezaParsi/COPG/blob/screenshots/Screenshot_20260712-032329_WebUI%20X.png?raw=true" alt="Library packages" />
      <br><sub><b>Library</b> · Packages</sub>
    </td>
    <td align="center" width="25%">
      <img src="https://github.com/AlirezaParsi/COPG/blob/screenshots/Screenshot_20260711-113747_WebUI%20X.png?raw=true" alt="Per-app spoof toggles" />
      <br><sub><b>Per-App</b> · Spoof toggles</sub>
    </td>
    <td align="center" width="25%">
      <img src="https://github.com/AlirezaParsi/COPG/blob/screenshots/Screenshot_20260702-162602_WebUI%20X.png?raw=true" alt="GPU profile editor" />
      <br><sub><b>GPU</b> · Profile editor</sub>
    </td>
  </tr>
</table>
</div>

---

## 📦 Installation

### Requirements

- A **rooted** Android device (**9.0+**)
- One root solution **+ a Zygisk implementation**:

| Root | Min&nbsp;version | Zygisk |
|------|:------:|--------|
| ![Magisk](https://img.shields.io/badge/Magisk-v24%2B-00B39B?style=flat&logo=android&logoColor=white) | 24 | [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) · [ReZygisk](https://github.com/PerformanC/ReZygisk) · [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) |
| ![KernelSU](https://img.shields.io/badge/KernelSU-0.6.6%2B-7D4698?style=flat) | 0.6.6 | [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) · [ReZygisk](https://github.com/PerformanC/ReZygisk) · [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) |
| ![APatch](https://img.shields.io/badge/APatch-0.10%2B-4285F4?style=flat) | 0.10 | [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) · [ReZygisk](https://github.com/PerformanC/ReZygisk) · [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) |

> [!IMPORTANT]
> Standard / built‑in **Magisk Zygisk is _not_ supported** (not safe). Use one of the Zygisk
> implementations above.

### Get the module

[![MMRL](https://mmrl.dev/assets/badge.svg)](https://mmrl.dev/repository/zguectZGR/COPG)
[![Magisk Alt Repo](https://img.shields.io/badge/Magisk_Alt_Repo-COPG-00B39B?style=for-the-badge&logo=magisk&logoColor=white)](https://github.com/Magisk-Modules-Alt-Repo/COPG)
[![KernelSU Repo](https://img.shields.io/badge/KernelSU_Repo-COPG-7D4698?style=for-the-badge)](https://github.com/KernelSU-Modules-Repo/COPG)

Install from **[MMRL](https://mmrl.dev/repository/zguectZGR/COPG)** (auto‑updates), from the
**[Magisk Alt Repo](https://github.com/Magisk-Modules-Alt-Repo/COPG)** or the
**[KernelSU Modules Repo](https://github.com/KernelSU-Modules-Repo/COPG)**, or grab the latest
`COPG.zip` from **[Releases](https://github.com/AlirezaParsi/COPG/releases)**.

### Steps

1. Download the latest **`COPG.zip`** from [Releases](https://github.com/AlirezaParsi/COPG/releases).
2. Install via your root manager → **Modules → Install from storage → select the ZIP**.
3. **Reboot.**
4. Verify it shows up in your root manager (look for **✨ COPG spoof ✨**).
5. Open the module's **WebUI** to manage devices, games and tweaks.

---

## 🖥️ WebUI

A no‑reboot, on‑device control panel built into the module. On **KernelSU** and **APatch** it opens
straight from the manager. On **Magisk**, install the **KSU WebUI** app and open COPG from there.

- 📋 **Library** — add &amp; manage **device profiles** and **per‑app spoof lists** with search,
  sort &amp; filters
- ➕ **Add Package** — pick any installed app, choose a device profile, toggle **CPU / GPU / SIM /
  Prop / Android ID / Advertising ID (GAID) / App Set ID / DRM / IMEI / Timezone / Language /
  WebView User‑Agent / Fake Uptime / Mock‑Location / Hide VPN / Hide Developer Options** and the
  **DND / Auto‑Brightness / Keep‑Screen‑On / Screen‑DPI** tweaks
- 📊 **Dashboard** — live system info: Android, ABI, Zygisk variant, root &amp; kernel
- 🆔 **Advertising ID** — view, randomize, set a custom one or restore your real ID (Settings · free)
- 📡 **Global Hooks** — device‑wide **Global IMEI** (Settings): one fake IMEI for every app, `*#06#`
  and the dialer
- 💾 **Backup / Restore** &amp; **Sync from GitHub**
- 🎨 **Light / Dark / AMOLED** themes · 🌍 **9 languages** (EN, FA, AR, DE, ES, ID, TH, TR, ZH)

<details>
<summary><b>⚙️ Advanced — edit profiles by hand</b></summary>

<br>

The WebUI is the recommended way to manage profiles, but you can also edit
`/data/adb/modules/COPG/COPG.json` directly:

```json
{
  "PACKAGES_REDMAGIC_9_PRO": [
    "com.mobilelegends.mi",
    "com.supercell.brawlstars:with_cpu"
  ],
  "PACKAGES_REDMAGIC_9_PRO_DEVICE": {
    "BRAND": "ZTE",
    "MODEL": "NX769J",
    "FINGERPRINT": "ZTE/NX769J/..."
  }
}
```

Package **tags** are colon suffixes — e.g. `:cpu=<model>` (CPU spoof + pick the chip),
`:gpu=<model>` (GPU spoof + pick the GPU), `:cow` (prop spoof), `:aid` (Android ID),
`:serial` (per‑app serial), `:gaid` (Advertising ID), `:appset` (App Set ID), `:drm` (Widevine),
`:imei` (IMEI), `:sim=<carrier>` / `:simx=<carrier>` (SIM · safe / aggressive), `:tz=<zone>` (timezone),
`:lang=<bcp47>` (language / region), `:ua=<profile>` (WebView User‑Agent), `:uptime=<sec>` (fake uptime),
`:mock` (mock‑location hide), `:vpn` / `:vpns` (VPN hide), `:hidedev` (hide developer options),
`:blocked` (force real CPU), `:dnd` / `:dab` / `:kso` / `:nolog` / `:dpi=<n>` (comfort tweaks).

</details>

---

## ❓ FAQ

<details>
<summary><b>🤔 What does COPG stand for?</b></summary>
<br>
Originally <b>CO</b> (Call of Duty) + <b>PG</b> (PUBG) — the two games it was first built for. It
now supports most games and apps, but the name is kept for its history.
</details>

<details>
<summary><b>📌 Will this get me banned?</b></summary>
<br>
COPG is designed to be undetectable, but safety can't be guaranteed. <b>Use at your own risk.</b>
</details>

<details>
<summary><b>⚡ Does it affect performance?</b></summary>
<br>
Minimal. The spoof runs only at app launch and doesn't touch in‑game performance.
</details>

<details>
<summary><b>🔧 How do I open the WebUI?</b></summary>
<br>
On <b>KernelSU</b> and <b>APatch</b>, open it straight from the manager. On <b>Magisk</b>, install
the <b>KSU WebUI</b> app and open COPG from there.
</details>

---

## 💬 Community

<div align="center">

[![Telegram Channel](https://img.shields.io/badge/Telegram_Channel-2CA5E0?style=for-the-badge&logo=telegram&logoColor=white)](https://t.me/COPG_module)
[![Telegram Group](https://img.shields.io/badge/Telegram_Group-2CA5E0?style=for-the-badge&logo=telegram&logoColor=white)](https://t.me/TheAOSP)

</div>

> 🌐 **Translations are community‑driven.** Only English &amp; Persian are maintained by the author —
> PRs for the other languages are very welcome!

---

## ₿ Support COPG

COPG is developed in my spare time and given away free. If it leveled up your games, please consider
supporting with **any amount you wish** — every bit is real motivation for stronger, continued
development. And a ⭐ on GitHub helps a lot too!

| Network | Address |
| --- | --- |
| ![USDT ERC20](https://img.shields.io/badge/USDT-ERC20-627EEA?style=flat-square&logo=ethereum&logoColor=white) | `0xB8eb7Ea033823C9aA4616B0648B89CDbC931BAAd` |
| ![USDT TRC20](https://img.shields.io/badge/USDT-TRC20-EF0027?style=flat-square&logo=tron&logoColor=white) | `TMMDAyJ9Fs3yQpidd2Q4eYLTMkM1hsrauV` |
| ![USDT BEP20](https://img.shields.io/badge/USDT-BEP20-F0B90B?style=flat-square&logo=binance&logoColor=white) | `0xB8eb7Ea033823C9aA4616B0648B89CDbC931BAAd` |
| ![GRAM TON](https://img.shields.io/badge/GRAM-TON-0098EA?style=flat-square&logo=ton&logoColor=white) | `UQAOHoREeGeJ0_kzJpSW3m-6Dlb_lzdHpT1a-gA7NkbuCM8N` |

> ⚠️ Double‑check the **network** before sending — funds sent on the wrong chain can be lost.

---

## 📈 Activity

<div align="center">

![Repobeats analytics](https://repobeats.axiom.co/api/embed/83b280d0986b3c023ed5f1fdf3f00f77288e3da3.svg "Repobeats analytics image")

</div>

---

<div align="center">

**If COPG leveled up your games, drop a ⭐ — it really helps!**

Made with ❤️ by **Alireza Parsi** · © 2026 COPG Project

</div>
