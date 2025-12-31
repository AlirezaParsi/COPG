# COPG Spoof Module
#### **Support Group**:
- https://t.me/TheAOSP
#### **Telegram Channel**:
- https://t.me/COPG_module
---
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

