# ================================================
# COPG Module Installation Script
# ================================================

INSTALL_SUCCESS=true
ENABLE_GPHOTO_SPOOF=true

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
  if [ "$section" = "binary" ] || [ "$section" = "gphoto" ]; then
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

# ====================== Architecture Detection ======================
detect_architecture() {
  ABI_LIST=$(getprop ro.product.cpu.abilist)
  ui_print " 📜 Supported ABIs: $ABI_LIST"

  IS_ARM64=false
  IS_ARM32=false
  IS_X86_64=false
  IS_X86=false

  if echo "$ABI_LIST" | grep -qE "arm64-v8a|aarch64"; then
    IS_ARM64=true
  fi
  if echo "$ABI_LIST" | grep -qE "armeabi-v7a|armv7"; then
    IS_ARM32=true
  fi
  if echo "$ABI_LIST" | grep -qE "x86_64"; then
    IS_X86_64=true
  fi
  if echo "$ABI_LIST" | grep -qE "x86" && ! echo "$ABI_LIST" | grep -qE "x86_64"; then
    IS_X86=true
  fi
}

# ====================== Install Controller ======================
install_controller() {
  print_box_start
  ui_print "      ✦ Installing Controller ✦  "
  print_empty_line

  CONTROLLER_INSTALLED=false

  # اولویت نصب: x86_64 > arm64 > x86 > arm32
  if $IS_X86_64 && [ -f "$MODPATH/controller_x86_64" ]; then
    mv "$MODPATH/controller_x86_64" "$MODPATH/controller"
    ui_print " ✔ Installed x86_64 Controller"
    CONTROLLER_INSTALLED=true

  elif $IS_ARM64 && [ -f "$MODPATH/controller_arm64" ]; then
    mv "$MODPATH/controller_arm64" "$MODPATH/controller"
    ui_print " ✔ Installed ARM64 Controller"
    CONTROLLER_INSTALLED=true

  elif $IS_X86 && [ -f "$MODPATH/controller_x86" ]; then
    mv "$MODPATH/controller_x86" "$MODPATH/controller"
    ui_print " ✔ Installed x86 Controller"
    CONTROLLER_INSTALLED=true

  elif $IS_ARM32 && [ -f "$MODPATH/controller_armv7" ]; then
    mv "$MODPATH/controller_armv7" "$MODPATH/controller"
    ui_print " ✔ Installed ARM32 Controller"
    CONTROLLER_INSTALLED=true
  fi

  if $CONTROLLER_INSTALLED; then
    chmod 0755 "$MODPATH/controller"
    ui_print " ➤ Primary controller activated"
  else
    ui_print " ✗ No Compatible Controller Found!"
    print_failure_and_exit "binary"
  fi

  print_empty_line
  ui_print " 🧹 Cleaning up unused controller binaries..."

  if $IS_X86_64 || $IS_X86; then
    ui_print " ➤ Keeping all controllers (Emulator mode)"
  else
    if $IS_ARM64; then
      rm -f "$MODPATH/controller_x86_64" "$MODPATH/controller_x86" 2>/dev/null
      ui_print " ➤ Kept: arm64 + armv7 (fallback)"
    else
      rm -f "$MODPATH/controller_arm64" "$MODPATH/controller_x86_64" "$MODPATH/controller_x86" 2>/dev/null
      ui_print " ➤ Kept: Only armv7"
    fi
  fi

  print_box_end
  print_empty_line
}

# ====================== Cleanup Zygisk Libraries ======================
cleanup_zygisk() {
  print_box_start
  ui_print "      ✦ Zygisk Libraries Cleanup ✦"
  print_empty_line

  if $IS_X86_64 || $IS_X86; then
    ui_print " ➤ Keeping all Zygisk libraries (Emulator)"
  else
    if $IS_ARM64; then
      rm -f "$MODPATH/zygisk/x86_64.so" "$MODPATH/zygisk/x86.so" 2>/dev/null
      ui_print " ➤ Kept: arm64-v8a.so + armeabi-v7a.so"
    else
      rm -f "$MODPATH/zygisk/arm64-v8a.so" "$MODPATH/zygisk/x86_64.so" "$MODPATH/zygisk/x86.so" 2>/dev/null
      ui_print " ➤ Kept: Only armeabi-v7a.so"
    fi
  fi

  # چک نهایی
  if [ ! -f "$MODPATH/zygisk/arm64-v8a.so" ] && \
     [ ! -f "$MODPATH/zygisk/armeabi-v7a.so" ] && \
     [ ! -f "$MODPATH/zygisk/x86_64.so" ] && \
     [ ! -f "$MODPATH/zygisk/x86.so" ]; then
    ui_print " ✗ No Zygisk library found after cleanup!"
    print_failure_and_exit "binary"
  fi

  print_box_end
  print_empty_line
}

# ====================== Zygisk Detection (Original) ======================
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

  if [ "$ROOT_SOLUTION" = "Magisk" ]; then
    ZYGISK_STATUS=$(magisk --sqlite "SELECT value FROM settings WHERE key='zygisk';" 2>/dev/null)
    if [ "$ZYGISK_STATUS" = "value=1" ]; then
      if [ -d "$REZYGISK_PATH" ]; then
        if [ -f "$REZYGISK_PATH/disable" ]; then
          ui_print " ✗ ReZygisk Installed but Disabled!"
          ui_print " ➤ Enable ReZygisk in Modules"
          print_failure_and_exit "zygisk"
        else
          ui_print " ✔ Magisk: ReZygisk Active    "
          print_box_end
          return
        fi
      elif [ -d "$ZYGISK_NEXT_PATH" ]; then
        if [ -f "$ZYGISK_NEXT_PATH/disable" ]; then
          ui_print " ✗ Zygisk Next Installed but Disabled!"
          ui_print " ➤ Enable Zygisk Next in Modules"
          print_failure_and_exit "zygisk"
        else
          ui_print " ✔ Magisk: Zygisk Next Active    "
          print_box_end
          return
        fi
      else
        ui_print " ✗ Magisk: Native Zygisk Not Supported!"
        ui_print " ➤ Install ReZygisk or Zygisk Next"
        ui_print " ➤ Disable Native Zygisk in Settings"
        print_failure_and_exit "zygisk"
      fi
    else
      if [ -d "$REZYGISK_PATH" ]; then
        if [ -f "$REZYGISK_PATH/disable" ]; then
          ui_print " ✗ ReZygisk Disabled!         "
          ui_print " ➤ Enable in $MANAGER_NAME       "
          print_failure_and_exit "zygisk"
        else
          ui_print " ✔ Magisk: ReZygisk Active    "
          print_box_end
          return
        fi
      elif [ -d "$ZYGISK_NEXT_PATH" ]; then
        if [ -f "$ZYGISK_NEXT_PATH/disable" ]; then
          ui_print " ✗ Zygisk Next Disabled!         "
          ui_print " ➤ Enable in $MANAGER_NAME       "
          print_failure_and_exit "zygisk"
        else
          ui_print " ✔ Magisk: Zygisk Next Active    "
          print_box_end
          return
        fi
      else
        ui_print " ✗ Magisk: No Zygisk Detected!   "
        ui_print " ➤ Install ReZygisk or Zygisk Next"
        print_failure_and_exit "zygisk"
      fi
    fi
  else
    if [ -d "$ZYGISK_NEXT_PATH" ] || [ -d "$REZYGISK_PATH" ]; then
      if [ -f "$ZYGISK_NEXT_PATH/disable" ] || [ -f "$REZYGISK_PATH/disable" ]; then
        ui_print " ✗ $ROOT_SOLUTION: Zygisk Module Disabled! "
        ui_print " ➤ Enable in $MANAGER_NAME    "
        print_failure_and_exit "zygisk"
      else
        ui_print " ✔ $ROOT_SOLUTION: Zygisk Module Active    "
        print_box_end
      fi
    else
      ui_print " ✗ $ROOT_SOLUTION: Zygisk Module Not Found! "
      ui_print " ➤ Install Zygisk Next/ReZygisk Module    "
      print_failure_and_exit "zygisk"
    fi
  fi
}

prompt_gphoto_spoof() {
  print_box_start
  ui_print "      ✦ Google Photos Spoof ✦    "
  print_empty_line
  ui_print " ❓ Enable Unlimited Photos?      "
  ui_print " ➤ Volume Up: Yes (Recommended)  "
  ui_print " ➤ Volume Down: No               "
  print_box_end

  TIMEOUT=10
  START_TIME=$(date +%s)

  while true; do
    CURRENT_TIME=$(date +%s)
    ELAPSED=$((CURRENT_TIME - START_TIME))
    if [ $ELAPSED -ge $TIMEOUT ]; then
      print_empty_line
      ui_print " ⏰ Timeout (10s) - Disabled"
      ENABLE_GPHOTO_SPOOF=false
      return
    fi

    if command -v timeout >/dev/null 2>&1; then
      EVENT=$(timeout 0.1 getevent -lc1 2>/dev/null | tr -d '\r')
    else
      EVENT=$(getevent -lc1 2>/dev/null | tr -d '\r')
    fi

    if [ -n "$EVENT" ]; then
      if echo "$EVENT" | grep -q "KEY_VOLUMEUP.*DOWN"; then
        print_empty_line
        ui_print " ✅ Enabled Unlimited Photos"
        ENABLE_GPHOTO_SPOOF=true
        return
      elif echo "$EVENT" | grep -q "KEY_VOLUMEDOWN.*DOWN"; then
        print_empty_line
        ui_print " ❌ Disabled Unlimited Photos"
        ENABLE_GPHOTO_SPOOF=false
        return
      fi
    fi

    sleep 0.1
  done
}

setup_gphoto_spoof() {
  print_box_start
  ui_print "      ✦ Google Photos Spoof ✦    "
  print_empty_line
  ui_print " ⚙ Configuring Sysconfig Files   "
  mkdir -p "$MODPATH/system/product/etc/sysconfig" "$MODPATH/system/etc/sysconfig" 2>/dev/null
  if [ -d "/system/product/etc/sysconfig" ]; then
    find /system/product/etc/sysconfig -type f | while read -r file; do
      if grep -qE "PIXEL_2020_|PIXEL_2021_|PIXEL_2019_PRELOAD|PIXEL_2018_PRELOAD|PIXEL_2017_PRELOAD|PIXEL_2022_" "$file"; then
        filename=$(basename "$file")
        grep -vE "PIXEL_2020_|PIXEL_2021_|PIXEL_2022_|PIXEL_2018_PRELOAD|PIXEL_2019_PRELOAD|PIXEL_2017_PRELOAD" "$file" > \
          "$MODPATH/system/product/etc/sysconfig/$filename" 2>/dev/null
      fi
    done
  fi
  if [ -d "/system/etc/sysconfig" ]; then
    find /system/etc/sysconfig -type f | while read -r file; do
      if grep -qE "PIXEL_2020_|PIXEL_2021_|PIXEL_2019_PRELOAD|PIXEL_2018_PRELOAD|PIXEL_2017_PRELOAD|PIXEL_2022_" "$file"; then
        filename=$(basename "$file")
        grep -vE "PIXEL_2020_|PIXEL_2021_|PIXEL_2022_|PIXEL_2018_PRELOAD|PIXEL_2019_PRELOAD|PIXEL_2017_PRELOAD" "$file" > \
          "$MODPATH/system/etc/sysconfig/$filename" 2>/dev/null
      fi
    done
  fi
  find "$MODPATH/system/product/etc/sysconfig" "$MODPATH/system/etc/sysconfig" -type f 2>/dev/null | while read -r file; do
    chmod 0644 "$file" 2>/dev/null
    chcon u:object_r:system_file:s0 "$file" 2>/dev/null
  done

  ui_print " ✔ Unlimited Photos Configured   "
  print_box_end
}

cleanup_gphoto_directories() {
  rm -rf "$MODPATH/system/etc/sysconfig" 2>/dev/null
  rm -rf "$MODPATH/system/product/etc/sysconfig" 2>/dev/null
  rm -rf "$MODPATH/product/etc/sysconfig" 2>/dev/null
  
  find "$MODPATH" -type d -empty -delete 2>/dev/null
}

# ====================== Main Installation Flow ======================

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

detect_architecture

if $INSTALL_SUCCESS; then
  check_zygisk || {
    INSTALL_SUCCESS=false
  }
fi

if $INSTALL_SUCCESS; then
  install_controller || {
    INSTALL_SUCCESS=false
  }
fi

if $INSTALL_SUCCESS; then
  cleanup_zygisk || {
    INSTALL_SUCCESS=false
  }
fi

if $INSTALL_SUCCESS; then
  chmod 0755 "$MODPATH/service.sh" "$MODPATH/action.sh" "$MODPATH/update_config.sh" 2>/dev/null
  chmod 0644 "$MODPATH/COPG.json" "$MODPATH/list.json" 2>/dev/null
  chmod 0444 "$MODPATH/cpuinfo_spoof" 2>/dev/null
  
  for file in "$MODPATH/COPG.json" "$MODPATH/list.json" "$MODPATH/cpuinfo_spoof" \
              "$MODPATH/service.sh" "$MODPATH/action.sh" "$MODPATH/update_config.sh"; do
    if [ -f "$file" ]; then
      chcon u:object_r:system_file:s0 "$file" 2>/dev/null
    fi
  done
fi

if $INSTALL_SUCCESS; then
  prompt_gphoto_spoof
  if $ENABLE_GPHOTO_SPOOF; then
    setup_gphoto_spoof || {
      INSTALL_SUCCESS=false
    }
  else
    print_box_start
    ui_print "      ✦ Google Photos Spoof ✦    "
    print_empty_line
    ui_print " ⚙ Removing Google Photos Config "
    if [ -f "$MODPATH/COPG.json" ]; then
      sed -i '/com\.google\.android\.apps\.photos/d' "$MODPATH/COPG.json" 2>/dev/null
      chmod 0644 "$MODPATH/COPG.json" 2>/dev/null
      chcon u:object_r:system_file:s0 "$MODPATH/COPG.json" 2>/dev/null
    fi
    
    cleanup_gphoto_directories
    
    ui_print " ✔ Unlimited Photos Disabled    "
    print_box_end
  fi
fi

if $INSTALL_SUCCESS; then
  print_empty_line
  print_box_start
  ui_print " ✅ Module Successfully Installed "
  print_box_end
fi

if ! $INSTALL_SUCCESS; then
  exit 1
fi
