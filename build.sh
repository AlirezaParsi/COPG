#!/data/data/com.termux/files/usr/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
# COPG — Termux build + package script
#
# Builds the Zygisk companion (.so) and the unified_controller for THIS device's
# ABI, assembles a flashable Magisk/KSU/APatch module ZIP, and drops it in
#   /storage/emulated/0/Download/COPG
#
# Termux only. Installs any missing prerequisites (clang, zip, llvm) itself.
# Multi-ABI release builds are done by the json.yml CI; this script builds the
# single native ABI for fast on-device build-&-test.
#
#   Usage:  ./build.sh [version] [versionCode]
#   e.g.    ./build.sh 4.3.5 435
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

REPO="$(cd "$(dirname "$0")" && pwd)"
cd "$REPO"

VERSION="${1:-5.0.0}"
VERSION_CODE="${2:-500}"
OUT_DIR="/storage/emulated/0/Download/COPG"
BUILD_DIR="$REPO/.build"
STAGE="$BUILD_DIR/module"

c_info=$'\033[1;36m'; c_ok=$'\033[1;32m'; c_err=$'\033[1;31m'; c_off=$'\033[0m'
log(){ printf '%s▶ %s%s\n' "$c_info" "$*" "$c_off"; }
ok(){  printf '%s✓ %s%s\n' "$c_ok" "$*" "$c_off"; }
die(){ printf '%s✗ %s%s\n' "$c_err" "$*" "$c_off" >&2; exit 1; }

# ── prerequisites ────────────────────────────────────────────────────────────
need(){ # <command> <termux-package>
  command -v "$1" >/dev/null 2>&1 && return 0
  log "installing $2 …"
  pkg install -y "$2" >/dev/null 2>&1 || die "could not install $2 (run: pkg update)"
}
log "checking prerequisites"
need clang++  clang
need zip      zip
need llvm-strip llvm
UPX=""; command -v upx >/dev/null 2>&1 && UPX=1   # optional, only shrinks the binary

ls -d /storage/emulated/0 >/dev/null 2>&1 || die "no storage access — run: termux-setup-storage"

# ── detect device ABI ────────────────────────────────────────────────────────
case "$(uname -m)" in
  aarch64)       ABI=arm64-v8a;   CTL=controller_arm64 ;;
  armv7l|armv8l) ABI=armeabi-v7a; CTL=controller_armv7 ;;
  x86_64)        ABI=x86_64;      CTL=controller_x86_64 ;;
  i686|i386)     ABI=x86;         CTL=controller_x86 ;;
  *) die "unsupported host arch: $(uname -m)" ;;
esac
log "target ABI: $ABI   version: $VERSION ($VERSION_CODE)"

# ── fresh staging ────────────────────────────────────────────────────────────
rm -rf "$BUILD_DIR"
mkdir -p "$STAGE/zygisk" "$STAGE/webroot" "$STAGE/system"

# ── assemble static module files ─────────────────────────────────────────────
log "assembling module tree"
cp -r module/.   "$STAGE/"            # everything that ships at module root: customize/service/uninstall.sh,
                                      # system.prop, config.json, banner.png, cpuinfo_spoof, common/,
                                      # COPG.json, list.json, META-INF/
cp -r webroot/.  "$STAGE/webroot/"
cp -r system/.   "$STAGE/system/" 2>/dev/null || true

cat > "$STAGE/module.prop" <<EOF
id=COPG
name=✨ COPG SPOOF ✨
version=$VERSION
versionCode=$VERSION_CODE
author=AlirezaParsi
description=Spoof your device for games/apps to unlock new features.
support=https://t.me/theaosp
updateJson=https://raw.githubusercontent.com/AlirezaParsi/COPG/refs/heads/JSON/update.json
minMagisk=20.4
banner=banner.png
EOF

# ── build Zygisk companion (.so) ─────────────────────────────────────────────
log "building zygisk companion ($ABI.so)"
clang++ -std=c++17 -shared -fPIC -Os -flto -ffunction-sections -fdata-sections \
  -Wl,--gc-sections -Wl,--exclude-libs,ALL \
  -I src/include src/spoof_module.cpp src/atexit.cpp -llog \
  -o "$STAGE/zygisk/$ABI.so"
llvm-strip --strip-unneeded "$STAGE/zygisk/$ABI.so"
ok "zygisk/$ABI.so  ($(du -h "$STAGE/zygisk/$ABI.so" | cut -f1))"

# ── build unified_controller (libc++ bundled, libc from /system → ships fine) ─
log "building $CTL"
clang++ -std=c++17 -O3 -Os -fPIE -pie -static-libstdc++ \
  -fdata-sections -ffunction-sections -Wl,--gc-sections \
  -I src/include src/unified_controller.cpp -o "$STAGE/$CTL"
llvm-strip --strip-unneeded "$STAGE/$CTL"
[ -n "$UPX" ] && upx --best --lzma -q "$STAGE/$CTL" >/dev/null 2>&1 || true
ok "$CTL  ($(du -h "$STAGE/$CTL" | cut -f1))"

# ── permissions ──────────────────────────────────────────────────────────────
chmod 0755 "$STAGE/customize.sh" "$STAGE/service.sh" "$STAGE/uninstall.sh" \
           "$STAGE/META-INF/com/google/android/update-binary" \
           "$STAGE/$CTL" "$STAGE/zygisk/$ABI.so" 2>/dev/null || true
chmod 0644 "$STAGE/COPG.json" "$STAGE/list.json" 2>/dev/null || true

# ── package ──────────────────────────────────────────────────────────────────
mkdir -p "$OUT_DIR"
ZIP="$OUT_DIR/COPG-v$VERSION-$ABI.zip"
rm -f "$ZIP"
( cd "$STAGE" && zip -r -q -X "$ZIP" . -x '.*' )
ok "packaged → $ZIP  ($(du -h "$ZIP" | cut -f1))"
log "done"
