# COPG (COD+PUBG=COPG)

COPG is a Zygisk module designed to spoof device properties for specific Android games, such as Call of Duty Mobile and PUBG Mobile. It modifies the `android.os.Build` fields in the game's runtime to bypass device checks, allowing you to play on unsupported devices or emulate different hardware.

## Features
- Spoofs device properties (e.g., model, brand) for targeted apps.
- Currently supports:
  - Call of Duty Mobile (`com.activision.callofduty.shooter`) - Spoofs as Lenovo TB-9707F.
  - PUBG Mobile (`com.tencent.ig, com.pubg.imobile` ) - Spoofs as Xiaomi 2210132G.
  - Mobile legends spoof as Samsung Galaxy s24 ultra.
  - Fortnite spoof as Samsung Galaxy s24 ultra.
- Lightweight and built with Zygisk for seamless injection.
- Compatible with Magisk (Zygisk enabled) or KernelSU with Zygisk Next.

## Requirements
- Rooted Android device with:
  - Magisk v24+ with Zygisk enabled, or
  - KernelSU with Zygisk Next installed (download from [ZygiskNext releases](https://github.com/Dr-TSNG/ZygiskNext)).
- Android 9.0 (API 28) or higher (tested up to Android 14).

## Installation
1. **Download the Module**:
   - Grab the latest Update from the [Releases](https://github.com/AlirezaParsi/COPG/releases) page or build it yourself (see below).

2. **Install via Magisk/KSU**:
   - Open the Magisk app or KernelSU manager.
   - Go to "Modules" > "Install from storage".
   - Select `COPG.zip` and flash it.
   - Reboot your device.

3. **Verify**:
   - Check the Magisk/KSU Modules tab to ensure "COPG Zygisk Module" is enabled.
   - Go to Zygisk Next settings and check if COPG module is enabled.

## Usage
- Launch COD Mobile or PUBG Mobile.
- The module automatically spoofs device properties for these apps:
  - COD Mobile: Appears as Lenovo TB-9707F.
  - PUBG Mobile: Appears as Xiaomi 2210132G.
- No manual configuration needed!

## Building from Source
1. **Prerequisites**:
   - Android NDK r27 (27.0.12077973).
   - CMake 3.21+.
   - GitHub Actions (workflow included).

2. **Steps**:
   - Clone this repo: `git clone https://github.com/yourusername/COPG.git`
   - Push changes to trigger the GitHub Actions workflow (`.github/workflows/build.yml`).
   - Download `COPG.zip` from the workflow artifacts unzip(Because artifact output is zip inside zip) and install`COPG.zip` with Magisk/KSU.

## Troubleshooting
- **Not Detected**: Ensure Zygisk Next is installed and active (Magisk Settings > Zygisk > On).
- **No Spoofing**: Check logs with `adb logcat | grep SpoofModule` for errors.
- **Game Crashes**: Verify your device’s Android version compatibility.

## License
This project is licensed under the [All Rights Reserved License](#license). Unauthorized copying, modification, or distribution is prohibited without explicit permission from the author.

## Credits
- Built with inspiration from [Zygisk module sample](https://github.com/topjohnwu/zygisk-module-sample).

