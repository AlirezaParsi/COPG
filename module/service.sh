#!/system/bin/sh

until [ "$(getprop sys.boot_completed)" = "1" ]; do
    sleep 2
done

sleep 5
chmod 0644 data/adb/modules/COPG/COPG.json
chcon u:object_r:system_file:s0 /data/adb/modules/COPG/COPG.json
exec /data/adb/modules/COPG/controller >/dev/null 2>&1 &
