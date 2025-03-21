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
  - Mobile Legends (`com.mobile.legends`): Spoofs as POCO F5 (23049PCD8G).
- **How It Works**:
  - On boot, it loads `config.json` into a fast memory list (~150ms one-time).
  - When you open a listed app, it spoofs in ~1ms—no noticeable delay.
- **Adding Apps**: Edit `config.json` (see below), push it to `/data/adb/modules/COPG/`, and reboot.

### Customizing `config.json`
- Format (example from full config):
  ```json    "PACKAGES_BS4": [
        "com.proximabeta.mf.uamo"
    ],
    "PACKAGES_BS4_DEVICE": {
        "BRAND": "Black Shark",
        "DEVICE": "Black Shark 4",
        "MANUFACTURER": "Xiaomi",
        "MODEL": "2SM-X706B",
        "ro.product.model": "PRS-H0",
        "FINGERPRINT": "BlackShark/PRS-H0/Black Shark 4:13/TQ3A.230805.001/20230315:user/release-keys",
        "BUILD_ID": "TQ3A.230805.001",
        "DISPLAY": "TQ3A.230805.001.A1",
        "PRODUCT": "PRS-H0",
        "VERSION_RELEASE": "13",
        "SERIAL": "BS412345678",
        "CPUINFO": "processor\t: 0\nmodel name\t: ARMv8-A (Black Shark 4)\nHardware\t: Qualcomm Snapdragon 870\nSerial\t: BS412345678",
        "SERIAL_CONTENT": "BS412345678"
    }
