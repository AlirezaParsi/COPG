# COPG Spoof Module
#### **Support Group**:
- https://t.me/TheAOSP
#### **Telegram Channel**:
- https://t.me/COPG_module
---
## v6.8.0
*Adds per-user spoofing — a different device identity for each Android user or dual-app clone — keeps your PRO license across updates, and shows the module's real health on the home screen.*

### Per-User Spoofing — new
*   **A different device for each Android user.** If you use dual-apps / app clones (HyperOS, One UI, etc.), a work profile, or a second Android user, you can now give the SAME app a different spoof per user — e.g. the main WhatsApp reads a Pixel while the cloned one reads a Galaxy. Add the app twice, once per user, each on its own device profile.
*   **The picker finds your clones.** The app picker now has an **"Apps for"** dropdown that lists every Android user on the phone. Owner apps look normal; a cloned copy shows a small **"User 10"** tag so you can tell them apart. Picking one automatically scopes it to that user.
*   **Launch & clear-data hit the right copy.** The restart and clear-data buttons now act on the exact user the entry is scoped to — clearing a clone no longer wipes the main app.
*   **Each clone gets its own identity.** Derived values (Android ID, serial, advertising / App Set ID, IMEI) are keyed per user, so two clones of the same app never share an ID.

### PRO license now survives updates — fix
*   **No more re-importing after every update.** Your device-locked PRO license is now carried over automatically when you update the module, and backed up so it also survives a full uninstall + reinstall. Update and your PRO features stay on.

### Real module status on the home screen — new
*   **See if the module is actually working.** The status badge no longer just checks that the folder exists — it now reflects the real state: **Active** (loaded and running this boot), **Reboot to activate** (just installed), **Not loaded** (didn't start — bad install / needs attention), **Disabled**, **Uninstalling**, or **Zygisk off**. Colour-coded green / amber / red / grey.

### Country flags in the pickers — polish
*   **Flags everywhere you pick a country.** The SIM/carrier, language, timezone and GPS location pickers now show a country flag next to each entry, so the right one is easier to spot.

### Languages
*   **New: Português (Brasil).** The WebUI is now fully translated to Brazilian Portuguese — pick it in Settings → Language.
*   **All translations updated.** Every language (English, فارسی, العربية, Deutsch, Español, Português, Bahasa Indonesia, ไทย, Türkçe, 中文) is brought fully up to date with the new features and screens — 10 languages, complete coverage.

### Fixes
*   **Scrolling in the editors.** The Add/Edit dialogs couldn't be scrolled on some phones — fixed, the whole form scrolls again.
*   **Timezone list.** Added Europe/Lisbon (Portugal) to the built-in timezones.

---
## v6.7.0
*Adds a per-app GPS location spoof, a fake Play install source, and a PairIP license-check block for running paid / locked apps.*

### GPS Location Spoof — new PRO feature
*   **Put any app anywhere on the map.** A new **GPS** option in the package editor makes an app read a location you pick — choose a country/city from the built-in list (135 places) or drop in your own latitude/longitude. It covers the normal location APIs **and** Google's Fused/GMS provider, so map, weather, dating, delivery and region-locked apps see the spoofed spot.
*   **Only the coordinates change.** Latitude and longitude are overwritten; accuracy, altitude and speed stay real, so the fix still looks natural. No need for a separate mock-location app or Developer Options.
*   **Resident — use with care.** This hook stays mapped while the app runs, so a strict anti-cheat CAN detect it. It sits behind a red *"use at your own risk"* confirmation and should **never** be used on anti-cheat games. PRO, arm64 and arm32.

### Fake Play Install Source — new PRO feature
*   **Make an app look Play-installed.** A new **Fake Play Install Source** toggle makes an app read its installer as the Google Play Store (`com.android.vending`) — for apps that gate features or trust on a "genuine Play install".
*   **Invisible to the app.** It works in the system, not inside the app — nothing of the module is loaded into the app's process, so there's nothing on-device to detect. Applies at boot; no residency.

### Block PairIP License Check — new PRO feature
*   **Run paid / locked apps.** A new **Block PairIP License Check** toggle stops Google's PairIP "you don't own this app / buy it on Play" screen from appearing, so the app opens normally.
*   **Done out of process — undetectable.** The block happens in the Android system, not inside the app, so PairIP's in-app anti-tamper can't see it. **Safe / not detectable.** It works for apps where the license screen is a separate step; if that screen is the app's own launcher or the check is built into native code, it won't help — just turn it off if an app misbehaves. PRO.

---
## v6.6.0
*Adds a device-wide "Disable FLAG_SECURE" so you can screenshot and screen-record apps that normally block it.*

### Disable FLAG_SECURE — new free feature
*   **Screenshot and record any app.** A new **Disable FLAG_SECURE** switch in Settings → Global Hooks lets you capture apps that block screenshots and screen recording (banking, chat, wallets, incognito tabs).
*   **Invisible to the app.** It works in the system, not inside the app — the app still thinks its screen is protected and can't tell the flag was lifted, so there's nothing in-app to detect. It's **device-wide** (every app), toggled on/off with no reboot.
*   **One caveat.** Protected streaming video (Netflix / Disney+ and other hardware-DRM playback) still records black — that block lives in the hardware, below where any app-level switch can reach.

### No Storage Restrict — new free feature
*   **Pick Android/data, obb, the SD-card root and Download in the file picker.** Android 11+ blocks selecting those folders through the system file picker (Storage Access Framework). A new **No Storage Restrict** switch in Settings → Global Hooks removes that block so file managers and backup apps can grant access to them again.
*   **Works where the old Xposed module doesn't.** Newer Android renamed the internal check the popular module hooks, which is why it stopped working / needed forcing. COPG hooks the current name (and the old ones), so it keeps working across Android versions. Device-wide, toggled on/off with no reboot.

### Under the hood — bootloop protection
*   **Auto-recovery for the system-level hooks.** Disable FLAG_SECURE, VPN-hide and per-app Language all run inside the Android system. If a phone-specific quirk ever made one of them crash the system at startup, the module now notices after a few boots and starts that hook **off by itself** so the device boots normally — no recovery flash, no data loss. Your settings are kept and the feature comes back on the next clean boot.

---
## v6.5.0
*Adds a per-app proxy (route one app through your own SOCKS5/HTTP/SOCKS4 exit — no VPN, no interface), a full SIM/carrier overhaul with a phantom-SIM mode and 140 carriers, and closes a device-fingerprint leak that some detectors were catching.*

### Per-App Proxy — new PRO feature
*   **Route just one app through your own proxy.** Point any app at your **SOCKS5, HTTP, or SOCKS4** proxy so its traffic exits from that IP and country — the rest of your phone stays on your real connection. Pair it with a matching SIM/region so device, SIM and IP all agree.
*   **Stealthy and ban-safe.** It works at the kernel level: **no VPN, no network interface the app can see, and nothing runs inside the app** — so there's nothing on-device to detect. The only thing that matters is the proxy you pick (use a clean/residential one; a flagged datacenter IP can be blocked by strict services — that's the proxy's reputation, not the module).
*   **Manage proxies in the app.** A new **Proxy** picker lets you save proxies (name, type, host, port, optional username/password) and assign one per app. No reboot — it applies within seconds.

### SIM / Carrier spoof — big overhaul
*   **Phantom SIM — make a SIM appear on an empty slot.** A new **Add SIM** mode makes a device with no SIM (or a free slot) report a full SIM present — status, carrier name, operator code and country all show up in device-info apps.
*   **Carrier and country now land everywhere.** Fixed cases where the operator code or country stayed blank in some apps (e.g. DevCheck) even with the carrier spoof on. Status, name, code and country now report consistently.
*   **140 carriers across 67 countries.** The carrier list grew from 12 to **140** real carriers worldwide — pick any, or add your own.

### Device consistency — detection fix
*   **Closes a fingerprint leak some detectors caught.** With COW prop spoofing on, only the *base* device props were faked while their **partition copies** (system / vendor / odm / product / system_ext) kept the real values — a multi-source mismatch some detection tools flagged. Now every partition copy of the device fields **and** all nine build-fingerprint variants (including `ro.system.build.fingerprint` and `ro.product.build.fingerprint`) are faked together, so there's no cross-source conflict. Thanks to the users who reported this.

### Other
*   **Clear data, per app.** Each installed app in the list now has a **Clear data** button — it wipes the app's stored data (logins, cache, saved IDs) so the next time you open it, it re-reads your spoofed device as a brand-new install. Confirm-gated, since it can't be undone.

---
## v6.4.0
*Adds a display refresh-rate spoof, brings every advanced spoof to Android 12–15, rebuilds VPN-hide for wider coverage, and unlocks high-FPS modes on more games.*

### Display refresh-rate spoof — new free feature
*   **Report any refresh rate you like.** A new **Refresh Rate** option in the package editor makes an app read a custom screen refresh rate (e.g. 120, 144, 165 Hz) instead of your real panel's — handy for device-info apps and for games that show or gate on the display's Hz.
*   **Covers the whole display API.** Both the current refresh rate and the full list of supported display modes report the spoofed value, so apps that read the *maximum* rate see it too.
*   **Resident — use with care.** This hook stays mapped while the app runs, so a strict anti-cheat CAN detect it. It sits behind a red *"use at your own risk"* confirmation and should **never** be used on anti-cheat games. Free, arm64 and arm32.

### Every advanced spoof now works on Android 12–15
*   **Big fix for older Androids.** The resident spoofs — VPN-hide, mock-location, Advertising ID, Widevine/DRM, IMEI, refresh rate and more — silently did nothing on Android 12 through 15; they only took effect on Android 16. They now work on **Android 12 and up**. If any of these did nothing for you before, update and try again.

### VPN-hide rebuilt for wider coverage
*   **Hides your VPN from more apps.** The aggressive VPN-hide (**vpnn**) now covers both the Java and the native ways an app can spot a VPN in a single toggle — including checks that read the network-interface list, which some detectors used to slip through.
*   **More compatible under protected apps.** Its hooking engine was moved to the safer method the rest of COPG already uses, improving compatibility with hardened/anti-tamper apps.
*   **Safe VPN-hide unchanged.** The lighter, anti-cheat-safe VPN-hide is still there — use that one on games where a ban is a risk.

### More devices can install & activate
*   **Fixes install and license activation on phones with an unusual CPU-ABI list.** Some devices report their architecture in a non-standard way, which could abort the install or block activation. The installer now detects the architecture reliably across all supported phones.

### New device profiles
*   **Arena Breakout — high-FPS unlock.** A **Black Shark** profile matched to Arena Breakout's device whitelist unlocks its higher frame-rate options. It's applied as a stealth Build-only spoof — nothing stays in the app's memory, so it's anti-cheat-safe.
*   **Honor of Kings — high-FPS unlock.** A new **Xiaomi 14** profile is matched to Honor of Kings' device list to unlock its higher frame-rate options, again as a stealth Build-only spoof.

---
## v6.3.1
*Bug-fix release — fixes license activation on some newer devices.*

### Fixes
*   **License now activates on every device (Samsung Galaxy S25 Ultra & others).** On phones whose CPU‑ABI list reports a non‑standard name, the installer could fail to select the licensing helper — leaving buyers stuck on **"Could not read device fingerprint"** with no way to get their device code or activate PRO. The installer now falls back to the matching helper for your architecture and verifies it was installed, so activation works everywhere. If you hit this error, just reinstall this version.
*   **Device share codes now include the baseband.** Sharing a device profile now also carries the **Baseband** (`Build.RADIO`) value, so an imported profile matches the original exactly.

---
## v6.3.0
*Adds a per-app browser identity (User-Agent) spoof, plus baseband and per-app serial to round out the device fingerprint.*

### Browser identity (User-Agent) spoof — new PRO feature
*   **Give each app its own browser User-Agent.** A new **User-Agent** row in the package editor makes any WebView-based app report a **different browser/device UA string** — useful for sites and in-app web pages that gate content, layout or pricing on the browser identity.
*   **Named profiles, reusable across apps.** Pick from a searchable list of built-in UA profiles or tap **＋** to add your own string — saved and reusable across apps, just like the CPU, GPU, timezone and language lists.
*   **Overrides apps that set their own UA.** As well as the default WebView User-Agent, it re-applies the fake one when the app loads a page, so apps that force their own UA still see the spoofed value.
*   **Resident — use it with care.** This hook stays mapped in the app's memory while it runs, so a strict anti-cheat CAN detect it. It sits behind a red *"use at your own risk"* confirmation and should **never** be used on anti-cheat games. **PRO**, arm64 and arm32; hidden on x86.

### Baseband (radio) version spoof — new feature
*   **Fake the modem/baseband version.** The device editor's **Advanced build props** now includes a **Baseband** field (`Build.RADIO`), so apps that read the radio firmware version see the value you set instead of your real modem's.
*   **One-tap Generate.** A **Generate** button fills a realistic Qualcomm-style baseband string matched to the profile, or type your own.
*   **Stealth, and free.** It's applied as a normal Build field plus its matching system property, and the module unloads before the app starts — nothing of COPG stays in memory. No risk warning, no license needed.

### Per-app serial — now per package
*   **A different serial for each app.** Serial spoofing was previously device-profile only; a new **Serial** toggle in the package editor now gives an app its **own** `Build.SERIAL`, derived from the package so it's stable per app and never shared between two apps.
*   **Plays nice with device profiles.** If the chosen device profile already sets a serial, the per-app toggle is disabled (the profile value wins) — no conflicting values.
*   **Stealth, and free.** Field-set plus a copy-on-write `ro.serialno`, zero residency. *(Note: modern Android hides the real serial from ordinary apps, so this mainly matters for apps still reading the legacy field.)*

### Per-app screen DPI — new tweak
*   **Give a game its own screen density.** A new **Screen DPI** field in the package editor's **Tweaks** section sets the display density while that app is open — a lower DPI fits more on screen (handy for some games' UI), a higher one makes everything bigger.
*   **Saves and restores your real DPI automatically.** Your current density is backed up the first time it applies and restored when you close the app — you never have to set it back by hand.
*   **Held while the app runs, no switch lag.** The density is applied once when the app opens and **kept while it's running**, including when you switch to other apps — it isn't re-applied on every app switch, so moving between apps stays smooth. It's put back only when the app is closed. Valid range 120–640; leave blank to turn off. **Free**, nothing loads into the app.

---
## v6.2.5
*Adds per-app language & region spoofing — make an app read (and render in) another language and country, without changing your real system language.*

### Language & region spoof — new feature
*   **Give each app its own language.** A new **Language** row in the package editor makes an app read a **different language & country** than the rest of your phone — useful for region-locked content, store pricing and language-based checks. Where the app ships that language's text, its UI actually **displays** in it too.
*   **Built-in locales or add your own.** Pick from a searchable list of common locales (English–US/UK, Spanish, Portuguese–Brazil, French, German, Arabic, Japanese, Chinese, …) or tap **＋** to add any BCP-47 tag (e.g. `pt-BR`, `es-MX`, `zh-Hans-CN`) — saved and reusable across apps, just like the CPU, GPU and timezone lists.
*   **Applied the stealthy way — nothing loads into the app.** The language is injected the same way Android's own per-app-language setting works, from the **system side**. Nothing of COPG loads into the app's own process, so it's invisible to anti-cheat / integrity memory scans and safe for hardened apps. **Free**, no risk warning.
*   **No reboot to change.** Turn it on and relaunch the app; adding or changing a language applies without a reboot.

### Resident spoofs — new hooking engine (works in more apps)
*   **Mock-Location, Advertising ID, App Set ID, Widevine (DRM) and per-app IMEI now use a new hooking engine.** These resident spoofs were moved to a hooking technique that no longer conflicts with **LSPosed / Xposed** and keeps working inside more **Play-protected** apps that used to crash — so they apply reliably where they previously failed. The spoof behaviour itself is unchanged, and they are still resident, so the same red *"use at your own risk"* rules apply and they should never be used on anti-cheat games.

---
## v6.1.2
*Adds per-app timezone spoofing — make an app believe your phone is in another part of the world, without touching the real system clock.*

### Timezone spoof — new feature
*   **Give each app its own timezone.** A new **Timezone** row in the package editor makes an app read a **different timezone** than the rest of your phone — clocks, calendars, timestamps and anything using the modern date/time APIs all shift to the zone you pick, while your real system time stays untouched.
*   **Built-in zones or add your own.** Pick from a searchable list of common zones (New York, London, Dubai, Tokyo, …) or tap **＋** to add any IANA timezone ID (e.g. `America/Toronto`) — your custom zones are saved and reusable across apps, just like the CPU and GPU model lists.
*   **Covers native readers too.** As well as the app's Java/Kotlin timezone, it forges the `persist.sys.timezone` property, so native code that reads the zone directly sees the fake one as well.
*   **Stealth, and free.** The value lives in the app's own runtime and the module unloads before the app starts, so nothing of COPG stays in memory — safe against anti-cheat scans. No risk warning, no license needed.

### Fake uptime — new PRO feature
*   **Make an app think the device has been running for days.** A new **Fake Uptime** toggle in the package editor adds the number of days you choose **on top of** the real device uptime, so the app sees a much longer uptime — useful against apps that treat a freshly-booted / freshly-reset device as suspicious (anti-fraud, referral and reward flows).
*   **Doesn't reset when you close the app.** Because the fake uptime rides the real boot clock, it keeps ticking and stays high across app closes / relaunches — it only drops on an actual reboot. It shifts both `SystemClock.elapsedRealtime` and the native `sysinfo` / `clock_gettime` readers, so Java and native code agree. Your real clock, date and video playback are left untouched.
*   **Resident — use it with care.** Like GPU / Mock-Location spoofing, this hook stays mapped in the app's memory while it runs, so a strict anti-cheat CAN detect it and may **ban you**. It sits behind a red *"use at your own risk"* confirmation and should **never** be used on anti-cheat games. **PRO**, arm64 and arm32; hidden on x86.

### Fixes
*   **Emulator licenses are more robust.** Changing an emulator's virtual device settings no longer wrongly deactivates a valid license. *(Real phones are unaffected.)*

---
## v6.0.0
*Completes the identity-spoof suite — after Advertising ID and Android ID, each app can now carry its own **App Set ID** too, the last of the Play-Services fingerprint IDs.*

### App Set ID spoof — new PRO feature
*   **Give each app its own App Set ID.** A new **App Set ID** toggle in the package editor makes an app read a **different Google App Set ID** than the rest of your phone. App Set ID is a resettable ID that ad, analytics and anti-fraud SDKs (and device-fingerprinting libraries) read through Play Services to tie your installs together — spoofing it per app makes each one look like a separate device, for reward / referral / multi-account and anti-fingerprint use.
*   **Automatic per-app ID, or pin your own.** Turn it on and the app gets a stable App Set ID derived from its own package — nothing to fill in. Or type / generate an exact UUID to pin a value, or re-roll that app's ID whenever you want.
*   **Resident — use it with care.** Like GPU / Advertising-ID / DRM spoofing, this hook stays mapped in the app's memory while it runs, so a strict anti-cheat CAN detect it and may **ban you**. It sits behind a red *"use at your own risk"* confirmation and should **never** be used on anti-cheat games. **PRO**, arm64 and arm32; hidden on x86.

### New device profile
*   **HUAWEI Mate 80 Pro Max (SGT-AL10).** Added a ready-made device profile for Huawei's HarmonyOS flagship — pick it in the package editor to make an app read a Mate 80 Pro Max identity (brand / model / fingerprint).

### New CPU profile
*   **Snapdragon 8 Elite Gen 5 (SM8850).** Added a ready-made CPU model for Qualcomm's latest flagship, from a real device capture — pick it in the package editor's CPU model list to make an app read an 8 Elite Gen 5 CPU.

### Interface
*   **Cleaner Widevine level picker.** The DRM security-level control (L1 / L2 / L3) is now a segmented selector that matches the rest of the app, instead of a plain system dropdown.
*   **"Don't show this again" on the risk warnings.** Each *"use at your own risk"* confirmation (GPU, Mock-Location, DRM, IMEI, Advertising ID, App Set ID, aggressive VPN / SIM) now has a **remember-my-choice** checkbox. Tick it once and that spoof stops re-prompting — the choice is remembered per spoof, so the others still warn you the first time.
*   **All 9 languages updated.** App Set ID, Advertising ID, DRM, IMEI, Global IMEI and the VPN sub-toggle are now fully translated in every language (Arabic, German, Spanish, Indonesian, Thai, Turkish, Chinese — English and Persian already covered).

### Fixes
*   **Android ID spoof now works on Android 12 / 12L.** On those versions the per-app **Android ID** forge could silently fail to apply (their settings cache tracks value generations differently) — it's now handled, so the set or derived Android ID takes on Android 12 / 12L too.
*   **Hide VPN (safe mode) applies without a reboot.** Turning on the safe **Hide VPN** now takes effect immediately — no more rebooting first before an app sees the VPN stripped.

---
## v5.9.0
### Global IMEI — device-wide IMEI spoof (new PRO feature)
*   **One IMEI for the whole device.** A new **Global IMEI** panel (Settings → Global Hooks) makes *every* app — plus Settings, the dialer and `*#06#` — read the same fake IMEI. Unlike the per-app IMEI toggle, this hooks the telephony service itself (`com.android.phone`), so it applies device-wide. Set the **SIM 1** IMEI and, on dual-SIM, a separate **SIM 2** IMEI.
*   **App-invisible — no anti-cheat risk.** Because the hook lives in a **separate system process** apps can't scan, it's invisible to in-app anti-cheat / integrity memory checks — so unlike GPU / Mock-Location / per-app IMEI there is **no red "use at your own risk" gate**. It is still device-wide (it changes the IMEI for every app, including banking, until you turn it off) and **not tower-level** — the modem still reports the real IMEI to the network.
*   **PRO.** Locked on the free tier. Leave the fields blank to keep the real IMEI; toggling it restarts the telephony service to apply.

### Hide VPN — pairip-safe mode is back (per-app, doesn't crash LSPosed)
*   **The main Hide VPN is now the safe one.** v5.8.0 removed the system-service VPN hide because the old global version **clashed with LSPosed and could crash it** (both patched the same core runtime). It's back — now **scoped to the app by UID** and built on a different hook engine that **coexists with LSPosed by design (no runtime clash), so it no longer crashes LSPosed**. **Hide VPN** hooks the connectivity **source** in the system and hands *only that app* a VPN-stripped network. Nothing loads into the app's own process, so **anti-tamper / integrity scans can't see it and banking / pairip apps don't crash** — this is the one to use for banking and hardened apps. **Free, works on all CPUs — reboot after enabling.**
*   **Aggressive (in-app) is now a sub-option.** The older in-app hook — which strips the VPN from inside the app and also covers a native `getifaddrs` read — moved to a nested **Aggressive (in-app)** toggle under Hide VPN, behind a red *"use at your own risk"* warning. It stays **resident and detectable** (anti-cheat can ban, pairip apps crash), so use it only for apps the safe layer doesn't fully cover. arm64 / arm32.
*   **What each covers.** Safe mode covers the modern `getNetworkCapabilities → hasTransport(VPN)` check; the aggressive sub adds the legacy network-type and native-interface paths.

### Per-app Advertising ID (GAID) spoof — new PRO feature
*   **Give each app its own Google Advertising ID.** Alongside the device-wide Advertising ID control in Settings, you can now spoof the GAID **per app** from the package editor. Every app you turn it on for reads its **own distinct advertising ID** — made for ad / reward / referral / multi-account apps where each install should look like a different device.
*   **Automatic per-app ID, or pin your own.** Turn it on and the app gets a stable ID derived from its own package — nothing to fill in. Or type / generate an exact UUID to pin a specific value, or to re-roll that app's ID whenever you want.
*   **Resident — use it with care.** Like GPU spoofing and Mock-Location hide, this hook stays mapped in the app's memory while it runs, so a strict anti-cheat CAN detect it and may **ban you**. It sits behind a red *"use at your own risk"* confirmation and should **never** be used on anti-cheat games. Runs on **arm64 and arm32**; hidden on x86.
*   **PRO feature.** Unlocks with a COPG PRO license alongside GPU / Android ID / COW / SIM / Mock-Location. On the free tier the row shows a lock. The device-wide Advertising ID reset in Settings stays **free**.

### Widevine (DRM) level spoof — new PRO feature
*   **Report a higher DRM security level.** A new **DRM / Widevine** toggle in the package editor makes an app read your device's Widevine security level as **L1** (or an exact level you pick — L1 / L2 / L3), and lets you spoof the Widevine **device ID** and **system ID** too. Useful for apps that gate features on, or simply display, the DRM level.
*   **Fools the check, not the content pipeline.** This changes what the app *reads* for the security level — it does **not** grant real hardware HD DRM (that's decided server-side by provisioning). Treat it as a display / gate spoof.
*   **Resident — use it with care.** Like GPU and Advertising-ID spoofing, this hook stays mapped while the app runs, so a strict anti-cheat CAN detect it. It sits behind a red *"use at your own risk"* confirmation and should **never** be used on anti-cheat games. **PRO**, arm64 and arm32.

### IMEI / device ID spoof — new PRO feature
*   **Fake the IMEI per app.** A new **IMEI** toggle makes an app read a **fake IMEI / device ID** instead of your real one. Turn it on for an automatic, Luhn-valid IMEI derived per app (stable for that app), or type an exact 15-digit IMEI to pin one. Dual-SIM devices can set a second IMEI for slot 2.
*   **Only where the app can actually read it.** Since Android 10 a normal app needs a privileged permission to read the IMEI at all, so most apps already can't — this covers the apps that legitimately can (with phone permission) and makes them see the fake value instead of the real one.
*   **Resident — use it with care.** This hook stays mapped while the app runs, so a strict anti-cheat CAN detect it — behind the red *"use at your own risk"* confirmation, **never** for anti-cheat games. **PRO**, arm64 and arm32.

### Android ID & Advertising ID — simpler, no device seed
*   **No more seed fields.** The per-device **Android ID seed** field has been removed (and the new Advertising ID never needed one). Each app now automatically gets its **own** Android ID / Advertising ID derived from the app itself, so two apps never share one — matching how real Android works. One less thing to set up.
*   **Still pin an exact value.** You can always type or generate an exact Android ID / Advertising ID for a single app — to match another phone, or to re-roll that app's identity.
*   **Old profiles keep working.** Device profiles that still carry an old Android ID seed load fine; the seed is simply ignored and the per-app value is used instead. That app's Android ID may change once, since it no longer mixes in the old seed.

### Fixes
*   **Global IMEI: SIM 2 no longer mirrors SIM 1.** `*#06#` and dual-SIM apps now show the correct per-slot IMEI.
*   **Global IMEI: PRO license now recognised in the phone process.** A valid license is no longer rejected for the telephony hook.
*   **DRM device ID shown in Base64.** The Widevine device-ID field now uses the same **Base64** format DRM-info apps display, so what you set matches what the app shows.

---
## v5.8.0
### Hide VPN — reworked (fixes crashes with LSPosed)
*   **Fixes the crash some phones saw after v5.7.9.** The previous "system-service" VPN hide installed a hook inside Android's system_server, which clashed with LSPosed (both hook the same core runtime) and could crash LSPosed on some devices while working on others. That path is **removed**. VPN hide now runs **per-app only** — no system_server hook, so LSPosed is left alone.
*   **VPN hide is now an in-app hook.** Turning on **Hide VPN** hooks the app's own network APIs (ConnectivityManager / NetworkCapabilities / NetworkInfo / LinkProperties / NetworkInterface) and strips the VPN out. Because it runs inside the app, it is **resident and detectable**: a strict anti-cheat can spot it and may **ban you**, and hardened anti-tamper apps (banking / Play-Integrity / pairip) can **crash** with it on. So the toggle now sits behind a red *"use at your own risk"* confirmation — use it only for normal device-info / network apps, **never** for competitive games or banking. (The old "safe for banking" system-service mode no longer exists.)
*   **Simpler VPN control.** The separate **Deep hide** sub-toggle was removed — the base Hide VPN is already resident, so the extra layer was the same risk class. One toggle now. Runs on **arm64 and arm32**.

### Hide Developer Options — now also hides USB debugging from bank apps
*   **Covers the properties, not just the settings.** Hardened apps (e.g. some banks) don't read Developer Options through the normal Android Settings — their native protection reads the **USB-debugging system properties** (`sys.usb.config`, `adbd`, …) directly, so **Hide Developer Options** used to be bypassed even with it on. It now also spoofs those properties to a non-debug state, per app, still fully stealth (nothing of COPG stays in memory). So a bank app that refused to run with USB debugging on should now start normally. Still **free**.

---
## v5.7.9
### Hide Developer Options & USB Debugging — new, free
*   **Make an app think Developer Options are off.** A new **Hide Developer Options** toggle in the package editor makes the app read Developer Options *and* USB Debugging as **OFF** even while they're on — handy for apps that refuse to run or nag when they detect a developer phone.
*   **Fully stealth, safe for anti-cheat.** The value is forged in the app's own settings cache and the module unloads before the app runs, so **nothing of COPG stays in memory** — anti-cheat scans see a clean process. It covers the standard settings check apps use; a hardened app that reads the settings provider directly can still see the real state. **Free** — no PRO license needed.

### Hide VPN — new, free
*   **Hide that you're on a VPN.** A new **Hide VPN** toggle stops an app from seeing an active VPN — the system reports no VPN connection, so apps that block or limit VPN users behave normally.
*   **Safe for banking & anti-tamper apps.** Hide VPN runs entirely inside Android's system service, stripping the VPN out of the network info *before* it reaches the app — nothing of COPG runs inside the app itself. Banking / Play-Integrity / pairip-protected apps won't crash and can't detect it by scanning their own memory. Covers the standard connectivity checks. **Free**, off by default.
*   **Deep hide (optional, risky).** A sub-switch adds a full in-app hook that also covers apps which read the raw network-interface list (e.g. device-info tools that see "tun0"). This layer stays mapped in the app, so a strict anti-cheat CAN detect it and may **ban you** — the bigger risk — and hardened anti-tamper apps will crash with it on. It sits behind a red *"use at your own risk"* confirmation and should **never** be used on anti-cheat games. Runs on **arm64 and arm32**.

### Share a device setup as a code — new, free
*   **Send a whole device to anyone with one code.** Every device now has a **Share** button that turns its full identity — brand, model, fingerprint, build fields, serial, Android ID seed and the rest — into a short code you can copy and send. The other person taps **Import**, pastes it, and the device appears instantly: no more typing every field by hand. Free, and works even in browser preview.
*   **Share the games too, if you want.** When you share a device that has packages, an **"Include packages"** switch also sends its app list — with each app's spoof settings (CPU / GPU / COW / Android ID / tweaks) — so the other person gets your exact per-app setup, not just the bare device.
*   **No accidental duplicates.** If a shared package already belongs to another device on the importer's phone, COPG asks whether to **move it to the new device** or **keep it on the existing one**. A package can only live on one device, so your spoofs stay correct instead of clashing.

### Set an exact Android ID per app
*   **Pin a specific Android ID.** Besides the automatic per-app Android ID, you can now type or generate an **exact 16-character Android ID** for a single app — no device seed needed — for example to make a second phone read the same ID as your first.

### New device profile
*   **Galaxy Tab S9 Ultra — unlock 120 FPS in BGMI / PUBG Mobile.** Added the Samsung Galaxy Tab S9 Ultra (Snapdragon 8 Gen 2) profile with **BGMI pre-assigned to it**. Spoofing your device as the Tab S9 Ultra makes BGMI / PUBG Mobile offer the **120 FPS** graphics option that is normally locked to a short list of flagship devices. The CPU stays real (ban-safe) — only the device identity is faked, and you can send this whole setup to a friend with the new device-share code.

### Fixes
*   **No more boot loops.** A build change in a recent version could leave some setups (certain Magisk / Kitsune configurations) stuck in a boot loop. That change has been reverted — the module boots cleanly again.
*   **CPU spoof stays applied when you switch apps.** The spoofed `/proc/cpuinfo` could fall back to the real CPU after you left a game and returned to it. COPG now re-applies the fake CPU while the game is in the foreground, so it sticks across app switches.

### Other
*   **Clearer Android ID help, fully translated.** The Android ID descriptions now explain that the device field is a *seed* and that a per-app exact ID needs no seed. All new text — hide developer options, VPN hide, device sharing — is translated across every supported language.

## v5.7.1
### Hide mock location — new PRO feature, per app
*   **Make an app accept your mock GPS as real.** A new **Hide Mock Location** toggle in the package editor stops an app from seeing that your location comes from a GPS-spoofer / joystick app — `Location.isMock()` and the mock-provider flag read false, so location apps and games treat the coordinates as genuine.
*   **Covers every way the location arrives.** It clears the mock flag on the Location object itself — not just one getter — so it works whether the app reads through the system location manager, Google's fused provider, or a native / Unity caller. An app that builds its own mock Location to *test* for tampering still behaves normally, so the hook doesn't give itself away.
*   **Resident — use it with care.** Like GPU spoofing, this hook stays mapped in memory while the app runs, so a strict anti-cheat CAN detect it. It sits behind a red *"use at your own risk"* confirmation and should **never** be used on anti-cheat games. It also does **not** beat server-side movement / speed checks — the location must still move at human speed.
*   **Works on 32-bit devices too.** Mock-location hide now runs on both **arm64 and arm32 (armeabi-v7a)** phones. On x86 / x86_64 (emulators) the toggle is hidden, since there's no supported hooking backend there.
*   **PRO feature.** Unlocks with a COPG PRO license alongside GPU / Android ID / COW / SIM. On the free tier the row shows a lock.

### Fixes
*   **Aggressive SIM — network country no longer shows your real region.** With an aggressive SIM carrier set, the *Network* country in device-info apps kept showing your real country instead of the spoofed carrier's. Some apps read the network country through a binder call the previous carrier hook didn't cover; it's now intercepted too, so the network country matches the spoofed carrier.
*   **Translation parity across all 9 languages.** The *delete backup* strings now appear translated instead of in English on Arabic, German, Spanish, Indonesian, Thai, Chinese and Turkish.

### Other improvements
*   **License card shows days remaining.** Settings → License now displays how many days are left on your PRO license.
*   **Delete backups from the app.** The Backup & Restore sheet now lets you delete individual backups you no longer need.

## v5.6.1
### Fixes
*   **Free tier can no longer open the GPU / SIM profile pickers.** On the free tier, tapping a package's GPU or SIM carrier picker now shows the upgrade prompt instead of opening the add / edit list. (Spoofing itself was already blocked for free users — this just keeps the PRO editors out of the free UI.)

## v5.6.0
### Carrier / SIM spoof — new PRO feature, per app
*   **Fake the SIM carrier per app.** A new **SIM / Carrier** toggle in the package editor makes an app read a different network operator — carrier name, operator code (MCC/MNC), and country. Pick from a built-in list of carriers, or **add your own** (name / MCC / MNC / country) and edit or delete them later, just like the CPU and GPU pickers.
*   **Two modes, you choose the trade-off.** **Safe** rewrites the carrier the way most apps read it — fully **stealth**, zero memory residency, safe even for anti-cheat games. **Aggressive** additionally covers apps that read the SIM through the newer subscription API; it **stays mapped while the app runs, so it can be detected** — it sits behind a red *"use at your own risk"* confirmation and must **never** be used on anti-cheat games.
*   **A different carrier per SIM slot.** On a dual-SIM device you can give SIM 1 and SIM 2 different carriers, or set one carrier for both.
*   **PRO feature.** SIM spoofing unlocks with a COPG PRO license, alongside GPU / Android ID / COW. Safe mode is fully stealth like COW; on the free tier the row shows a lock.

### New CPU models
*   **Three more CPU profiles** you can spoof per app — **HiSilicon Kirin 9030S**, **MediaTek Dimensity 8350**, and **MediaTek Dimensity 9400+**. Pick any of them (or import your own `cpuinfo`) from a package's CPU Model list.

### GPU spoof presets
*   **Two more games ship pre-configured for GPU spoofing (Adreno 830)** — **Blood Strike** and **Where Winds Meet**. GPU spoofing stays a **PRO**, resident feature: the preset only takes effect with a PRO license, and like all GPU spoofing it stays mapped while the game runs, so use it at your own risk.

### Advertising ID control (free)
*   **Reset, set, or restore your Google Advertising ID** from Settings → Privacy. View the current ID, generate a random one, type a custom one, or restore your real ID — it's backed up automatically the first time you change it. Applies device-wide to every app, no reboot.

### More languages
*   **Turkish added**, and every community translation (Arabic, German, Spanish, Indonesian, Thai, Chinese) brought fully up to date. COPG now ships in **9 languages**.

### Android ID and prop spoof are now PRO features
*   **Per-app Android ID spoofing (`:aid`) now needs a COPG PRO license** — the same license that unlocks GPU. Unlike GPU it stays fully **stealth** (forged then unloaded, zero memory residency), so it's safe even for anti-cheat games. On the free tier the toggle shows a lock; get a license to enable it.
*   **COW prop spoofing (`:cow`) is now PRO too** — also fully **stealth** (props forged then the module unloads, zero residency, anti-cheat safe). Locked on the free tier.
*   Everything else stays **free**: CPU model spoof, device profiles, Build/serial fields, block CPU, and all the tweaks.

### Fixes
*   **Aggressive SIM — SIM 1 country no longer shows blank** in some device-info apps. The carrier injection was opening hidden-API access for the whole app too broadly, which changed how those apps looked up the network country; it's now scoped to only what the feature itself needs, so the app's own lookups behave normally again.

## v5.5.1
### GPU spoofing (per-app) — PRO feature, use at your own risk
*   **GPU spoofing is unlocked with a COPG PRO license.** Every other spoof (CPU, COW props, Android ID, block CPU, tweaks) stays **free** — GPU is the one PRO-gated toggle. On the free tier the GPU row shows a lock; get a license to enable it.
*   **Fake the GPU per app — and it actually reaches the game now.** A **GPU Spoofing** toggle in the package editor lets a game read a *different* graphics chip across **OpenGL ES** (`GL_RENDERER`, `GL_VENDOR`, `GL_VERSION`, GLSL version), **EGL** (`EGL_VERSION`), and **Vulkan** (device name, vendor ID, device ID, API version, driver version, features, and reported VRAM). Games that resolve their graphics calls through the driver dispatcher are now covered — the spoof lands where it did nothing before.
*   **Full profile editor.** It opens a manifest-driven **GPU profile picker** where profiles can be added, **edited**, and deleted. Each profile has the basics (name / renderer / vendor / Vulkan vendor ID) plus a collapsible **Advanced spoofs** section for the GL/EGL/Vulkan extras. The two risky ones — **Fake GL extensions** and **Force all Vulkan features** — sit behind their own warnings, since claiming a capability the real GPU lacks can crash a game.
*   **Gated behind a clear warning — on purpose.** Unlike every other spoof in COPG, GPU spoofing **stays mapped in the game's memory while it runs**, so it *can* be detected. Turning it on pops a red **"use at your own risk"** confirmation with a 5-second cooldown. **Do not enable it for anti-cheat games** — it's meant for benchmarking, compatibility checks, and curiosity, not for getting past bans.
*   **Hidden where it can't work.** The spoof's hook is ARM-only, so the toggle auto-hides on x86 / x86_64 (Intel) devices and emulators where it can't attach.
*   **See your real GPU.** The System tab now shows the device's actual GPU so you know what you're spoofing away from.

### Unified package model — one kind of package, tags for everything
*   **Every package is now a device package.** The old **Device / CPU-Only / Blocked** types are gone. Each app sits on a device profile and you flip exactly the spoofs you want — CPU, GPU, COW props, Android ID, Block CPU — as independent toggles in a tidy **Spoofing** group, with the comfort tweaks (DND / auto-brightness / keep-screen-on / no-log) in a **Tweaks** group.
*   **New "This Device" profile.** A built-in profile that keeps your **real** device identity but still lets you add CPU / GPU / COW / Android-ID spoofs or block CPU per app — handy for apps you don't want to disguise but do want to tweak.
*   **Automatic migration.** Old configs with the global CPU block-list / CPU-only list are converted the moment they load, so existing setups keep working — just reorganized into the new model.
*   **Block CPU anywhere.** Any package on any profile (including This Device) can be set to force the **real** processor. Still mutually exclusive with CPU Spoofing.

### CPU spoof — survives resume and backgrounding
*   **Optional foreground reconcile.** A new opt-in keeps the correct `cpuinfo` mounted as you switch apps, fixing the case where resuming or backgrounding a game could quietly revert it to the real CPU. The Zygisk helper still mounts at launch; this covers everything after.

### Smaller stuff
*   **Dropped 32-bit x86.** The build no longer ships the rarely-used 32-bit x86 library (arm64, arm, and x86_64 are unchanged).
*   **CPU profiles flag missing files.** A CPU profile whose `cpuinfo` file isn't actually present is now clearly marked in the picker instead of silently doing nothing.
*   **Custom profiles are deletable**, and CPU/GPU profiles sync from GitHub by manifest.

### WebUI & polish
*   **Launch a spoofed app from the library.** Installed packages now show a **play** button that force-stops and relaunches the app, so it restarts through a fresh spoof — no need to leave the WebUI and find it yourself.
*   **AMOLED on System theme.** With the theme set to **System**, dark mode now uses **AMOLED black** instead of plain dark (the explicit Dark option is unchanged).
*   **Custom CPU/GPU forms tidied.** The "add profile" fields show their titles as proper labels above each box, matching the device and package editors, and the CPU/GPU add & edit sheets now lift above the on-screen keyboard so nothing hides while you type.
*   **GPU row + fuller Kernel row.** The System tab now shows your real **GPU**, and the **Kernel** row includes the architecture (e.g. `5.10.404R aarch64`) instead of just the release.

---
## v5.4.0
### Per-app CPU spoofing — pick any chip, or load your own
*   **Choose a CPU model per app.** With CPU Spoofing now opens a **CPU profile picker** so each game can pretend to run on a *specific* chip. Ships with **8 profiles** out of the box — Qualcomm Snapdragon 8 Elite, HiSilicon Kirin 9000 / 9000S / 9020 / 9020A / 9030 Pro, and Xiaomi Xring O1 / O3.
*   **Load your own `cpuinfo`.** Tap the **＋** next to the picker's search, choose any `cpuinfo` file from storage, give it a name, and it's saved into the module — **reusable across every app**, not just the one you're editing.
*   **CPU spoof for CPU-Only packages too.** Packages in the CPU-Only list now get the same model picker.
*   **Cleaner tags.** A spoofed app now carries a single `cpu=` tag (with the model baked in) instead of a redundant pair — existing setups keep working and tidy themselves up when you re-save them.

### Mobile Legends (and other multi-process games) fixed
*   **Child processes are now spoofed.** Games that run extra processes (e.g. Mobile Legends' `:UnityKillsMe`) used to leak the **real** device in those processes because COPG only matched the exact process name. It now matches the **base package**, so every process of the game sees the spoof.
*   **Added Mobile Legends variants** (Global, US, VNG, and the Moonton / Samsung / Huawei builds) to the device list.

### Per-app ANDROID_ID
*   **Opt an app into a unique ANDROID_ID.** A new **Spoof Android ID** toggle in the package editor derives a per-app ID from the device profile's seed — so each app gets a *different* ID (matching how real Android 8+ behaves), and the ⓘ previews the exact value the app will read. Still zero-residency: the module unloads before the app runs.

### Stability
*   **Fixed a crash with COW prop spoof on some devices.** On certain phones (e.g. OnePlus / Android 16) a spoofed property was stored in a "long" form that the COW edit corrupted, crashing apps at launch. COPG now detects those and skips them safely — the device is still spoofed via the Build.* fields; nothing crashes.

### WebUI & polish
*   Device editor: the **Serial** field moved into the **Advanced build props** group.
*   Smoother in-app config (background-safe app-icon downloads, tightened security policy).
*   **All 8 languages** brought to full parity.

---
## v5.3.1
### Hotfix — CPU spoof rolled back (was causing bans)
*   **Reverted the new CPU spoof method.** v5.3.0 mounted the fake `cpuinfo` inside each game's own process — but that mount is **visible to the app itself** (`/proc/self/mountinfo`), which some games flagged as tampering and **banned**. v5.3.1 restores the **previous CPU spoof** (system-level mount via the helper) that does not expose the mount to the app.
*   **CPU blocking is opt-in again (like v4.7.3) — no more 50% failures.** The intermediate 5.1.x builds had every non-spoofed app in a profile force a global CPU "unblock", which raced the spoof mounts so it only landed about half the time. Now only **With CPU Spoofing** mounts and only an explicit block unmounts; everything else leaves the mount alone — so CPU spoof applies reliably again.
*   **New per-app "Block CPU" toggle.** A package inside a device profile can now be set to **Block CPU** (forces it to see the real processor) right from the package editor — handy for keeping a banking app on real CPU while games in the same profile are spoofed. It's the opposite of With CPU Spoofing, so only one can be on.
*   **Everything else from v5.3.0 stays:** COW prop spoof, ANDROID_ID spoofing, the extra Build fields, and the tablet navigation-bar fix are all unaffected (none of them mount anything).

> If you installed v5.3.0, **update to v5.3.1** and clear data / force-stop any affected game.

---
## v5.3.0
### New Stealth Spoofing Engine
*   **COW prop spoof replaces GOT hooking.** The old method left a chunk of COPG's code mapped in the game's memory (detectable). The new **`cow` tag** edits device props (`ro.product.*`, fingerprint, and any custom prop) through a per-process copy-on-write page and then **unloads the module before the app runs** — so an anti-cheat scanning memory finds **nothing of COPG mapped**. Same effect, far stealthier. Only your app sees the fake; the system and other apps are untouched.
*   **ANDROID_ID spoofing (no reboot).** Set a per-device `ANDROID_ID` and apps read the fake value with **no reboot and no detectable hook** — the value is planted in the in-process settings cache. New field in the device editor with a **Generate** button.
*   **Serial spoofing.** Optional `Build.SERIAL` + `ro.serialno` per device, also with a Generate button.

### More Build Fields
*   **12 extra Build identifiers** can now be faked per device — `BOARD, HARDWARE, DISPLAY, ID, BOOTLOADER, TAGS, TYPE, SOC_MANUFACTURER, SOC_MODEL, SECURITY_PATCH, INCREMENTAL, CODENAME` — set both as `Build.*` fields **and** their matching `ro.*` props. They live in a collapsible **Advanced build props** group in the device editor, all with example placeholders.

### CPU Spoof — Reliable Every Launch
*   **Fixed the ~50% CPU-spoof failure.** The fake `cpuinfo` is now mounted **inside each game's own process**, in a private mount namespace, instead of through a shared root helper that raced against the per-app block logic. It now lands **every launch**, deterministically. (Per-app only — system and other apps still see the real CPU.)
*   **Removed the Zygisk companion entirely.** CPU spoofing is pure in-process C++ now — one less moving part, nothing extra mapped in the game's memory.

### WebUI & Cleanup
*   **Removed the "Blocked" package type / block list.** Redundant — leaving **With CPU Spoofing** off already blocks the CPU fake (the default). The Type picker is now just **Device** and **CPU Only**.
*   **Keyboard-aware editor sheets** — forms lift above the on-screen keyboard instead of hiding behind it.
*   **Example placeholders** on every device/package field (a coherent Pixel 8 Pro set) so the expected format is obvious.
*   Editing a device now **merges** over the saved profile instead of rebuilding it, preserving advanced keys.
*   Fixed the bottom navigation / ripple rendering off-center on tablets and wide screens.

---
## v5.1.1
### Fully Systemless — no /system mount, no metamodule
*   **No metamodule needed.** COPG no longer touches `/system` at all, so on newer KernelSU you **no longer need the system-overlay metamodule** — it installs and runs as a plain module on KernelSU / Magisk / APatch.
*   **Removed the Google Photos "unlimited backup" spoof.** It worked by overlaying Pixel feature-flag files into `/system`, which forced a system mount (and required that metamodule on newer KernelSU). Dropping it makes COPG **fully systemless** — lighter, cleaner, and free of mount conflicts.
*   **Removed `system.prop`.** The `game_default_frame_rate.disabled` prop meant to lift Android 15's 60fps game cap backfired on some ROMs (read inverted → games **locked** to 60fps). Removed — affected devices now run at full refresh.

### Fixes
*   **Console no longer freezes the UI.** A never-ending command typed in the console (bare `logcat`, `top`, `tail -f`) used to hang the whole WebUI, because the bridge waits for the process to exit. Streaming commands are now guarded: `logcat` auto-runs as `logcat -d`, and other infinite commands are refused with a hint.
*   **Zygisk detection on install.** Fixed two false negatives — Zygisk Next *disabled* + ReZygisk *enabled* no longer aborts install, and Magisk's built-in Zygisk being **ON** is now caught (it silently breaks Zygisk Next / ReZygisk).

---
## v5.1.0
### Faster, Smarter App Icons
*   **No more lag.** Package icons no longer hammer the root bridge one-by-one — a single background task fetches everything, so the Library stays smooth while icons fill in.
*   **Three sources:** Google Play → **APKPure** → F-Droid, so games missing from the Play Store (region-locked and the like) still get a real icon.
*   **Works offline.** Once fetched, icons are cached on-device and load instantly with no network.
*   **Refresh App Icons:** a new action in **Backup & Sync** wipes the icon cache and re-downloads everything — handy when a game changes its icon.

### WebUI Polish
*   **Fullscreen toggle:** an immersive mode that hides the status bar, in Settings.
*   **Device Profile picker:** choosing a device for a package is now a searchable, sortable list instead of a plain dropdown.
*   Fixed the black band under the status bar / notch in fullscreen, plus navigation-bar and sort/filter spacing touch-ups.

### Game List
*   Refreshed the bundled device and game profiles (`COPG.json` / `list.json`).

## v5.0.0
### A Complete Reimagining
*   **Brand-New WebUI:** The entire interface was rebuilt from the ground up with a modern, app-like design — replacing the old single-file UI with a fast, modular one.
*   **Library:** Devices and packages live in one place now, with search, sorting, filters, swipe-between-tabs, and live **Installed** badges.
*   **App Picker:** Add games straight from your installed apps, complete with real names and icons.
*   **Backup & Sync:** Export and restore your config, **Sync from GitHub** to pull the latest game list, and export logs — all saved to `Download/COPG`.
*   **Built-In Console:** Run shell commands, tail logcat, and save logs without ever leaving the app.
*   **Theming & Language:** Dark / Light / AMOLED / Auto themes with one-tap switching and haptic feedback. Now ships in **8 languages** — English, Persian (RTL), Arabic (RTL), German, Spanish, Indonesian, Thai and Chinese.
*   **Fresh app icon** and a cleaner WebUI title.

### Simpler, Safer Spoofing
*   **CPU spoof is now blocked by default.** Every device package is safe out of the box — flip on **With CPU Spoofing** only for the games that need the chipset faked.
*   **Retired the `blocked` and `notweak` tags.** They are no longer needed; a single clear toggle replaces them.
*   **New — GOT Hooking:** an advanced in-memory spoof method for stubborn apps that ignore the normal spoof. Off by default and gated behind a clear risk warning — it can be detected and may even get a game account banned, so only enable it if you know you need it.
*   Plain-language ℹ️ explanations sit next to every toggle, so you always know what each one does.
*   Added support of x84_64 and x86 devices/emulators.

### Per-App Game Tweaks (New)
*   Choose comfort tweaks **per game** — applied only while you are playing and automatically restored when you leave:
    *   **Do Not Disturb**, **Disable Auto-Brightness**, **Keep Screen On**, **Disable Logging**.
*   Smarter switching: hopping between games applies only the tweaks each one asks for — no more notification floods at the wrong moment.
*   Works for both device-spoof and CPU-spoof packages.

### Improvements
*   Rewrote the tweak controller to apply tweaks **per package** with precise apply/restore, dropping the old global toggle file.
*   Allowlist-based tag parsing — unknown or future tags are safely ignored everywhere in the module.
*   Reorganized the whole project (`src/`, `module/`) with a single clean CI workflow and an on-device build script.

### New Devices & Game Profiles
*   **New device:** **Infinix GT 50 Pro** added to the spoof library.
*   **Device refresh:** **RedMagic 10 Pro → RedMagic 11 Pro**, and **HONOR Magic V2 RSR → Sony Xperia 1 III** (old profiles removed, replaced with newer hardware).
*   **PUBG Mobile:** unlocked **HDR + 90+ FPS**.
*   **Free Fire / Free Fire MAX:** unlocked **144 FPS**.
*   **Asphalt 9:** unlocked **120 FPS**.

### Note
*   This is a massive overhaul and still a work in progress — more is on the way. Please report anything unexpected in the support group.
*   **Translations:** going forward only **English** and **Persian** are officially maintained. The other languages are community-driven — contributions and fixes via pull request are very welcome.

### Summary
v5.0.0 is the biggest release yet: a ground-up WebUI rewrite, a simpler and safer spoofing model (block-by-default CPU), brand-new per-app game tweaks, and a cleaner, faster foundation under the hood.
## v4.7.3
- fix version code
## v4.7.2
- Fixed camera/setting some apps were crashing (removed resetprop feature)
- Removed camera apps from black list (COPG.JSON)
- Dropped FIFA mobile support.
## v4.7.1
### HOTFIX Update
- Added blocked tag to all device packages for safety
- Fixed Update Gameslist and action button
### Note: 
- I recommend you add your game always as blocked package expect games those aren't working with device spoof only or they only work with CPU spoof

## v4.7.0
### New Features
*   **Safer Spoof Method:** Enhanced safety and reliability of the spoofing process.
*   **Android/SDK Spoof:** Added optional Android SDK version spoofing capability.
*   **Smart CPU Spoof (Snapdragon 8 Elite):** Introduces intelligent, app-targeted CPU information spoofing via `/proc/cpuinfo`.
*   **Enhanced Configuration (`COPG.json`):**
    *   Configuration file renamed from `config.json` to `COPG.json` (original `config.json` remains for MMRL app settings).
    *   **`blocked` tag:** For apps/games where you want device model spoofing but need to hide/remove CPU spoof.
    *   **`with_cpu` tag:** For apps/games where you want both device model spoofing and CPU spoofing applied together.
    *   **`cpu_only_packages`:** For apps/games where you want **only** CPU spoofing applied (also resets device properties to original via `resetprop`).
    *   **`blocklist`:** To hide/remove CPU spoof and reset properties for sensitive apps (e.g., banking apps, camera apps in Matrix ROM, system settings in some ROMs).
    *   **`notweak` tag:** Significantly improves I/O performance and reduces overhead for specified packages.
*   **Smarter Tweak Controller:** Improved configuration handling logic.
*   **Enhanced WebUI:**
    *   Added sorting and filtering capabilities.
    *   Source code cleanup and full support for all new configuration changes.
*   **Updated Game Support:**
    *   Added support for **Fortnite** and **Farlight 84**.
    *   Removed support for **Mobile Legends: Bang Bang**.

### Improvements
*   **Permission & SELinux:** Better handling during `COPG.json` restoration.
*   **Logging:**
    *   Simplified Zygisk logs.
    *   Changed log tag from `SpoofModule` to `COPGModule`.
*   **Configuration Management:** Removed deprecated `ignorelist.txt` and associated functions from the controller and WebUI.

### Summary
This major release focuses on intelligent, app-specific spoofing with the new Smart CPU feature, a more powerful and granular configuration system via `COPG.json`, a refined WebUI, and significant under-the-hood optimizations for performance and stability.
## v4.6.0
- Added new spoof method with resetprop to zygisk
- Rewrote COPG toggle manager from shell script to C++
- Rewrote config watcher from C to C++ and integrated into COPG toggle manager
- Improved performance for COPG toggle manager
- Fixed toggle persistence issue with smarter toggle management (now DND and screen time turns back to default when you close the game)
- Reduced CPU, RAM, and storage overhead for COPG toggle manager
- Cleaned up installation output logs
- Fixed FIFA max FPS unlock
- Added Galaxy Z Fold 5 support for FIFA
- Removed jq binary dependency
- Removed legacy copg_watcher binary
- Added donation pop-up to WebUI
- Added support pop-up to WebUI
- Changed license from OIL to AGPL-3.0 (now 100% free software)
## v4.5.5
- Fixed config parsing errors when you open a target app/game (e.g. Invalid package list for key PACKAGES_...)
- Fixed font color of save log popup in dark mode 
## v4.5.4
- Fixed config.json parsing for apps/games containing underscore ("_") characters in their package names (Zygisk)
- Optimized CPU usage for the config_watcher binary
- Reduced module file size and binary sizes
- Added Realme 15 Pro 5G device support and migrated Arena of Valor packages from Realme P3 5G
- Compiled all binaries with NDK 27
## v4.5.3 
- Fix config.json parsing issue which causes DND/Timeout and other toggles not working correctly 
## v4.5.2
- Removed the `/proc/cpuinfo` spoofing method due to its reliance on a mount technique that was flagged as suspicious by certain banking applications.
- Removed Fortnite 120 FPS unlock due to its dependency on `/proc/cpuinfo` spoofing.
## v4.5.1
- Spoofed the /proc/cpuinfo file to report the device as a Snapdragon 8 Elite.
   - This workaround tricks games like Fortnite, which rely on a strict CPU model check, into enabling hidden graphics settings (e.g., 120 fps mode).
- Added Fortnite 120 fps
- Updated icon for WebUI-X and mmrl repo Special Thanks to [AmbadeuZ](https://t.me/AmbadeuZ) for the new Icon
- Updated banner for KernelSU-Next and mmrl Thanks to [AmbadeuZ](https://t.me/AmbadeuZ) for the new Banner
## v4.5.0
- Added better spoof method to hide from anticheat (no freefire or delta force ban anymore)
- Added [atexit](https://github.com/5ec1cff/local_cxa_atexit_finalize_impl) by [5ec1cff](https://github.com/5ec1cff) to prevent detection from banking apps/ Anti cheats
- Added better config loading with less overhead and better performance
- Added Backup/Restore feature to WebUI
- Added save logcat file feature to the WebUI
- Added Better logcat output (now only shows errors)
- Added freeFire and freefire max 120 fps
- Added Clash of Titans (India)
- Added Wuthering Waves
- Added Arena of Valor/Honor of Kings all regions
- Removed Fortnite (because spoof doesn't work on it anymore) i will try to add it later with 120 fps support 
- Removed Apex mobile because this game doesn't exist anymore 
## v4.3.6
- Removed freefire
- Added volume control for action script
- Added optional volume control for installing Google photos spoof
- Removed spoofSystemProperties function
## v4.3.5
- Added Next-WebUI package manager
- Removed shortcut button from WebUI (use WebUI-X app to create shortcut)
- added logcat to WebUI (Now you don't need terminal to send me logs)
- added copy button to WebUI console/logs/output
## v4.3.4
- Added FreeFire 120 fps
- Added delta force into game repo
- Added Netflix spoof 
- Cleanup and improving performance and safety
- added KernelSU Next banner
- UI improvement
## v4.3.3
- Fixed root detection by apps because of COPG
- Fixed Mx player, Telegraph and some apps crashing because COPG
- Add version info in installation script 
## v4.3.2
- secure spoofing
- fixed Rezygisk installation 
## v4.3.1
- Added Google photos spoof for unlimited backup and Ai reimagine feature
- Added modern app picker (Use MMRL or WebUI-X)
- Added Game names
- Added TikTok to unlock 1080 streaming
- Added Ignorelist to prevent Apps/Games from general tweaks (e.g. DND, Timeout and ...)
- Fixed some permission peroblems
- Fixed some styles
- Removed useless props (CPU info, Build id, display id)
- Dropped support Zygisk Standard/Native ( Unsafe)
- Fixed crashing on Older devices
- Added swipe snackbar to get rid of it
- Added improvement to snackbar
- Added installed label for installed items
- Added ignore label with explaining popup
- fixed visual bugs and improved packagename error
## v4.0.2
- Improved error logging for SpoofModule and WebUI
- Enhanced permission handling for config files
- Added smoother UI animations and transitions
- Improved device selection UI in the game tab
- Added error popup
- Added prevention for game duplication
- Fixed scroll bar overlapping issues
- Enhanced UI with field highlighting
- Added Delta Force support in config
- Removed reboot requiring with dynamic config check
- Fixed APatch installation issues
- Removed reboot popup
- added snackbar with cooldown animation
- added shortcut button/Support for WebUI-X app
## v3.3.4
- Added apatch support
- Added better architecture detection
- Added Zygisk Next disable detection
- Added better ouput log for installation
- Added multiple root detection
- Added better module deleting 
## v3.3.3
- Updated config and fixed Black Shark device model  
- Changed UI color details and improved dark mode  
- Improved UI/UX and smoothness  
- Enhanced animations  
- Smoother switching between tabs  
- Added logging to the module for better debugging  
## v3.3.2
- fix installation for magisk 
## v3.3.1
- Better installation script 
- removing unused files after installing 
## v3.3
- improved UI
- Added more animations
- added better logging UI
- Added Android version input
- Minor fix when user was adding a new device 
## v3.2.2
- update config
## v3.2.1
### Added
- **New UI Sections**:  
  - Dedicated tabs for *Devices* and *Games* management.  
  - Search bar for quick filtering.  
- **Device/Game Controls**:  
  - Add, edit, or remove devices/games directly from the UI.  
  - Swap devices per-game dynamically.  
- **Quality of Life**:  
  - `KEEP_SCREEN_ON` toggle (timeout: ~83 hours) with auto-reset on exit.  
  - Screen Timeout Toggle to prevent interruptions.  
  - Swipe gestures to navigate between tabs.  
- **Animations**: Smoother MIUI/HyperOS-style transitions.  

### Changed  
- **UI/UX**:  
  - Updated theme and improved visual consistency.  
  - Enhanced smoothness across interactions.  
- **Game Detection**:  
  - Replaced `grep` with `jq` for reliable package parsing (ignores `_DEVICE` keys).  
- **Multi-Game Handling**:  
  - Toggles persist while any game is active; reset only after all close.  

### Fixed  
- **Root Compatibility**: Fixed Magisk integration with Zygisk Next installation.  
- **Stability**: Better console error handling.  
- **Edge Cases**: Split-screen/switching between games no longer breaks settings.  

---  
*For older versions, refer to previous releases.*  

## v3.1.5
### What’s New?
- **UI Upgrades**  
  - Sleeker output log for clearer feedback.  
  - Dark/light theme toggle—your eyes will thank you!  
- **Remove ROM Limits**  
  - New prop kills FPS caps in games like PUBG Mobile.  
- **Saves Your Settings**  
  - Stores DND and brightness settings when you start gaming, restores them after.  
- **Better Handling**  
  - DND and brightness stay consistent during gameplay, reset only when you exit.
## v3.1.0
### What’s New?
- **Enhanced User Interface**: Revamped UI with smoother transitions and a modern vibe for a delightful experience.
- **Clear Console Button**: Added a handy "Clear" button to reset the log output with one tap—no more cluttered console!
- **Auto-Scrolling Logs**: New logs now auto-scroll like a Linux terminal, keeping you on the latest updates without manual effort.
- **Improved Log Animation**: Slicker animation when expanding the log output for a seamless reveal.
- **Smarter DND Backend**: Adjusted "Do Not Disturb" to priority mode when on, toning down its aggressive silence for a balanced feel.
### Removed
- **Reboot Button**: Ditched the reboot option to streamline the interface and keep it simple.
### Under the Hood
- Optimized animations for smoother performance across devices.
- Polished responsiveness for a snappier, more fluid feel.
## v3.0.3
- added webui 
- webui: added disable DND toggle 
- webui: added disable auto brightness  toggle
- webui: added stop logger toggle 
- webui: added update game list button
- webui/action: added game list compare 
- webui: added reboot pop-up question if game list updated
- webui: added reboot button with pop-up question
## v2.8
- trying to fix mid-game crashing
- improve ban spoof system
- **Note**: getting banned at your own risk
## v2.6
- better anti-cheat handling
- **Note**: getting banned at your own risk
## v2.5
- added better spoof for Call of duty anti-cheat
- **Note**: getting banned at your own risk
## v2.3
- added action button for downloading updated config if needed
- added Zygisk check for installing module
- **Note**: getting banned at your own risk 
## v2.2
- Cache JNI Calls for faster app game launch.
- removed logd for even faster launch.
## v2.1.1
- fix s24 ultra in config.json
## v2.1
- now you can add your game to config yourself and you don't need to ask for new update
- config direction: /data/adb/modules/COPG/config.json
## v2
- Added Mobile Legends `com.mobilelegends.mi` support - spoof as ZTE NX769J
- Added Brawl Stars `com.supercell.brawlstars` support - spoof as ZTE NX769J
- Added Diablo Immortal `com.blizzard.diablo.immortal` support - spoof as ZTE NX769J
- Added Arena Breakout `com.netease.newspike` support - spoof as ZTE NX769J
- Added Call of Duty: Warzone Mobile `com.activision.callofduty.warzone` support - spoof as ZTE NX769J
- Added PUBG: New State `com.pubg.newstate` support - spoof as ZTE NX769J
- Added Destiny Warfare `com.gamedevltd.destinywarfare` support - spoof as ZTE NX769J
- Added Drive or Die 2 `com.pikpok.dr2.play` support - spoof as ZTE NX769J
- Added CarX Highway Racing `com.CarXTech.highWay` support - spoof as ZTE NX769J
- Added Shadow Fight 3 `com.nekki.shadowfight3` support - spoof as ZTE NX769J
- Added Shadow Fight Arena `com.nekki.shadowfightarena` support - spoof as ZTE NX769J
- Added Asphalt 8 `com.gameloft.android.ANMP.GloftA8HM` support - spoof as ZTE NX769J
- Added Shadow Fight 2 `com.nekki.shadowfight` support - spoof as ZTE NX769J
- Added Need for Speed No Limits `com.ea.game.nfs14_row` support - spoof as ZTE NX769J
- Added Real Racing 3 `com.ea.games.r3_row` support - spoof as ZTE NX769J
- Added Squad Busters `com.supercell.squad` support - spoof as ZTE NX769J
- Added Battle Prime `com.blitzteam.battleprime` support - spoof as ZTE NX769J

- Added Honor of Kings `com.proximabeta.mf.uamo` support - spoof as Black Shark 4

- Added Apex Legends Mobile `com.ea.gp.apexlegendsmobilefps` support - spoof as Xiaomi Mi 11T PRO
- Added Tower of Fantasy `com.levelinfinite.hotta.gp` support - spoof as Xiaomi Mi 11T PRO
- Added Clash of Clans `com.supercell.clashofclans` support - spoof as Xiaomi Mi 11T PRO
- Added Mobile Legends (Vietnam) `com.vng.mlbbvn` support - spoof as Xiaomi Mi 11T PRO

- Added Arena of Valor (Global) `com.levelinfinite.sgameGlobal` support - spoof as Xiaomi Mi 13 Pro
- Added Arena of Valor (China) `com.tencent.tmgp.sgame` support - spoof as Xiaomi Mi 13 Pro
- Added PUBG Mobile (Korea) `com.pubg.krmobile` support - spoof as Xiaomi Mi 13 Pro
- Added PUBG Mobile (China) `com.rekoo.pubgm` support - spoof as Xiaomi Mi 13 Pro
- Added PUBG Mobile (Taiwan) `com.tencent.tmgp.pubgmhd` support - spoof as Xiaomi Mi 13 Pro
- Added PUBG Mobile (Vietnam) `com.vng.pubgmobile` support - spoof as Xiaomi Mi 13 Pro

- Added Blood Strike `com.netease.lztgglobal` support - spoof as OnePlus 8 PRO
- Added League of Legends: Wild Rift `com.riotgames.league.wildrift` support - spoof as OnePlus 8 PRO
- Added League of Legends: Wild Rift (Taiwan) `com.riotgames.league.wildrifttw` support - spoof as OnePlus 8 PRO
- Added League of Legends: Wild Rift (Vietnam) `com.riotgames.league.wildriftvn` support - spoof as OnePlus 8 PRO

- Added Fortnite `com.epicgames.fortnite` support - spoof as OnePlus 9 PRO
- Added Epic Games Portal `com.epicgames.portal` support - spoof as OnePlus 9 PRO
- Added League of Legends: Wild Rift (China) `com.tencent.lolm` support - spoof as OnePlus 9 PRO

- Added Mobile Legends: Bang Bang `com.mobile.legends` support - spoof as POCO F5

- Added Free Fire `com.dts.freefireth` support - spoof as Asus ROG Phone
- Added Free Fire MAX `com.dts.freefirethmax` support - spoof as Asus ROG Phone

- Added FIFA Mobile `com.ea.gp.fifamobile` support - spoof as Asus ROG Phone 6
- Added Asphalt 9 `com.gameloft.android.ANMP.GloftA9HM` support - spoof as Asus ROG Phone 6
- Added Shadowgun Legends `com.madfingergames.legends` support - spoof as Asus ROG Phone 6
- Added Black Desert Mobile `com.pearlabyss.blackdesertm` support - spoof as Asus ROG Phone 6
- Added Black Desert Mobile (Global) `com.pearlabyss.blackdesertm.gl` support - spoof as Asus ROG Phone 6

- Added Call of Duty: Mobile (Garena) `com.garena.game.codm` support - spoof as Lenovo TB-9707F
- Added Call of Duty: Mobile (Korea) `com.tencent.tmgp.kr.codm` support - spoof as Lenovo TB-9707F
- Added Call of Duty: Mobile (Vietnam) `com.vng.codmvn` support - spoof as Lenovo TB-9707F
---
## v1.0.5
- Added Fortnite support (spoof to s24 ultra)
- added mobile legends support (spoof to s24 ultra)

