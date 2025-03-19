# COPG

COPG is a Zygisk module that spoofs device properties for specific Android games, like Call of Duty Mobile and PUBG Mobile. It tweaks the `android.os.Build` fields (e.g., model, brand) at runtime to trick games into thinking your phone is a different device—perfect for bypassing device checks or playing on unsupported hardware.

## Features
- Spoofs device info (model, brand, etc.) for targeted apps listed in `config.json`.
- Super fast: Adds only ~1ms to app launch time with optimized lookups and cached JNI calls.
- Flexible: Update the spoof list by editing `config.json`—no rebuild needed.
- Lightweight: Built with Zygisk for smooth injection into app processes.
- Works with Magisk (Zygisk enabled) or KernelSU with Zygisk Next.

## Requirements
- Rooted Android device with:
  - Magisk v24+ (Zygisk enabled), or
  - KernelSU with Zygisk Next (get it from [ZygiskNext releases](https://github.com/Dr-TSNG/ZygiskNext)).
- Android 9.0 (API 28) or higher (tested up to Android 14).

## Installation
1. **Download the Module**:
   - Get the latest `COPG.zip` from the [Releases](https://github.com/AlirezaParsi/COPG/releases) page or build it yourself (see below).

2. **Install via Magisk/KSU**:
   - Open Magisk or KernelSU manager.
   - Go to "Modules" > "Install from storage".
   - Pick `COPG.zip` and flash it.
   - Reboot your device.

3. **Verify**:
   - In Magisk/KSU, check the Modules tab—look for "✨ COPG spoof ✨" as enabled.
   - In Zygisk settings, confirm COPG is active.

## Usage
- The module uses `/data/adb/modules/COPG/config.json` to decide which apps to spoof and what device info to use.
- **Default Examples**:
  - COD Mobile (`com.activision.callofduty.shooter`): Spoofs as Lenovo TB-9707F.
  - PUBG Mobile (`com.tencent.ig`): Spoofs as Xiaomi Mi 13 Pro (2210132G).
- **How It Works**:
  - On boot, it loads `config.json` into a fast memory list (~150ms one-time).
  - When you open a listed app, it spoofs in ~1ms—no noticeable delay.
- **Adding Apps**: Edit `config.json` (see below), push it to `/data/adb/modules/COPG/`, and reboot.

### Customizing `config.json`
- Format:
  ```json
  {
    "PACKAGES_REDMAGIC9": ["com.mobile.legends", "com.supercell.brawlstars"],
    "PACKAGES_REDMAGIC9_DEVICE": {"BRAND": "ZTE", "DEVICE": "NX769J", "MANUFACTURER": "ZTE", "MODEL": "NX769J"},
    "PACKAGES_COD": ["com.activision.callofduty.shooter"],
    "PACKAGES_COD_DEVICE": {"BRAND": "Lenovo", "DEVICE": "TB-9707F", "MANUFACTURER": "Lenovo", "MODEL": "Lenovo TB-9707F"}
  }
