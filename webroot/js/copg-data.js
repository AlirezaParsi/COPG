/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   copg-data.js — COPG.json / list.json data layer
   (bridge + parse + model + persistence; no DOM)

   COPG.json shape (shared with zygisk/binaries):
     "cpu_spoof": { blacklist:[pkg…], cpu_only_packages:[pkg…] }   // global, optional
     "PACKAGES_<KEY>":        [ "com.x:blocked", "com.y:with_cpu" ] // a device's game list
     "PACKAGES_<KEY>_DEVICE": { BRAND, DEVICE, MANUFACTURER, MODEL,
                                FINGERPRINT, PRODUCT, SERIAL?, ANDROID_VERSION?, SDK_INT? }
   Insertion order of keys is meaningful and preserved on save (keyOrder).
   Package tags are colon suffixes: pkg:blocked, pkg:with_cpu, pkg:got.
   Logic ported from the previous WebUI (old.js) for full parity.
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
(function (w) {

  const MODULE_DIR = '/data/adb/modules/COPG';
  const CONFIG_PATH = `${MODULE_DIR}/COPG.json`;
  const LIST_PATH   = `${MODULE_DIR}/list.json`;
  const BACKUP_DIR  = '/sdcard/Download/COPG';
  const LOG_DIR     = '/sdcard/Download/COPG/LOGS';
  // GitHub "Sync" — same source the module's update_config.sh pulls from
  // (AlirezaParsi/COPG, JSON branch). Downloaded straight to stdout, parsed +
  // applied + saved through the normal path; downloader fallback curl → wget →
  // busybox wget (each a single, simple command — no compound shell).
  // COPG.json / list.json live under module/ in the repo; branch stays JSON (this
  // repo is PR'd into JSON), so only the path carries the module/ prefix.
  const SYNC_CONFIG_URL = 'https://raw.githubusercontent.com/AlirezaParsi/COPG/refs/heads/JSON/module/COPG.json';
  const SYNC_LIST_URL   = 'https://raw.githubusercontent.com/AlirezaParsi/COPG/refs/heads/JSON/module/list.json';

  /* ─── KernelSU exec bridge (Promise wrapper, from old.js) ─── */
  function execCommand(command) {
    return new Promise((resolve, reject) => {
      const cb = `exec_cb_${Date.now()}_${Math.floor(Math.random() * 1e6)}`;
      w[cb] = (errno, stdout, stderr) => {
        delete w[cb];
        if (errno === 0) resolve(stdout || '');
        else reject(stderr || `Command failed (code ${errno})`);
      };
      try {
        if (typeof ksu !== 'undefined' && ksu.exec) ksu.exec(command, '{}', cb);
        else { delete w[cb]; reject('KSU API not available'); }
      } catch (e) { delete w[cb]; reject(String(e)); }
    });
  }

  const hasBridge = () => (typeof ksu !== 'undefined' && !!ksu.exec);

  /* shell-single-quote escape: ' → '\'' */
  const shq = s => String(s).replace(/'/g, "'\\''");

  /* ─── Tag helpers (verbatim parity with old.js) ─── */
  const clean   = pkg => pkg.split(':')[0];
  const tagsOf  = pkg => pkg.split(':').slice(1);
  const join    = (name, tags) => tags.length ? `${name}:${tags.join(':')}` : name;
  function addTag(pkg, tag) {
    const t = tagsOf(pkg); if (!t.includes(tag)) t.push(tag);
    return join(clean(pkg), t);
  }
  function removeTag(pkg, tag) {
    return join(clean(pkg), tagsOf(pkg).filter(x => x !== tag));
  }
  const hasTag = (pkg, tag) => tagsOf(pkg).includes(tag);

  /* ─── Android ↔ SDK mapping (from old.js) ─── */
  const androidToSdk = {
    '10': 29, '10.0': 29, '11': 30, '11.0': 30, '12': 31, '12.0': 31,
    '12L': 32, '12.1': 32, '13': 33, '13.0': 33, '14': 34, '14.0': 34,
    '15': 35, '15.0': 35, '16': 36, '16.0': 36, '17': 37, '17.0': 37,
  };
  const sdkToAndroid = (() => {
    const m = {};
    for (const [a, s] of Object.entries(androidToSdk)) m[s] = a.split('.')[0];
    return m;
  })();
  function sdkFromAndroid(v) {
    const c = String(v).replace(/[^0-9.L]/g, '');
    if (androidToSdk[c] != null) return androidToSdk[c];
    const main = c.split('.')[0];
    if (androidToSdk[main] != null) return androidToSdk[main];
    if (c.endsWith('L')) { const w2 = c.slice(0, -1); if (androidToSdk[w2] != null) return androidToSdk[w2]; }
    return null;
  }
  function androidFromSdk(v) {
    const c = String(v).replace(/[^0-9]/g, '');
    if (sdkToAndroid[c] != null) return sdkToAndroid[c];
    for (const [a, s] of Object.entries(androidToSdk)) if (String(s) === c) return a.split('.')[0];
    return null;
  }

  /* ─── Internal state ─── */
  let config   = {};   // parsed COPG.json
  let keyOrder = [];   // key insertion order
  let names    = {};   // list.json: cleanPkg -> display name
  let previewUnsaved = false;   // browser-preview edits not persisted

  /* ─── Device key derivation ─── */
  const deviceKeyFromName = name => `PACKAGES_${name.trim().toUpperCase().replace(/ /g, '_')}_DEVICE`;
  const pkgKeyOf = deviceKey => deviceKey.replace(/_DEVICE$/, '');

  /* ─── Load ─── */
  async function readFilePreview(rel) {
    // Browser preview: fetch the JSON shipped alongside the WebUI.
    try {
      const res = await fetch(rel, { cache: 'no-store' });
      if (res.ok) return await res.text();
    } catch (_) { /* fall through */ }
    // file:// fallback via XHR
    try {
      return await new Promise((resolve, reject) => {
        const x = new XMLHttpRequest();
        x.open('GET', rel, true);
        x.onreadystatechange = () => {
          if (x.readyState === 4) {
            if (x.status === 200 || x.status === 0) resolve(x.responseText || '');
            else reject(`HTTP ${x.status}`);
          }
        };
        x.send();
      });
    } catch (_) { return ''; }
  }

  async function loadAll() {
    let cfgText = '', listText = '';
    if (hasBridge()) {
      try { cfgText  = await execCommand(`cat ${CONFIG_PATH}`); } catch (_) {}
      try { listText = await execCommand(`cat ${LIST_PATH}`);   } catch (_) {}
    } else {
      // webroot/ is module/webroot/; the JSON lives one level up at module/COPG.json
      cfgText  = await readFilePreview('../COPG.json');
      listText = await readFilePreview('../list.json');
    }
    try { config = cfgText ? JSON.parse(cfgText) : {}; }
    catch (_) { config = {}; }
    try { names = listText ? JSON.parse(listText) : {}; }
    catch (_) { names = {}; }
    keyOrder = Object.keys(config);
    return { ok: true };
  }

  /* ─── Selectors ─── */
  const nameFor = c => names[c] || c;

  function listDevices() {
    const out = [];
    for (const key of keyOrder) {
      if (key.endsWith('_DEVICE') && config[key]) {
        const props   = config[key];
        const pkgKey  = pkgKeyOf(key);
        const games   = Array.isArray(config[pkgKey]) ? config[pkgKey] : [];
        out.push({
          key, pkgKey,
          name:  props.DEVICE || key.replace('PACKAGES_', '').replace('_DEVICE', ''),
          model: props.MODEL || 'Unknown',
          brand: props.BRAND || '',
          props,
          gameCount: games.length,
        });
      }
    }
    return out;
  }

  /* Flat package list across devices + cpu_spoof, deduped by clean name.
     Order mirrors old.js render: blocklist, then cpu_only, then device lists. */
  function listPackages() {
    const out = [];
    const seen = new Set();
    const push = (raw, type, deviceKey, deviceName) => {
      const c = clean(raw);
      if (seen.has(c)) return;
      seen.add(c);
      out.push({
        clean: c, raw, type, deviceKey: deviceKey || null,
        deviceName: deviceName || null,
        tags: tagsOf(raw),
        with_cpu: hasTag(raw, 'with_cpu'),
        got: hasTag(raw, 'got'),
        dnd: hasTag(raw, 'dnd'),
        dab: hasTag(raw, 'dab'),
        kso: hasTag(raw, 'kso'),
        nolog: hasTag(raw, 'nolog'),
        name: nameFor(c),
      });
    };
    const cpu = config.cpu_spoof || {};
    (cpu.blacklist || []).forEach(p => push(p, 'blocked'));
    (cpu.cpu_only_packages || []).forEach(p => push(p, 'cpu_only'));
    for (const key of keyOrder) {
      if (key.startsWith('PACKAGES_') && !key.endsWith('_DEVICE') && Array.isArray(config[key])) {
        const dk = `${key}_DEVICE`;
        const dn = config[dk]?.DEVICE || key.replace('PACKAGES_', '');
        config[key].forEach(p => push(p, 'device', key, dn));
      }
    }
    return out;
  }

  /* Find which list a clean package currently lives in (for duplicate detection). */
  function locate(cleanName, ignoreClean) {
    if (ignoreClean === cleanName) return '';
    const cpu = config.cpu_spoof || {};
    if ((cpu.blacklist || []).some(p => clean(p) === cleanName)) return 'blocklist';
    if ((cpu.cpu_only_packages || []).some(p => clean(p) === cleanName)) return 'CPU only list';
    for (const key of keyOrder) {
      if (key.startsWith('PACKAGES_') && !key.endsWith('_DEVICE') && Array.isArray(config[key])) {
        if (config[key].some(p => clean(p) === cleanName))
          return `device "${config[`${key}_DEVICE`]?.DEVICE || key}"`;
      }
    }
    return '';
  }

  function deviceNameExists(name, model, ignoreKey) {
    for (const [key, val] of Object.entries(config)) {
      if (key.endsWith('_DEVICE') && key !== ignoreKey) {
        if (val.DEVICE === name)  return { field: 'name' };
        if (model && val.MODEL === model) return { field: 'model' };
      }
    }
    return null;
  }

  /* ─── Device mutations (ported from old.js saveDevice / deleteDevice) ─── */
  function ensureCpuSpoof() {
    if (!config.cpu_spoof) {
      config.cpu_spoof = { blacklist: [], cpu_only_packages: [] };
      if (!keyOrder.includes('cpu_spoof')) keyOrder.unshift('cpu_spoof');
    }
    if (!Array.isArray(config.cpu_spoof.blacklist)) config.cpu_spoof.blacklist = [];
    if (!Array.isArray(config.cpu_spoof.cpu_only_packages)) config.cpu_spoof.cpu_only_packages = [];
  }

  /* form: {name, brand, model, manufacturer, fingerprint, android, sdk}
     editingKey: existing _DEVICE key when editing, else null. */
  function upsertDevice(form, editingKey) {
    const name  = form.name.trim();
    const model = (form.model || '').trim() || 'Unknown';
    const brand = (form.brand || '').trim() || 'Unknown';
    const deviceKey = deviceKeyFromName(name);
    const packageKey = pkgKeyOf(deviceKey);

    const data = {
      BRAND: brand,
      DEVICE: name,
      MANUFACTURER: (form.manufacturer || '').trim() || 'Unknown',
      MODEL: model,
      FINGERPRINT: (form.fingerprint || '').trim()
        || `${brand}/${model}/${model}:14/UP1A.231005.007/20230101:user/release-keys`,
      PRODUCT: model,
    };
    if ((form.serial || '').trim())  data.SERIAL = form.serial.trim();
    if ((form.android || '').trim()) data.ANDROID_VERSION = form.android.trim();
    if ((form.sdk || '').trim())     data.SDK_INT = form.sdk.trim();

    if (editingKey && editingKey !== deviceKey) {
      // renamed: migrate key positions + package array
      const oldPkgKey = pkgKeyOf(editingKey);
      const di = keyOrder.indexOf(editingKey);
      const pi = keyOrder.indexOf(oldPkgKey);
      if (di !== -1) keyOrder[di] = deviceKey;
      if (pi !== -1) keyOrder[pi] = packageKey;
      if (config[oldPkgKey]) { config[packageKey] = config[oldPkgKey]; delete config[oldPkgKey]; }
      delete config[editingKey];
    } else if (!editingKey) {
      keyOrder.push(packageKey, deviceKey);
    }
    if (!Array.isArray(config[packageKey])) config[packageKey] = [];
    config[deviceKey] = data;
    return { deviceKey, packageKey };
  }

  function deleteDevice(deviceKey) {
    const packageKey = pkgKeyOf(deviceKey);
    const removed = Array.isArray(config[packageKey]) ? [...config[packageKey]] : [];
    // drop the device's games from list.json
    removed.forEach(p => { const c = clean(p); if (names[c]) delete names[c]; });
    delete config[packageKey];
    delete config[deviceKey];
    keyOrder = keyOrder.filter(k => k !== packageKey && k !== deviceKey);
    return { removedCount: removed.length };
  }

  /* ─── Package mutations (ported from old.js saveGame / deleteGame) ─── */
  /* form: { pkg (may include tags as typed), name, type:'device'|'cpu_only'|'blocked',
            deviceKey (PACKAGES_…_DEVICE when type=device), with_cpu, got,
            dnd, dab, kso, nolog }
     editing: { clean, type, deviceKey } | null  */
  function upsertPackage(form, editing) {
    ensureCpuSpoof();
    const cpu = config.cpu_spoof;
    const cleanName = clean(form.pkg.trim());
    const displayName = (form.name || '').trim() || cleanName;
    const type = form.type;
    const newDeviceKey = form.deviceKey || null;       // _DEVICE key
    const newPkgKey = newDeviceKey ? pkgKeyOf(newDeviceKey) : null;

    // list.json: rename-aware update
    if (editing && editing.clean && editing.clean !== cleanName) delete names[editing.clean];
    names[cleanName] = displayName;

    // Build the tagged package string. Spoof tags (with_cpu/blocked/got) are device-only;
    // tweak tags (dnd/dab/kso/nolog) apply to device AND cpu_only (controller comfort toggles).
    let finalPkg = cleanName;
    if (type === 'device') {
      if (form.with_cpu)  finalPkg = addTag(finalPkg, 'with_cpu');
      if (form.got)       finalPkg = addTag(finalPkg, 'got');
    }
    if (type === 'device' || type === 'cpu_only') {
      if (form.dnd)       finalPkg = addTag(finalPkg, 'dnd');
      if (form.dab)       finalPkg = addTag(finalPkg, 'dab');
      if (form.kso)       finalPkg = addTag(finalPkg, 'kso');
      if (form.nolog)     finalPkg = addTag(finalPkg, 'nolog');
    }

    const oldClean   = editing ? editing.clean : null;
    const oldType    = editing ? editing.type : null;
    const oldDevKey  = editing ? editing.deviceKey : null;

    // Remember original position to preserve order on in-place edit
    let origPos = -1, origList = null;
    if (editing) {
      if (oldType === 'blocked')  { origPos = cpu.blacklist.findIndex(p => clean(p) === oldClean); origList = 'blocked'; }
      else if (oldType === 'cpu_only') { origPos = cpu.cpu_only_packages.findIndex(p => clean(p) === oldClean); origList = 'cpu_only'; }
      else if (oldType === 'device' && oldDevKey && Array.isArray(config[oldDevKey])) {
        origPos = config[oldDevKey].findIndex(p => clean(p) === oldClean); origList = oldDevKey;
      }
    }

    // Remove from previous home if type/device changed
    if (oldClean) {
      const bi = cpu.blacklist.findIndex(p => clean(p) === oldClean);
      const ci = cpu.cpu_only_packages.findIndex(p => clean(p) === oldClean);
      if (bi !== -1 && type !== 'blocked')  cpu.blacklist.splice(bi, 1);
      if (ci !== -1 && type !== 'cpu_only') cpu.cpu_only_packages.splice(ci, 1);
      if (oldDevKey && (type !== 'device' || oldDevKey !== newPkgKey) && Array.isArray(config[oldDevKey])) {
        const oi = config[oldDevKey].findIndex(p => clean(p) === oldClean);
        if (oi !== -1) config[oldDevKey].splice(oi, 1);
      }
    }

    // Insert into new home
    if (type === 'device') {
      if (!Array.isArray(config[newPkgKey])) {
        config[newPkgKey] = [];
        if (!keyOrder.includes(newPkgKey)) {
          const di = keyOrder.indexOf(newDeviceKey);
          if (di !== -1) keyOrder.splice(di, 0, newPkgKey); else keyOrder.push(newPkgKey);
        }
      }
      const arr = config[newPkgKey];
      const ex = arr.findIndex(p => clean(p) === clean(finalPkg));
      if (ex !== -1) arr.splice(ex, 1);
      if (origList === newPkgKey && origPos !== -1) arr.splice(origPos, 0, finalPkg);
      else arr.push(finalPkg);
    } else if (type === 'cpu_only') {
      const arr = cpu.cpu_only_packages;
      const ex = arr.findIndex(p => clean(p) === cleanName);
      if (ex !== -1) arr.splice(ex, 1);
      if (origList === 'cpu_only' && origPos !== -1) arr.splice(origPos, 0, finalPkg);
      else arr.push(finalPkg);
    } else if (type === 'blocked') {
      const arr = cpu.blacklist;
      const ex = arr.findIndex(p => clean(p) === cleanName);
      if (ex !== -1) arr.splice(ex, 1);
      if (origList === 'blocked' && origPos !== -1) arr.splice(origPos, 0, cleanName);
      else arr.push(cleanName);
    }
    return { clean: cleanName };
  }

  function deletePackage(cleanName, type, deviceKey) {
    ensureCpuSpoof();
    const cpu = config.cpu_spoof;
    if (type === 'blocked') {
      const i = cpu.blacklist.findIndex(p => clean(p) === cleanName);
      if (i !== -1) cpu.blacklist.splice(i, 1);
    } else if (type === 'cpu_only') {
      const i = cpu.cpu_only_packages.findIndex(p => clean(p) === cleanName);
      if (i !== -1) cpu.cpu_only_packages.splice(i, 1);
    } else if (type === 'device' && deviceKey && Array.isArray(config[deviceKey])) {
      const i = config[deviceKey].findIndex(p => clean(p) === cleanName);
      if (i !== -1) config[deviceKey].splice(i, 1);
    }
    if (names[cleanName]) delete names[cleanName];
  }

  /* ─── Serialize + persist ─── */
  function serializeConfig() {
    const ordered = {};
    for (const k of keyOrder) if (config[k] !== undefined) ordered[k] = config[k];
    for (const k of Object.keys(config)) if (!keyOrder.includes(k)) { keyOrder.push(k); ordered[k] = config[k]; }
    return JSON.stringify(ordered, null, 2);
  }

  async function save() {
    const cfgStr  = serializeConfig();
    const listStr = JSON.stringify(names, null, 2);
    if (!hasBridge()) { previewUnsaved = true; return { saved: false, preview: true }; }
    await execCommand(`echo '${shq(cfgStr)}' > ${CONFIG_PATH}`);
    await execCommand(`echo '${shq(listStr)}' > ${LIST_PATH}`);
    try {
      await execCommand(`chmod 644 ${CONFIG_PATH} ${LIST_PATH}`);
      await execCommand(`chcon u:object_r:system_file:s0 ${CONFIG_PATH}`);
      await execCommand(`chcon u:object_r:system_file:s0 ${LIST_PATH}`);
    } catch (e) { console.warn('post-write chmod/chcon:', e); }
    return { saved: true };
  }

  function exportText() { return { config: serializeConfig(), list: JSON.stringify(names, null, 2) }; }

  /* ─── Backup / Restore ─────────────────────────────────────────────────
     Export the live config to BACKUP_DIR as a timestamped pair
     (COPG-<ts>.json + list-<ts>.json); import reads a chosen backup back in,
     validates the JSON, then re-saves through save() so the same chmod/chcon
     path runs and the in-memory model stays the single source of truth.
     In browser preview there's no filesystem: backupConfig() returns the data
     so the UI can offer a blob download, listBackups() is empty, and restore
     is driven by a file <input> via restoreFromText(). Designed so new export
     targets (e.g. extra files) only need a new entry in currentSnapshot(). ─── */

  function stamp() {
    const d = new Date(), p = n => String(n).padStart(2, '0');
    return `${d.getFullYear()}${p(d.getMonth() + 1)}${p(d.getDate())}` +
           `-${p(d.getHours())}${p(d.getMinutes())}${p(d.getSeconds())}`;
  }

  function isPlainObject(o) { return !!o && typeof o === 'object' && !Array.isArray(o); }

  // Decide whether a parsed JSON object is a COPG.json (config) or a list.json
  // (display names) WITHOUT trusting the filename. Signals, strongest first:
  //   • any PACKAGES_* key or a cpu_spoof key → config (unambiguous shape)
  //   • every value is a string               → list   (flat pkg→name map)
  //   • any non-string (array/object) value   → config (config-shaped)
  //   • empty {}                              → list, but ambiguous (let the
  //     user confirm — an empty file could legitimately be either one).
  function detectKind(obj) {
    if (!isPlainObject(obj)) return { kind: 'config', ambiguous: false };
    const keys = Object.keys(obj);
    if (keys.some(k => /^PACKAGES_/i.test(k) || k === 'cpu_spoof'))
      return { kind: 'config', ambiguous: false };
    if (keys.length === 0) return { kind: 'list', ambiguous: true };
    if (Object.values(obj).every(v => typeof v === 'string'))
      return { kind: 'list', ambiguous: false };
    return { kind: 'config', ambiguous: false };
  }

  // Parse + classify a backup file's text. Throws on bad JSON / non-object so
  // the caller can surface an "invalid file" message. The import-from-file flow
  // uses this to show the detected type AND offer the user an override.
  function inspectBackup(text) {
    const obj = JSON.parse(text);                  // throws on malformed JSON
    if (!isPlainObject(obj)) throw new Error('Not a JSON object');
    const d = detectKind(obj);
    return { obj, kind: d.kind, ambiguous: d.ambiguous };
  }

  function applyConfigObject(obj) {
    config = isPlainObject(obj) ? obj : {};
    keyOrder = Object.keys(config);
  }
  function applyNamesObject(obj) { names = isPlainObject(obj) ? obj : {}; }

  // Snapshot of what export would write — the one place that lists backup files.
  function currentSnapshot() {
    const ts = stamp();
    return {
      ts,
      files: [
        { name: `COPG-${ts}.json`, kind: 'config', data: serializeConfig() },
        { name: `list-${ts}.json`, kind: 'list',   data: JSON.stringify(names, null, 2) },
      ],
    };
  }

  async function backupConfig() {
    const snap = currentSnapshot();
    if (!hasBridge()) return { preview: true, ts: snap.ts, files: snap.files };  // UI → blob download
    await execCommand(`mkdir -p ${BACKUP_DIR}`);
    const written = [];
    for (const f of snap.files) {
      const path = `${BACKUP_DIR}/${f.name}`;
      await execCommand(`echo '${shq(f.data)}' > ${path}`);
      written.push({ name: f.name, path, kind: f.kind });
    }
    return { preview: false, ts: snap.ts, dir: BACKUP_DIR, files: written };
  }

  // Device-only: list *.json in BACKUP_DIR, grouped into restorable sets.
  // Timestamped COPG-/list- files pair up by their <ts>; any other *.json file
  // is its own single-file entry (kind auto-detected at restore time).
  async function listBackups() {
    if (!hasBridge()) return [];
    let out = '';
    try { out = await execCommand(`ls -1 ${BACKUP_DIR}`); } catch (_) { return []; }
    const files = String(out).split('\n').map(s => s.trim()).filter(s => /\.json$/i.test(s));
    const sets = new Map();
    files.forEach(name => {
      const m = name.match(/^(COPG|list)-(\d{8}-\d{6})\.json$/i);
      const key = m ? m[2] : name;
      if (!sets.has(key)) sets.set(key, { ts: m ? m[2] : null, configFile: null, listFile: null, file: null });
      const set = sets.get(key);
      if (m && /^COPG$/i.test(m[1]))      set.configFile = name;
      else if (m && /^list$/i.test(m[1])) set.listFile = name;
      else                                set.file = name;
    });
    return [...sets.values()].sort((a, b) => {
      if (a.ts && b.ts) return b.ts.localeCompare(a.ts);   // newest timestamp first
      if (a.ts) return -1;
      if (b.ts) return 1;
      return (b.file || '').localeCompare(a.file || '');
    });
  }

  async function readApplyFile(name, forcedKind) {
    const text = await execCommand(`cat ${BACKUP_DIR}/${name}`);
    const obj  = JSON.parse(text);                       // throws on malformed → caller catches
    const kind = forcedKind || detectKind(obj).kind;
    if (kind === 'config') applyConfigObject(obj); else applyNamesObject(obj);
    return kind;
  }

  // Restore an entry from listBackups(); re-saves through save().
  async function restoreBackup(entry) {
    if (!hasBridge()) return { saved: false, preview: true, restored: [] };
    const restored = [];
    if (entry.configFile || entry.listFile) {
      if (entry.configFile) restored.push(await readApplyFile(entry.configFile, 'config'));
      if (entry.listFile)   restored.push(await readApplyFile(entry.listFile, 'list'));
    } else if (entry.file) {
      restored.push(await readApplyFile(entry.file, null));
    }
    const res = await save();
    return { ...res, restored };
  }

  // Restore from raw text (file <input> / pasted JSON). Auto-detects the kind
  // unless the caller forces it ('config' | 'list') — the import UI passes the
  // user's chosen type so a wrong guess can never silently overwrite the other
  // file.
  async function restoreFromText(text, forcedKind) {
    const obj = JSON.parse(text);                        // throws on malformed
    if (!isPlainObject(obj)) throw new Error('Not a JSON object');
    const kind = forcedKind || detectKind(obj).kind;
    if (kind === 'config') applyConfigObject(obj); else applyNamesObject(obj);
    const res = await save();
    return { ...res, restored: [kind] };
  }

  // Download one URL to stdout and parse it as JSON. Tries curl, then wget,
  // then busybox wget — each a SINGLE simple command (no pipes/compound), per
  // the bridge constraint. A 404/HTML body fails JSON.parse → next downloader.
  async function fetchRemoteJson(url) {
    const cmds = [
      `curl -fsSL ${url}`,        // -f: fail on HTTP error, -L: follow redirects
      `wget -qO- ${url}`,
      `busybox wget -qO- ${url}`,
    ];
    let lastErr = null;
    for (const cmd of cmds) {
      try {
        const out = await execCommand(cmd);
        const obj = JSON.parse(String(out).trim());   // throws on empty/non-JSON
        if (!isPlainObject(obj)) throw new Error('remote payload is not a JSON object');
        return obj;
      } catch (e) { lastErr = e; }                     // fall through to next downloader
    }
    throw lastErr || new Error('no downloader available');
  }

  // Save the in-app console buffer to a timestamped .txt under LOG_DIR
  // (mirrors old.js's saveLogToFile, minus its existence-probing — the seconds
  // in stamp() make collisions effectively impossible, so no compound `ls`).
  // Browser preview has no FS → return the text for a blob download.
  async function saveLog(text) {
    const name = `COPG-LOG-${stamp()}.txt`;
    if (!hasBridge()) return { preview: true, name, data: text };
    await execCommand(`mkdir -p ${LOG_DIR}`);
    const path = `${LOG_DIR}/${name}`;
    await execCommand(`echo '${shq(text)}' > ${path}`);
    try { await execCommand(`chmod 644 ${path}`); } catch (e) { console.warn('log chmod:', e); }
    return { preview: false, name, path };
  }

  // "Sync from GitHub": pull the latest COPG.json + list.json from the repo and
  // adopt them. Both are fetched BEFORE anything is applied so a half-failed
  // sync can't leave a mismatched config/list pair; then one save() persists.
  async function syncFromGitHub() {
    if (!hasBridge()) return { saved: false, preview: true, synced: [] };
    const cfg = await fetchRemoteJson(SYNC_CONFIG_URL);   // throws → caller shows fail toast
    const lst = await fetchRemoteJson(SYNC_LIST_URL);
    applyConfigObject(cfg);
    applyNamesObject(lst);
    const res = await save();
    return { ...res, synced: ['config', 'list'] };
  }

  /* ─── System info (real device data) ───────────────────────────────────
     Built from SIMPLE single-purpose bridge calls only — the same kind of
     plain command (`cat`, `getprop`, `uname`) that loadAll() proves works on
     this device. NO compound `;`/`$(...)`/`exit 0` scripts: some ksu.exec
     bridges choke on those (that's why the System tab stayed on "—" while the
     plain `cat` config load succeeded). Each value is read independently, so
     one failing call never blanks the rest.
       • getprop                  → Android/SDK + ABI list (ro.product.cpu.abilist)
       • uname -r                 → kernel
       • cat module.prop          → COPG version/code/author/description
       • cat <zygisk>/module.prop → Zygisk variant name (+ disable file)
       • ROOT BINARIES ARE NOT ON $PATH under ksu.exec — call them by absolute
         path: /data/adb/ksud, /data/adb/apd, /data/adb/magisk/magisk.        */
  let sysCache = null;

  // Absolute paths — root binaries are NOT on $PATH inside the ksu.exec shell.
  // `args` is a list: the binary is run once per arg and the outputs are joined
  // before parsing, because the version NAME and version CODE often come from
  // DIFFERENT subcommands:
  //   • KernelSU: `ksud -V` → name (e.g. 3.2.4); `ksud debug version` → code (32473)
  //   • Magisk:   `magisk -c` → name (e.g. 27.0); `magisk -V` → code (27000)
  const ROOT_BINS = {
    APatch:   { bin: '/data/adb/apd',            args: ['-V'] },
    KernelSU: { bin: '/data/adb/ksud',           args: ['-V', 'debug version'] },
    Magisk:   { bin: '/data/adb/magisk/magisk',  args: ['-c', '-V'] },
  };

  function cleanVer(s) { return (s || '').replace(/^(ksud?|kernelsu|apd?|magisk|version)[:\s]*/i, '').trim(); }

  /* Normalise a root-tool version string to "x.y.z(code)" when both are present.
     Input is the joined output of the tool's version subcommands (see ROOT_BINS),
     so it may contain the name and code in any order/separator; we extract the
     semver and the first standalone 4+ digit code and surface whatever we read. */
  function normRootVer(raw) {
    const s = (raw || '').replace(/\s+/g, ' ').trim();
    if (!s) return '';
    // 1) the semantic version, e.g. 3.2.4 (or v1.0.5)
    const sm = s.match(/v?(\d+\.\d+(?:\.\d+)?)/);
    const ver = sm ? sm[1] : '';
    // 2) the version CODE = any standalone run of 4+ digits that ISN'T part of
    //    the semver. Remove the semver first, then grab the first long number —
    //    this catches every separator (paren/colon/dot/dash/space): 3.2.4(32473),
    //    32473:3.2.4, v3.2.4-32473, 3.2.4.32473 …
    const rest = sm ? s.replace(sm[0], ' ') : s;
    const cm = rest.match(/(\d{4,})/);
    const code = cm ? cm[1] : '';
    if (ver && code) return `${ver}(${code})`;
    if (ver)  return ver;
    if (code) return code;
    return cleanVer(s);
  }

  // one prop out of a `getprop` dump: lines look like `[key]: [value]`
  function propOf(dump, key) {
    const m = (dump || '').match(new RegExp('\\[' + key.replace(/\./g, '\\.') + '\\]:\\s*\\[([^\\]]*)\\]'));
    return m ? m[1].trim() : '';
  }
  // one field out of a module.prop / build.prop style `key=value` dump
  function fieldOf(dump, key) {
    const m = (dump || '').match(new RegExp('^' + key + '=(.*)$', 'm'));
    return m ? m[1].trim() : '';
  }

  // best-effort single command → text ('' on any failure), never throws.
  // A non-zero exit here is EXPECTED, not an error: `code 127` just means that
  // root binary isn't installed (e.g. apd/magisk on a KSU device), `code 1`
  // means an optional file is absent (a `disable` flag, or a not-installed
  // zygisk variant). So these are logged at info level (gated behind Debug
  // Logs), never as warnings/errors — they're normal probing, not failures.
  async function tryCmd(cmd) {
    try { return (await execCommand(cmd)) || ''; }
    catch (e) { try { console.log(`sysinfo: "${cmd}" → ${e}`); } catch (_) {} return ''; }
  }

  /* Detect a root solution by its absolute binary; returns its version
     (name+code, normalised) or null when the binary isn't present. Runs each
     configured subcommand and joins the outputs so normRootVer sees both the
     version name and the version code. A missing binary yields no output. */
  async function detectRoot(name) {
    const { bin, args } = ROOT_BINS[name];
    let combined = '';
    for (const a of args) {
      const out = (await tryCmd(`${bin} ${a} 2>/dev/null`)).trim();
      if (out) combined += ' ' + out;
    }
    combined = combined.trim();
    if (!combined) return null;               // binary absent / no output
    return { name, version: normRootVer(combined) };
  }

  async function readZygisk(dir, label) {
    const prop = await tryCmd(`cat ${dir}/module.prop 2>/dev/null`);
    if (!prop.trim()) return null;                       // dir/module.prop absent
    const off = (await tryCmd(`ls ${dir}/disable 2>/dev/null`)).trim() !== '';
    return { variant: label, version: cleanVer(fieldOf(prop, 'version')), on: !off };
  }

  async function getSystemInfo() {
    if (sysCache) return sysCache;
    if (!hasBridge()) return null;                       // preview: keep placeholders

    // Fire the independent reads together; each is a plain, single command.
    const [props, kernel, modProp, apd, ksud, magisk, zNext, reZy] = await Promise.all([
      tryCmd('getprop'),
      tryCmd('uname -r'),
      tryCmd('cat /data/adb/modules/COPG/module.prop 2>/dev/null'),
      detectRoot('APatch'),
      detectRoot('KernelSU'),
      detectRoot('Magisk'),
      readZygisk('/data/adb/modules/zygisksu', 'Zygisk Next'),
      readZygisk('/data/adb/modules/rezygisk', 'ReZygisk'),
    ]);

    const android = propOf(props, 'ro.build.version.release');
    const sdk     = propOf(props, 'ro.build.version.sdk');
    // ABI: prefer the full abilist (what the user asked for), first entry shown.
    const abilist = propOf(props, 'ro.product.cpu.abilist');
    const abi     = abilist || propOf(props, 'ro.product.cpu.abi');

    // Root: priority APatch → KernelSU → Magisk (mirrors customize.sh).
    const roots = [apd, ksud, magisk].filter(Boolean);
    const primary = roots[0] || null;
    const secondary = roots.filter(r => primary && r.name !== primary.name);

    // Zygisk: prefer an active (enabled) variant over a disabled one. Name only.
    const zygCandidates = [zNext, reZy].filter(Boolean);
    let zygisk = { variant: 'Not installed', version: '', on: false };
    if (zygCandidates.length) zygisk = zygCandidates.find(z => z.on) || zygCandidates[0];
    else if (primary && primary.name === 'Magisk') zygisk = { variant: 'Magisk Zygisk', version: '', on: true };

    const moduleOff = (await tryCmd('ls /data/adb/modules/COPG/disable 2>/dev/null')).trim() !== '';

    sysCache = {
      androidVer: android ? `${android}${sdk ? ` (API ${sdk})` : ''}` : '—',
      sdk,
      abi: abi || '—',
      fingerprint: propOf(props, 'ro.build.fingerprint'),
      kernel: kernel.trim() || '—',
      roots, primary, secondary,
      rootEnv: primary ? primary.name : 'Unknown',
      rootVersion: primary ? primary.version : '',
      // e.g. "KernelSU 3.2.4(32473)" — the root solution + its version
      rootVersionLabel: primary ? `${primary.name}${primary.version ? ' ' + primary.version : ''}` : '—',
      zygisk,
      // "Zygisk Next (v1.2.2)" — variant name + version when known
      zygiskLabel: zygisk.version
        ? `${zygisk.variant} (${/^v/i.test(zygisk.version) ? zygisk.version : 'v' + zygisk.version})`
        : zygisk.variant,
      version: cleanVer(fieldOf(modProp, 'version')),
      versionCode: fieldOf(modProp, 'versionCode'),
      author: fieldOf(modProp, 'author'),
      description: fieldOf(modProp, 'description'),
      moduleActive: !moduleOff,
    };
    return sysCache;
  }

  /* ─── Installed packages (Set of package names) ─────────────────────────
     The host's package API differs by environment, so every call below is
     async-tolerant (await Promise.resolve — MMRL WebUI-X interfaces are
     Promise-based) and we try, in order: the KernelSU `ksu` API, the WebUI-X
     `$packageManager`, then a `pm` shell fallback. A diagnostic line is logged
     so the in-app Console shows exactly which interface answered. */
  let installedCache = null;

  /* Module interface via the SANITIZED module id (MMRL WebUI-X).
     https://mmrl.dev/guide/webuix/sanitized-ids — the host injects the module's
     interface under `$<sanitized id>`, where every char outside [a-zA-Z0-9_] in
     the module id is replaced with `_`. For module "COPG" that is `$COPG`. */
  const MODULE_ID = 'COPG';
  const SANITIZED_ID = MODULE_ID.replace(/[^a-zA-Z0-9_]/g, '_');
  function moduleInterface() { return w['$' + SANITIZED_ID] || w.$COPG || w.$copg || null; }

  /* Android ApplicationInfo flags (https://mmrl.dev/guide/webuix/application-info-flags) */
  const APP_FLAG_SYSTEM = 0x00000001;
  const APP_FLAG_UPDATED_SYSTEM_APP = 0x00000080;
  function flagsAreSystem(flags) {
    const f = Number(flags) || 0;
    return (f & APP_FLAG_SYSTEM) !== 0 || (f & APP_FLAG_UPDATED_SYSTEM_APP) !== 0;
  }

  const dlog = (m, t) => { try { if (w.logEvent) w.logEvent(m, t || 'info'); } catch (_) {} };

  /* Normalise whatever a package source returns into a clean string[] of
     package names. Handles: a JS array (already parsed), a `{packages:[…]}`
     object, a JSON-string array (ksu.listPackages returns a JSON STRING — see
     kernelsu-alt: it does `JSON.parse(ksu.listPackages(type))`), and a newline
     list from `pm list packages` whose lines look like `package:com.foo`. */
  function normalizePkgList(raw) {
    let arr = [];
    if (Array.isArray(raw)) arr = raw;
    else if (raw && typeof raw === 'object' && Array.isArray(raw.packages)) arr = raw.packages;
    else if (typeof raw === 'string') {
      const s = raw.trim();
      if (s.startsWith('[')) { try { arr = JSON.parse(s); } catch (_) { arr = s.split('\n'); } }
      else arr = s.split('\n');               // `pm list packages` lines
    }
    return arr
      .map(p => (typeof p === 'string' ? p : (p && (p.packageName || p.package)) || ''))
      .map(s => s.replace(/^package:/, '').trim())   // strip `pm` prefix
      .filter(Boolean);
  }

  /* Diagnostic: which package interfaces are present? Logged once. */
  let apisLogged = false;
  function logApis() {
    if (apisLogged) return; apisLogged = true;
    dlog('APIs: ksu=' + (typeof ksu) +
         ' $packageManager=' + (typeof (w.$packageManager)) +
         ' $' + SANITIZED_ID + '=' + (typeof moduleInterface()) +
         ' bridge=' + hasBridge());
  }

  async function getInstalledPackages() {
    if (installedCache) return installedCache;
    logApis();
    let arr = [], via = '';
    // 1) KernelSU API. Per kernelsu-alt, ksu.listPackages(type) is SYNCHRONOUS
    //    and returns a JSON STRING (the lib does JSON.parse(ksu.listPackages(type))).
    //    We await Promise.resolve so a host that returns a Promise still works,
    //    and normalizePkgList parses the JSON-string / array either way.
    try {
      if (typeof ksu !== 'undefined' && typeof ksu.listPackages === 'function') {
        arr = normalizePkgList(await Promise.resolve(ksu.listPackages('all')));
        if (arr.length) via = 'ksu.listPackages';
      }
    } catch (e) { dlog('ksu.listPackages failed: ' + e, 'warn'); }
    // 2) WebUI-X $packageManager (Promise-based on MMRL → must await)
    if (!arr.length && typeof w.$packageManager !== 'undefined' && typeof w.$packageManager.getInstalledPackages === 'function') {
      try {
        arr = normalizePkgList(await Promise.resolve(w.$packageManager.getInstalledPackages(0, 0)));
        if (arr.length) via = '$packageManager.getInstalledPackages';
      } catch (e) { dlog('$packageManager.getInstalledPackages failed: ' + e, 'warn'); }
    }
    // 3) shell pm — NO pipe. Some ksu.exec bridges choke on `|`/`$(...)` (the same
    //    bug that blanked System info), so run plain `pm list packages` and strip
    //    the `package:` prefix in JS (normalizePkgList), like kernelsu-alt's spawn().
    if (!arr.length && hasBridge()) {
      try {
        arr = normalizePkgList(await execCommand('pm list packages'));
        if (arr.length) via = 'pm list packages';
      } catch (e) { dlog('pm list packages failed: ' + e, 'warn'); }
    }
    dlog('Installed packages: ' + arr.length + (via ? ' via ' + via : ' (none)'), arr.length ? 'success' : 'warn');
    installedCache = new Set(arr);
    return installedCache;
  }

  async function isInstalled(cleanName) { return (await getInstalledPackages()).has(cleanName); }

  /* Set of SYSTEM package names — fallback via `pm -s` when app flags aren't
     available from getPackagesInfo / getApplicationInfo. Cached. */
  let systemCache = null;
  async function getSystemPackages() {
    if (systemCache) return systemCache;
    let arr = [];
    if (hasBridge()) {
      // plain command (no pipe) — normalizePkgList strips the `package:` prefix
      try { arr = normalizePkgList(await execCommand('pm list packages -s')); } catch (_) {}
    }
    systemCache = new Set(arr);
    return systemCache;
  }

  /* Best-effort single app label (async-tolerant; KSU then $packageManager). */
  async function getAppLabel(pkg) {
    try {
      if (typeof ksu !== 'undefined' && typeof ksu.getPackageInfo === 'function') {
        const i = await Promise.resolve(ksu.getPackageInfo(pkg));
        if (i && (i.appLabel || i.label)) return i.appLabel || i.label;
      }
      if (typeof w.$packageManager !== 'undefined' && typeof w.$packageManager.getApplicationInfo === 'function') {
        const i = await Promise.resolve(w.$packageManager.getApplicationInfo(pkg, 0, 0));
        if (i) { if (typeof i.getLabel === 'function') return i.getLabel(); if (i.label || i.appLabel) return i.label || i.appLabel; }
      }
    } catch (_) { /* fall through */ }
    return null;
  }

  /* FAST installed-app list for the picker: [{pkg, label, system}] sorted A→Z.
     Only package names are awaited here (cheap), with list.json / package-name
     labels, so the picker shows immediately. Real labels + system flags are
     filled afterwards via enrichApps() (background). */
  async function listInstalledApps() {
    const set = await getInstalledPackages();
    return [...set]
      .map(pkg => ({ pkg, label: names[clean(pkg)] || pkg, system: false }))
      .sort((a, b) => a.label.toLowerCase().localeCompare(b.label.toLowerCase()));
  }

  /* Background enrichment for the picker: real app label + isSystem flag.
     Tries, in order: ksu.getPackagesInfo (batch), $packageManager.getApplicationInfo
     (per-app, reads ApplicationInfo.flags), pm -s for the system set. Returns
     { labels:{pkg->label}, system:Set }. Runs AFTER the list is on screen. */
  async function enrichApps(pkgs) {
    const labels = {};
    const system = new Set();
    let sawSystemFlag = false;

    // (a) KSU batch. Per kernelsu-alt the contract is
    //     ksu.getPackagesInfo(JSON.stringify(pkgs)) → JSON STRING of
    //     [{packageName, appLabel, isSystem, versionName, versionCode, uid}].
    try {
      if (typeof ksu !== 'undefined' && typeof ksu.getPackagesInfo === 'function') {
        let raw = await Promise.resolve(ksu.getPackagesInfo(JSON.stringify(pkgs)));
        if (typeof raw === 'string') { try { raw = JSON.parse(raw); } catch (_) { raw = []; } }
        if (Array.isArray(raw)) raw.forEach((info, i) => {
          if (!info) return;
          const pkg = info.packageName || info.package || pkgs[i];
          const label = info.appLabel || info.label;
          if (pkg && label) labels[pkg] = label;
          let sys = null;
          if (typeof info.isSystem === 'boolean') sys = info.isSystem;
          else if (info.applicationInfo && info.applicationInfo.flags != null) sys = flagsAreSystem(info.applicationInfo.flags);
          else if (info.flags != null) sys = flagsAreSystem(info.flags);
          if (sys !== null) { sawSystemFlag = true; if (sys && pkg) system.add(pkg); }
        });
        dlog('enrich: ksu.getPackagesInfo → ' + Object.keys(labels).length + ' labels', 'info');
      }
    } catch (e) { dlog('enrich ksu.getPackagesInfo failed: ' + e, 'warn'); }

    // (b) WebUI-X per-app fallback for any still-missing labels (bounded)
    if (Object.keys(labels).length === 0 && typeof w.$packageManager !== 'undefined' && typeof w.$packageManager.getApplicationInfo === 'function') {
      let n = 0;
      for (const pkg of pkgs) {
        try {
          const i = await Promise.resolve(w.$packageManager.getApplicationInfo(pkg, 0, 0));
          if (i) {
            const label = (typeof i.getLabel === 'function') ? i.getLabel() : (i.label || i.appLabel);
            if (label) labels[pkg] = label;
            if (i.flags != null) { sawSystemFlag = true; if (flagsAreSystem(i.flags)) system.add(pkg); }
            n++;
          }
        } catch (_) { /* skip this app */ }
      }
      dlog('enrich: $packageManager.getApplicationInfo → ' + n + ' apps', 'info');
    }

    // (c) system-set fallback
    if (!sawSystemFlag) {
      try { (await getSystemPackages()).forEach(p => system.add(p)); } catch (_) {}
    }
    return { labels, system };
  }

  /* Best-effort Play Store icon (bridge only). Returns data URI or null. */
  async function fetchPlayIcon(pkg) {
    if (!hasBridge()) return null;
    const p = shq(pkg);
    try {
      const found = await execCommand(
        `curl -sL --max-time 8 'https://play.google.com/store/apps/details?id=${p}&hl=en' 2>/dev/null | grep -o 'https://play-lh.googleusercontent.com/[^"\\\\=]*' | head -1`
      );
      const url = (found || '').trim().split('\n')[0];
      if (!url || !url.startsWith('https://play-lh')) return null;
      return await urlToDataUri(`${url}=s128`);
    } catch (_) { return null; }
  }

  /* F-Droid icon fallback (open repo, no Play presence needed). */
  async function fetchFdroidIcon(pkg) {
    if (!hasBridge()) return null;
    const p = shq(pkg);
    try {
      // F-Droid serves a per-package icon under the repo; resolve via the page.
      const found = await execCommand(
        `curl -sL --max-time 8 'https://f-droid.org/en/packages/${p}/' 2>/dev/null | grep -o 'https://f-droid.org/repo/[^"]*\\.\\(png\\|webp\\)' | head -1`
      );
      const url = (found || '').trim().split('\n')[0];
      if (!url || !url.startsWith('https://f-droid.org/repo/')) return null;
      return await urlToDataUri(url);
    } catch (_) { return null; }
  }

  /* curl a URL → base64 data URI (png). Returns null on any failure. */
  async function urlToDataUri(url) {
    try {
      const b64 = await execCommand(`curl -sL --max-time 8 '${shq(url)}' 2>/dev/null | base64 2>/dev/null | tr -d '\\n'`);
      const data = (b64 || '').replace(/\s+/g, '');
      if (data.length < 100) return null;
      return 'data:image/png;base64,' + data;
    } catch (_) { return null; }
  }

  /* Unified best-effort icon: Play first, then F-Droid. */
  async function fetchAppIcon(pkg) {
    return (await fetchPlayIcon(pkg)) || (await fetchFdroidIcon(pkg)) || null;
  }

  /* ─── Public API ─── */
  /* Immersive fullscreen toggle — KernelSU / ReZygisk WebUI bridge (`ksu.fullScreen`).
     true = fullscreen (status bar hidden), false = status bar shown. No-op when the
     bridge is absent (browser preview) or the host doesn't expose `fullScreen`. The
     page already pads `env(safe-area-inset-top)` (base.css) so content stays clear of
     the notch/clock while immersive. */
  function setFullscreen(on) {
    try {
      if (typeof ksu !== 'undefined' && typeof ksu.fullScreen === 'function') ksu.fullScreen(!!on);
    } catch (e) { dlog('fullScreen bridge failed: ' + e, 'warn'); }
  }

  w.COPG = {
    MODULE_DIR, CONFIG_PATH, LIST_PATH, BACKUP_DIR,
    execCommand, hasBridge,
    loadAll,
    // selectors
    listDevices, listPackages, nameFor, locate, deviceNameExists,
    // mutations
    upsertDevice, deleteDevice, upsertPackage, deletePackage,
    // persistence
    save, exportText,
    // backup / restore
    backupConfig, listBackups, restoreBackup, restoreFromText, inspectBackup, syncFromGitHub, saveLog,
    // system / device-environment info
    getSystemInfo, setFullscreen,
    // installed apps + icons (KSU API with pm fallback)
    getInstalledPackages, getSystemPackages, isInstalled, getAppLabel, listInstalledApps, enrichApps, fetchPlayIcon, fetchAppIcon,
    // helpers exposed for modals/UI
    clean, tagsOf, hasTag, deviceKeyFromName, pkgKeyOf,
    sdkFromAndroid, androidFromSdk,
    // flags / raw access
    get config()    { return config; },
    get keyOrder()  { return keyOrder; },
    get names()     { return names; },
    get previewUnsaved() { return previewUnsaved; },
    set previewUnsaved(v) { previewUnsaved = v; },
    get installedSet()   { return installedCache || new Set(); },
  };

})(window);
