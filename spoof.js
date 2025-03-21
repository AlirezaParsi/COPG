// Only log during debug (toggle via config.json if needed)
var debug = false;
function log(msg) { if (debug) console.log("[COPG] " + msg); }
log("Frida 16.7.0 loaded");

// Load config.json
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
                ro_product_model: device["ro.product.model"] || ""
            };
        });
    }
}
log("Loaded " + Object.keys(packageMap).length + " package mappings");

// Hook Java layer
Java.perform(function() {
    var System = Java.use("java.lang.System");
    var Build = Java.use("android.os.Build");
    var currentPackage = Java.use("android.app.ActivityThread").currentProcessName();

    // Hook System.getProperty
    System.getProperty.implementation = function(key) {
        var pkg = currentPackage.value;
        if (packageMap[pkg] && key === "ro.product.model" && packageMap[pkg].ro_product_model) {
            log("Spoofing ro.product.model to " + packageMap[pkg].ro_product_model);
            return packageMap[pkg].ro_product_model;
        }
        return this.getProperty(key);
    };

    // Spoof Build fields
    if (packageMap[currentPackage.value]) {
        var info = packageMap[currentPackage.value];
        Build.MODEL.value = info.model;
        Build.BRAND.value = info.brand;
        Build.DEVICE.value = info.device;
        Build.MANUFACTURER.value = info.manufacturer;
        log("Spoofed Build fields for " + currentPackage.value);
    }
});
