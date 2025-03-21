var debug = true;
function log(msg) { if (debug) console.log("[COPG] " + msg); }
log("Frida 16.7.0 loaded");

function loadConfig() {
    try {
        return JSON.parse(readFile("/data/adb/modules/COPG/config.json"));
    } catch (e) {
        log("Failed to load config.json: " + e);
        return {};
    }
}

var config = loadConfig();
var packageMap = {};
for (var key in config) {
    if (key.endsWith("_DEVICE")) continue;
    var packages = config[key];
    var deviceKey = key + "_DEVICE";
    if (config[deviceKey]) {
        var device = config[deviceKey];
        packages.forEach(function(pkg) {
            packageMap[pkg] = {
                brand: device.BRAND,
                device: device.DEVICE,
                manufacturer: device.MANUFACTURER,
                model: device.MODEL,
                ro_product_model: device["ro.product.model"] || device.MODEL
            };
        });
    }
}
log("Loaded " + Object.keys(packageMap).length + " package mappings");

Java.perform(function() {
    var System = Java.use("java.lang.System");
    var Build = Java.use("android.os.Build");
    var currentPackage = Java.use("android.app.ActivityThread").currentProcessName();

    log("Running in package: " + currentPackage.value);

    System.getProperty.implementation = function(key) {
        var pkg = currentPackage.value;
        log("getProperty called with key: " + key + " for " + pkg);
        if (packageMap[pkg] && key === "ro.product.model" && packageMap[pkg].ro_product_model) {
            log("Spoofing ro.product.model to " + packageMap[pkg].ro_product_model);
            return packageMap[pkg].ro_product_model;
        }
        return this.getProperty(key);
    };

    if (packageMap[currentPackage.value]) {
        var info = packageMap[currentPackage.value];
        Build.MODEL.value = info.model;
        Build.BRAND.value = info.brand;
        Build.DEVICE.value = info.device;
        Build.MANUFACTURER.value = info.manufacturer;
        log("Spoofed Build fields - MODEL: " + info.model + ", BRAND: " + info.brand);
    } else {
        log("No spoof data for " + currentPackage.value);
    }
});
