# ================================================
# COPG Module Installation Script
# ================================================

INSTALL_SUCCESS=true

print_box_start() {
  ui_print "╔═════════════════════════════════╗"
  ui_print "                                 "
}

print_box_end() {
  ui_print "                                 "
  ui_print "╚═════════════════════════════════╝"
}

print_empty_line() {
  ui_print "                                 "
}

print_failure_and_exit() {
  local section="$1"
  print_empty_line
  ui_print " ✗ Installation Failed!          "
  if [ "$section" = "binary" ]; then
    print_empty_line
    print_empty_line
    print_empty_line
  fi
  print_box_end
  exit 1
}

grep_prop() {
  local PROP_FILE="$1"
  local PROP_NAME="$2"
  if [ -f "$PROP_FILE" ]; then
    grep "^${PROP_NAME}=" "$PROP_FILE" | cut -d'=' -f2- | head -n 1
  else
    echo ""
  fi
}

print_module_version() {
  print_box_start
  ui_print "      ✦ COPG Module Version ✦    "
  print_empty_line
  MODULE_PROP="$MODPATH/module.prop"
  if [ -f "$MODULE_PROP" ]; then
    MODULE_VERSION=$(grep_prop "$MODULE_PROP" "version")
    MODULE_VERSION_CODE=$(grep_prop "$MODULE_PROP" "versionCode")
    if [ -n "$MODULE_VERSION" ]; then
      ui_print " ✔ Module Version: $MODULE_VERSION "
      [ -n "$MODULE_VERSION_CODE" ] && ui_print " ✔ Version Code: $MODULE_VERSION_CODE "
    else
      ui_print " ✗ Could Not Read Module Version! "
    fi
  else
    ui_print " ✗ module.prop Not Found!        "
  fi
  print_box_end
  print_empty_line
}

check_zygisk() {
  ZYGISK_NEXT_PATH="/data/adb/modules/zygisksu"
  REZYGISK_PATH="/data/adb/modules/rezygisk"

  print_box_start
  ui_print "      ✦ Zygisk Detection ✦      "
  print_empty_line

  DETECTED_ROOT_SOLUTIONS=""
  ROOT_SOLUTION_COUNT=0

  if command -v apd >/dev/null; then
    DETECTED_ROOT_SOLUTIONS="$DETECTED_ROOT_SOLUTIONS APatch"
    ROOT_SOLUTION_COUNT=$((ROOT_SOLUTION_COUNT + 1))
    ROOT_SOLUTION="APatch"
    MANAGER_NAME="APatch Manager"
  fi

  if command -v ksud >/dev/null; then
    DETECTED_ROOT_SOLUTIONS="$DETECTED_ROOT_SOLUTIONS KernelSU"
    ROOT_SOLUTION_COUNT=$((ROOT_SOLUTION_COUNT + 1))
    if [ $ROOT_SOLUTION_COUNT -eq 1 ]; then
      ROOT_SOLUTION="KernelSU"
      MANAGER_NAME="KernelSU Manager"
    fi
  fi

  if command -v magisk >/dev/null; then
    DETECTED_ROOT_SOLUTIONS="$DETECTED_ROOT_SOLUTIONS Magisk"
    ROOT_SOLUTION_COUNT=$((ROOT_SOLUTION_COUNT + 1))
    if [ $ROOT_SOLUTION_COUNT -eq 1 ]; then
      ROOT_SOLUTION="Magisk"
      MANAGER_NAME="Magisk Manager"
    fi
  fi

  if [ $ROOT_SOLUTION_COUNT -gt 1 ]; then
    ui_print " ✗ Multiple Root Solutions Found!"
    ui_print " ➤ Detected:$DETECTED_ROOT_SOLUTIONS"
    ui_print " ➤ Only One Root Solution Allowed"
    print_failure_and_exit "zygisk"
  elif [ $ROOT_SOLUTION_COUNT -eq 0 ]; then
    ui_print " ✗ No Supported Root Solution!   "
    ui_print " ➤ Supported Solutions:          "
    ui_print " ➤ • Magisk v26.4+ (ReZygisk/Zygisk Next)"
    ui_print " ➤ • KernelSU v0.7.0+ (Zygisk Next)"
    ui_print " ➤ • APatch v1.0.7+ (Zygisk Next)"
    print_failure_and_exit "zygisk"
  else
    ui_print " ➔ Root Solution: $ROOT_SOLUTION "
  fi

  # ── Per-variant ACTIVE state ──────────────────────────────────────────────
  # A variant counts as active only if its module dir exists AND has no
  # `disable` marker. Each is checked independently, so a disabled variant can
  # never mask another that IS active (e.g. Zygisk Next disabled + ReZygisk on
  # → ReZygisk still wins). `disable` on one says nothing about the other.
  ZN_ACTIVE=false; RZ_ACTIVE=false
  [ -d "$ZYGISK_NEXT_PATH" ] && [ ! -f "$ZYGISK_NEXT_PATH/disable" ] && ZN_ACTIVE=true
  [ -d "$REZYGISK_PATH" ]    && [ ! -f "$REZYGISK_PATH/disable" ]    && RZ_ACTIVE=true
  ZYGISK_INSTALLED=false
  { [ -d "$ZYGISK_NEXT_PATH" ] || [ -d "$REZYGISK_PATH" ]; } && ZYGISK_INSTALLED=true

  # ── Magisk: built-in Zygisk must be OFF ───────────────────────────────────
  # ReZygisk / Zygisk Next replace Magisk's native Zygisk and only load when
  # the built-in one is disabled. If native Zygisk is ON they silently fail to
  # work even though the module dir is present — so block install and tell the
  # user to turn it off (instead of falsely reporting the variant "Active").
  if [ "$ROOT_SOLUTION" = "Magisk" ]; then
    ZYGISK_STATUS=$(magisk --sqlite "SELECT value FROM settings WHERE key='zygisk';" 2>/dev/null)
    if [ "$ZYGISK_STATUS" = "value=1" ]; then
      ui_print " ✗ Magisk: Built-in Zygisk Enabled!"
      ui_print " ➤ Disable Native Zygisk in Magisk Settings"
      ui_print " ➤ ReZygisk / Zygisk Next need it OFF"
      print_failure_and_exit "zygisk"
    fi
  fi

  # ── Require an ACTIVE standalone Zygisk variant ───────────────────────────
  if $RZ_ACTIVE; then
    ui_print " ✔ $ROOT_SOLUTION: ReZygisk Active    "
    print_box_end
    return
  elif $ZN_ACTIVE; then
    ui_print " ✔ $ROOT_SOLUTION: Zygisk Next Active  "
    print_box_end
    return
  elif $ZYGISK_INSTALLED; then
    ui_print " ✗ $ROOT_SOLUTION: Zygisk Module Disabled!"
    ui_print " ➤ Enable ReZygisk / Zygisk Next in $MANAGER_NAME"
    print_failure_and_exit "zygisk"
  else
    ui_print " ✗ $ROOT_SOLUTION: No Zygisk Module Found!"
    ui_print " ➤ Install ReZygisk or Zygisk Next       "
    print_failure_and_exit "zygisk"
  fi
}

cleanup_unused_architectures() {
  ABI_LIST=$(getprop ro.product.cpu.abilist)
  
  # IMPORTANT: Check in order from most specific to least specific
  # x86_64 is the most specific (emulators with full compatibility)
  if echo "$ABI_LIST" | grep -q "x86_64"; then
    ui_print " 🧹 x86_64 emulator - keeping all architectures..."
    # Keep everything for Houdini compatibility
    ui_print " ✓ Kept: x86_64, x86, ARM64, ARM32"
  
  # x86 32-bit (rare)
  elif echo "$ABI_LIST" | grep -q "x86" && ! echo "$ABI_LIST" | grep -q "x86_64"; then
    ui_print " 🧹 x86 32-bit device - keeping x86 + ARM32..."
    rm -f "$MODPATH/zygisk/arm64-v8a.so" "$MODPATH/zygisk/x86_64.so" 2>/dev/null
    ui_print " ✓ Kept: x86, ARM32"
  
  # ARM devices (phones, tablets)
  elif echo "$ABI_LIST" | grep -qE "arm64|armeabi|armv7|arm"; then
    ui_print " 🧹 ARM device - removing x86/x86_64 libraries..."
    rm -f "$MODPATH/zygisk/x86.so" "$MODPATH/zygisk/x86_64.so" 2>/dev/null
    ui_print " ✓ Kept: ARM64 + ARM32"
  fi
  
  # Remove all controller source files (we only need 'controller')
  rm -f "$MODPATH/controller_arm64" "$MODPATH/controller_armv7" "$MODPATH/controller_x86" "$MODPATH/controller_x86_64" 2>/dev/null
}

print_module_version

if ! $BOOTMODE; then
  print_box_start
  ui_print "      ✦ Installation Error ✦     "
  print_empty_line
  ui_print " ✗ Recovery Mode Not Supported!  "
  ui_print " ➤ Install via Magisk/KSU/APatch "
  print_failure_and_exit "initial"
fi

if [ "$API" -lt 26 ]; then
  print_box_start
  ui_print "      ✦ Installation Error ✦     "
  print_empty_line
  ui_print " ✗ Android Version Too Old!      "
  ui_print " ➤ Requires Android 9.0+         "
  print_failure_and_exit "initial"
fi

if $INSTALL_SUCCESS; then
  check_zygisk || {
    INSTALL_SUCCESS=false
  }
fi

if $INSTALL_SUCCESS; then
  print_box_start
  ui_print "      ✦ Installing Controller ✦  "
  print_empty_line
  ui_print " ⚙ Detecting Device Architecture "

  ARM64_VARIANTS="arm64-v8a|armv8-a|arm64|aarch64"
  ARM32_VARIANTS="armeabi-v7a|armeabi|armv7-a|armv7l|armhf|arm"
  X86_64_VARIANTS="x86_64|amd64"
  X86_VARIANTS="x86|i386|i686"

  ABI_LIST=$(getprop ro.product.cpu.abilist)
  ui_print " 📜 Supported ABIs: $ABI_LIST"

  if $INSTALL_SUCCESS; then
    CONTROLLER_INSTALLED=false

    for ABI in $(echo "$ABI_LIST" | tr ',' ' '); do
      if echo "$ABI" | grep -qE "$ARM64_VARIANTS"; then
        if [ -f "$MODPATH/controller_arm64" ]; then
          mv "$MODPATH/controller_arm64" "$MODPATH/controller" || {
            ui_print " ✗ Failed to Rename ARM64 Controller!  "
            print_failure_and_exit "binary"
          }
          chmod 0755 "$MODPATH/controller" || {
            ui_print " ✗ Failed to Set Permissions (controller)!  "
            print_failure_and_exit "binary"
          }
          ui_print " ✔ Installed ARM64 Controller     "
          ui_print " ➤ ($ABI)                        "
          CONTROLLER_INSTALLED=true
          
          rm -f "$MODPATH/controller_armv7" "$MODPATH/controller_x86" "$MODPATH/controller_x86_64" 2>/dev/null
          break
        fi
      elif echo "$ABI" | grep -qE "$ARM32_VARIANTS"; then
        if [ -f "$MODPATH/controller_armv7" ]; then
          mv "$MODPATH/controller_armv7" "$MODPATH/controller" || {
            ui_print " ✗ Failed to Rename ARM32 Controller!  "
            print_failure_and_exit "binary"
          }
          chmod 0755 "$MODPATH/controller" || {
            ui_print " ✗ Failed to Set Permissions (controller)!  "
            print_failure_and_exit "binary"
          }
          ui_print " ✔ Installed ARM32 Controller     "
          ui_print " ➤ ($ABI)                        "
          CONTROLLER_INSTALLED=true
          
          rm -f "$MODPATH/controller_arm64" "$MODPATH/controller_x86" "$MODPATH/controller_x86_64" 2>/dev/null
          break
        fi
      elif echo "$ABI" | grep -qE "$X86_64_VARIANTS"; then
        if [ -f "$MODPATH/controller_x86_64" ]; then
          mv "$MODPATH/controller_x86_64" "$MODPATH/controller" || {
            ui_print " ✗ Failed to Rename x86_64 Controller!  "
            print_failure_and_exit "binary"
          }
          chmod 0755 "$MODPATH/controller" || {
            ui_print " ✗ Failed to Set Permissions (controller)!  "
            print_failure_and_exit "binary"
          }
          ui_print " ✔ Installed x86_64 Controller     "
          ui_print " ➤ ($ABI)                        "
          CONTROLLER_INSTALLED=true
          
          rm -f "$MODPATH/controller_arm64" "$MODPATH/controller_armv7" "$MODPATH/controller_x86" 2>/dev/null
          break
        fi
      elif echo "$ABI" | grep -qE "$X86_VARIANTS"; then
        if [ -f "$MODPATH/controller_x86" ]; then
          mv "$MODPATH/controller_x86" "$MODPATH/controller" || {
            ui_print " ✗ Failed to Rename x86 Controller!  "
            print_failure_and_exit "binary"
          }
          chmod 0755 "$MODPATH/controller" || {
            ui_print " ✗ Failed to Set Permissions (controller)!  "
            print_failure_and_exit "binary"
          }
          ui_print " ✔ Installed x86 Controller     "
          ui_print " ➤ ($ABI)                        "
          CONTROLLER_INSTALLED=true
          
          rm -f "$MODPATH/controller_arm64" "$MODPATH/controller_armv7" "$MODPATH/controller_x86_64" 2>/dev/null
          break
        fi
      fi
    done

    if ! $CONTROLLER_INSTALLED; then
      ui_print " ✗ No Compatible Controller Found! "
      ui_print " ➤ Supported Architectures:      "
      ui_print " ➤ • ARM64 (arm64-v8a)          "
      ui_print " ➤ • ARM32 (armeabi-v7a)        "
      ui_print " ➤ • x86_64                     "
      ui_print " ➤ • x86                        "
      print_failure_and_exit "binary"
    fi
  print_box_end
  print_empty_line
fi

  if $INSTALL_SUCCESS; then
    chmod 0755 "$MODPATH/service.sh" 2>/dev/null
    chmod 0644 "$MODPATH/COPG.json" "$MODPATH/list.json" 2>/dev/null
    chmod 0444 "$MODPATH/cpuinfo_spoof" 2>/dev/null

    for file in "$MODPATH/COPG.json" "$MODPATH/list.json" "$MODPATH/cpuinfo_spoof" \
                "$MODPATH/service.sh"; do
      if [ -f "$file" ]; then
        chcon u:object_r:system_file:s0 "$file" 2>/dev/null
      fi
    done
    
    # Clean up unused architecture files to reduce module size
    cleanup_unused_architectures
  fi


  if $INSTALL_SUCCESS; then
    print_empty_line
    print_box_start
    ui_print " ✅ Module Successfully Installed "
    print_box_end
  fi
fi

if ! $INSTALL_SUCCESS; then
  exit 1
fi
