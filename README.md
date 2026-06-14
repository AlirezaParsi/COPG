<div align="center">

<img src="https://raw.githubusercontent.com/AlirezaParsi/COPG/refs/heads/JSON/module/banner.png" width="560" alt="COPG banner" />

# 🎮 COPG

**The most advanced device &amp; CPU spoofer for Android — bypass restrictions and unlock premium graphics, higher FPS and exclusive features on most games and apps.**

<br>

[![Version](https://img.shields.io/badge/version-5.1.0-818cf8?style=for-the-badge)](https://github.com/AlirezaParsi/COPG/releases)
[![Zygisk](https://img.shields.io/badge/Zygisk-Compatible-34d399?style=for-the-badge)](https://github.com/topjohnwu/Magisk)
[![Android](https://img.shields.io/badge/Android-9.0%2B-3ddc84?style=for-the-badge&logo=android&logoColor=white)](https://www.android.com/)
[![Downloads](https://img.shields.io/github/downloads/AlirezaParsi/COPG/total?style=for-the-badge&color=f59e0b)](https://github.com/AlirezaParsi/COPG/releases)
[![License](https://img.shields.io/github/license/AlirezaParsi/COPG?style=for-the-badge&color=a78bfa)](LICENSE)

<a href="#-installation"><img src="https://img.shields.io/badge/⬇_Install-818cf8?style=for-the-badge" alt="Install" /></a>
<a href="#-webui"><img src="https://img.shields.io/badge/🖥_WebUI-1f2937?style=for-the-badge" alt="WebUI" /></a>
<a href="#-faq"><img src="https://img.shields.io/badge/❓_FAQ-1f2937?style=for-the-badge" alt="FAQ" /></a>
<a href="https://t.me/COPG_module"><img src="https://img.shields.io/badge/💬_Telegram-2CA5E0?style=for-the-badge" alt="Telegram" /></a>

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
Per‑app device profiles (brand, model, fingerprint, SDK, **serial**) so each game sees the exact
flagship it rewards.

#### ⚙️ CPU Spoofing
Spoof the CPU to flagship‑class silicon for apps that gate features on the chipset.

</td>
<td width="50%" valign="top">

#### 🧩 GOT Hooking
An extra spoofing method (GOT/PLT hooks) for apps that need a deeper layer than the default.

#### 🎛️ Per‑App Comfort Tweaks
Auto **Do‑Not‑Disturb**, **disable auto‑brightness**, **keep screen on** and **stop logging** —
applied only while a tagged game is active, then restored.

</td>
</tr>
</table>

> 🔁 **Add or remove devices, games &amp; apps without a reboot.** ✨ Fully customizable. 🌍 8‑language
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
| **Fortnite** | 120 FPS |
| **Asphalt 9** | 120 FPS |
| **Farlight 84** | Max Graphics |
| _…and 69+ more_ | Premium tiers unlocked |

</div>

#### 📱 App Enhancements
- **Google Photos** — unlimited backup + AI features
- **TikTok** — stream in full 1080p

---

## 🖼️ Screenshots

<div align="center">
<table>
  <tr>
    <td align="center" width="25%">
      <img src="https://github.com/AlirezaParsi/COPG/blob/screenshots/Screenshot_20260614-121040_KsuWebUI.png?raw=true" alt="Dashboard light" />
      <br><sub><b>Dashboard</b> · Light</sub>
    </td>
    <td align="center" width="25%">
      <img src="https://github.com/AlirezaParsi/COPG/blob/screenshots/Screenshot_20260614-121848_WebUI%20X.png?raw=true" alt="Dashboard dark" />
      <br><sub><b>Dashboard</b> · Dark</sub>
    </td>
    <td align="center" width="25%">
      <img src="https://github.com/AlirezaParsi/COPG/blob/screenshots/Screenshot_20260614-121127_KsuWebUI.png?raw=true" alt="Library packages" />
      <br><sub><b>Library</b> · Packages</sub>
    </td>
    <td align="center" width="25%">
      <img src="https://github.com/AlirezaParsi/COPG/blob/screenshots/Screenshot_20260614-121204_KsuWebUI.png?raw=true" alt="Add package" />
      <br><sub><b>Add Package</b> · Spoof + Tweaks</sub>
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

Install from **[MMRL](https://mmrl.dev/repository/zguectZGR/COPG)**, or grab the latest
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
- ➕ **Add Package** — pick any installed app, choose a device profile, toggle **CPU Spoofing /
  GOT Hooking** and the **DND / Auto‑Brightness / Keep‑Screen‑On** tweaks
- 📊 **Dashboard** — live system info: Android, ABI, Zygisk variant, root &amp; kernel
- 💾 **Backup / Restore** &amp; **Sync from GitHub**
- 🎨 **Light / Dark / AMOLED** themes · 🌍 **8 languages** (EN, FA, AR, DE, ES, ID, TH, ZH)

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

Package **tags** are colon suffixes — e.g. `:with_cpu` (CPU spoof), `:got` (GOT hooking),
`:dnd` / `:dab` / `:kso` / `:nolog` (comfort tweaks).

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

## 📈 Activity

<div align="center">

![Repobeats analytics](https://repobeats.axiom.co/api/embed/83b280d0986b3c023ed5f1fdf3f00f77288e3da3.svg "Repobeats analytics image")

<a href="https://www.star-history.com/#AlirezaParsi/COPG&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=AlirezaParsi/COPG&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=AlirezaParsi/COPG&type=Date" />
   <img alt="Star History Chart" width="640" src="https://api.star-history.com/svg?repos=AlirezaParsi/COPG&type=Date" />
 </picture>
</a>

</div>

---

<div align="center">

**If COPG leveled up your games, drop a ⭐ — it really helps!**

Made with ❤️ by **Alireza Parsi** · © 2026 COPG Project

</div>
