/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   library.js — Library page: devices & packages from COPG.json
   Real data via COPG.* (js/copg-data.js); CRUD modals in js/modals.js;
   icons in js/icons.js; app picker in js/apppicker.js.
   Adds: icons, installed badges, sort + filter, swipe between tabs.
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* current search / sort / filter per panel */
const LibQuery = {
  devices: '', packages: '',
  devSort: 'default',  // default | name | apps | brand
  pkgSort: 'device',   // default | name | device | type | installed
  pkgFilter: 'all',    // all | installed | blocked | cpu_only
};

const installedSet = () => (window.COPG && COPG.installedSet) ? COPG.installedSet : new Set();

/* ─── Inner tab switch ─── */
function switchLibTab(tabKey) {
  if (tabKey === State.activeLibTab) return;
  State.activeLibTab = tabKey;
  $$('.inner-tab').forEach(t => t.classList.toggle('active', t.dataset.libTab === tabKey));
  $$('.lib-panel').forEach(p => {
    const isActive = p.id === (tabKey === 'devices' ? 'libDevices' : 'libPackages');
    p.classList.toggle('active', isActive);
  });
}

$$('.inner-tab').forEach(tab => {
  tab.addEventListener('click', e => { spawnRipple(e, tab); switchLibTab(tab.dataset.libTab); });
});

/* ─── Empty-state helper ─── */
function libEmpty(key) {
  const div = document.createElement('div');
  div.className = 'lib-empty';
  div.textContent = I18N.t(key);
  return div;
}

/* ─── small inline SVG helpers ─── */
const SVG_EDIT  = `<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 20h9"/><path d="M16.5 3.5a2.1 2.1 0 0 1 3 3L7 19l-4 1 1-4z"/></svg>`;
const SVG_TRASH = `<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="3 6 5 6 21 6"/><path d="M19 6l-1 14a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2L5 6"/><path d="M10 11v6M14 11v6"/><path d="M9 6V4a1 1 0 0 1 1-1h4a1 1 0 0 1 1 1v2"/></svg>`;
const SVG_PHONE = `<svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><rect x="5" y="2" width="14" height="20" rx="2"/><circle cx="12" cy="17" r="1" fill="currentColor"/></svg>`;
const SVG_SORT  = `<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 6h12M3 12h9M3 18h6"/><path d="M17 8V5m0 0 2.5 2.5M17 5l-2.5 2.5"/><path d="M17 16v3m0 0 2.5-2.5M17 19l-2.5-2.5"/></svg>`;

const chip = (cls, text) => { const s = document.createElement('span'); s.className = 'chip ' + cls; s.textContent = text; return s; };

/* ─── Sort / filter definitions ─── */
const SORT_OPTS = {
  devices:  [['default', 'sort_default'], ['name', 'sort_name'], ['name_desc', 'sort_name_za'], ['apps', 'sort_apps'], ['brand', 'sort_brand']],
  packages: [['default', 'sort_default'], ['name', 'sort_name'], ['name_desc', 'sort_name_za'], ['device', 'sort_device'], ['type', 'sort_type'], ['installed', 'sort_installed']],
};

function sortDevices(arr) {
  const a = [...arr];
  if (LibQuery.devSort === 'default') return a;
  if (LibQuery.devSort === 'name')       a.sort((x, y) => x.name.toLowerCase().localeCompare(y.name.toLowerCase()));
  else if (LibQuery.devSort === 'name_desc') a.sort((x, y) => y.name.toLowerCase().localeCompare(x.name.toLowerCase()));
  else if (LibQuery.devSort === 'apps')  a.sort((x, y) => y.gameCount - x.gameCount);
  else if (LibQuery.devSort === 'brand') a.sort((x, y) => (x.brand || '~').localeCompare(y.brand || '~'));
  return a;
}
function sortPackages(arr) {
  const set = installedSet();
  const a = [...arr];
  if (LibQuery.pkgSort === 'default') return a;
  // 'name'/'name_desc' sort by DISPLAY name (x.name), not the package id.
  if (LibQuery.pkgSort === 'name')   a.sort((x, y) => x.name.toLowerCase().localeCompare(y.name.toLowerCase()));
  else if (LibQuery.pkgSort === 'name_desc') a.sort((x, y) => y.name.toLowerCase().localeCompare(x.name.toLowerCase()));
  else if (LibQuery.pkgSort === 'device') a.sort((x, y) => (x.deviceName || '~').localeCompare(y.deviceName || '~'));
  else if (LibQuery.pkgSort === 'type')   a.sort((x, y) => x.type.localeCompare(y.type));
  else if (LibQuery.pkgSort === 'installed') a.sort((x, y) => (set.has(y.clean) ? 1 : 0) - (set.has(x.clean) ? 1 : 0));
  return a;
}
function pkgMatchesFilter(p) {
  const set = installedSet();
  switch (LibQuery.pkgFilter) {
    case 'installed': return set.has(p.clean);
    case 'blocked':   return p.type === 'blocked';
    case 'cpu_only':  return p.type === 'cpu_only';
    default:          return true;
  }
}

/* ─── Render Devices ─── */
function renderDevices() {
  const list = $('#deviceList');
  if (!list) return;
  const devices = COPG.listDevices();
  const cnt = $('#devTabCount');
  if (cnt) cnt.textContent = devices.length;

  const q = LibQuery.devices.trim().toLowerCase();
  let rows = q
    ? devices.filter(d => (d.name + ' ' + d.model + ' ' + d.brand).toLowerCase().includes(q))
    : devices;
  rows = sortDevices(rows);

  list.innerHTML = '';
  if (!rows.length) { list.appendChild(libEmpty('empty_devices')); return; }

  rows.forEach((dev, i) => {
    const card = document.createElement('div');
    card.className = 'device-card';
    card.style.animationDelay = `${Math.min(i * 45, 400)}ms`;
    card.innerHTML = `
      <div class="device-card__avatar">${SVG_PHONE}</div>
      <div class="device-card__info">
        <div class="device-card__name"></div>
        <div class="device-card__meta"></div>
      </div>
      <div class="card-actions">
        <button class="card-btn card-btn--edit" aria-label="${I18N.t('act_edit')}">${SVG_EDIT}</button>
        <button class="card-btn card-btn--delete" aria-label="${I18N.t('act_delete')}">${SVG_TRASH}</button>
      </div>`;
    card.querySelector('.device-card__name').textContent = dev.name;
    card.querySelector('.device-card__meta').textContent =
      `${I18N.t('dev_model')}: ${dev.model} · ${dev.gameCount} ${I18N.t('dev_apps')}`;

    card.querySelector('.card-btn--edit').addEventListener('click', e => {
      e.stopPropagation(); spawnRipple(e, card); Modals.openDevice(dev.key);
    });
    card.querySelector('.card-btn--delete').addEventListener('click', e => {
      e.stopPropagation();
      Modals.confirmDelete(
        I18N.t('confirm_del_device_title'),
        `${dev.name} — ${dev.gameCount} ${I18N.t('dev_apps')}`,
        async () => { COPG.deleteDevice(dev.key); await persistAndRefresh(I18N.t('toast_device_deleted')); }
      );
    });
    card.addEventListener('click', e => { spawnRipple(e, card); Modals.openDevice(dev.key); });
    list.appendChild(card);
  });
}

/* ─── Render Packages ─── */
function renderPackages() {
  const list = $('#packageList');
  if (!list) return;
  const pkgs = COPG.listPackages();
  const cnt = $('#pkgTabCount');
  if (cnt) cnt.textContent = pkgs.length;

  const set = installedSet();
  const q = LibQuery.packages.trim().toLowerCase();
  let rows = pkgs.filter(pkgMatchesFilter);
  if (q) rows = rows.filter(p => (p.name + ' ' + p.clean).toLowerCase().includes(q));
  rows = sortPackages(rows);

  list.innerHTML = '';
  if (!rows.length) { list.appendChild(libEmpty('empty_packages')); return; }

  rows.forEach((pkg, i) => {
    const card = document.createElement('div');
    card.className = 'package-card';
    card.style.animationDelay = `${Math.min(i * 30, 400)}ms`;
    card.innerHTML = `
      <div class="package-card__icon" data-icon></div>
      <div class="package-card__info">
        <div class="package-card__device"></div>
        <div class="package-card__name"></div>
        <div class="package-card__pkg"></div>
        <div class="package-card__chips"></div>
      </div>
      <div class="card-actions">
        <button class="card-btn card-btn--edit" aria-label="${I18N.t('act_edit')}">${SVG_EDIT}</button>
        <button class="card-btn card-btn--delete" aria-label="${I18N.t('act_delete')}">${SVG_TRASH}</button>
      </div>`;
    card.querySelector('.package-card__name').textContent = pkg.name;
    card.querySelector('.package-card__pkg').textContent = pkg.clean;

    // Dim text ABOVE the name (same slot/style as the device model): the device
    // name for device packages, or the spoof type for cpu_only / blocked.
    const devEl = card.querySelector('.package-card__device');
    let aboveText = '';
    if (pkg.type === 'device')        aboveText = pkg.deviceName || '';
    else if (pkg.type === 'cpu_only') aboveText = I18N.t('pkgtype_cpu_only');   // "CPU Spoof"
    else if (pkg.type === 'blocked')  aboveText = I18N.t('pkgtype_blocked');    // "Block CPU Spoof"
    if (aboveText) devEl.textContent = aboveText;
    else devEl.remove();

    // chips: tags + installed (type now shown above the name, not as a chip)
    const chips = card.querySelector('.package-card__chips');
    if (pkg.with_cpu)    chips.appendChild(chip('chip--withcpu', I18N.t('tag_withcpu')));
    if (pkg.got)         chips.appendChild(chip('chip--got', I18N.t('tag_got')));
    // tweak tags — muted shared style so they read as a group, not as loud as spoof chips
    if (pkg.dnd)         chips.appendChild(chip('chip--tweak', I18N.t('tag_dnd')));
    if (pkg.dab)         chips.appendChild(chip('chip--tweak', I18N.t('tag_dab')));
    if (pkg.kso)         chips.appendChild(chip('chip--tweak', I18N.t('tag_kso')));
    if (pkg.nolog)       chips.appendChild(chip('chip--tweak', I18N.t('tag_nolog')));
    if (set.has(pkg.clean)) chips.appendChild(chip('chip--installed', I18N.t('badge_installed')));

    // lazy icon
    if (window.Icons) Icons.load(card.querySelector('[data-icon]'), pkg.clean, pkg.name);

    card.querySelector('.card-btn--edit').addEventListener('click', e => {
      e.stopPropagation(); spawnRipple(e, card); Modals.openPackage(pkg);
    });
    card.querySelector('.card-btn--delete').addEventListener('click', e => {
      e.stopPropagation();
      Modals.confirmDelete(
        I18N.t('confirm_del_package_title'),
        `${pkg.name}\n${pkg.clean}`,
        async () => { COPG.deletePackage(pkg.clean, pkg.type, pkg.deviceKey); await persistAndRefresh(I18N.t('toast_package_deleted')); }
      );
    });
    card.addEventListener('click', e => { spawnRipple(e, card); Modals.openPackage(pkg); });
    list.appendChild(card);
  });
}

/* ─── Persist + refresh both lists + banner ─── */
async function persistAndRefresh(successMsg) {
  try {
    if (window.logEvent) logEvent('Saving COPG.json / list.json…', 'info');
    const res = await COPG.save();
    renderDevices();
    renderPackages();
    if (typeof updateStats === 'function') updateStats();
    updatePreviewBanner();
    if (res && res.preview) {
      showToast(I18N.t('toast_preview_only'));
      if (window.logEvent) logEvent('Preview mode — not written to device.', 'warn');
    } else {
      if (successMsg) showToast(successMsg);
      if (window.logEvent) logEvent('Saved to /data/adb/modules/COPG (' + COPG.listDevices().length + ' devices, ' + COPG.listPackages().length + ' packages).', 'success');
    }
  } catch (err) {
    showToast(I18N.t('toast_save_failed'));
    console.error('save failed:', err);
  }
}

/* ─── Preview banner (no bridge) ─── */
function updatePreviewBanner() {
  const banner = $('#previewBanner');
  if (!banner) return;
  const show = !COPG.hasBridge() && COPG.previewUnsaved;
  banner.classList.toggle('visible', show);
}

/* ─── Sort sheet (built dynamically, reused for devices & packages) ─── */
document.body.insertAdjacentHTML('beforeend', `
  <div class="bottom-sheet" id="sortSheet" role="dialog" aria-modal="true" aria-label="Sort">
    <div class="sheet-handle"></div>
    <div class="sheet-header">
      <span class="sheet-header__title" data-i18n="sort_title">Sort by</span>
      <button class="sheet-header__close" id="sortClose" aria-label="Close">
        <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>
      </button>
    </div>
    <div class="sheet-body" id="sortSheetBody"></div>
  </div>`);
$('#sortClose')?.addEventListener('click', () => closeAllSheets());

function openSortSheet(ctx) {
  if (!SORT_OPTS[ctx]) return;          // ignore stray callers (e.g. other sheets' sort buttons)
  const body = $('#sortSheetBody');
  const cur = ctx === 'devices' ? LibQuery.devSort : LibQuery.pkgSort;
  body.innerHTML = '';
  SORT_OPTS[ctx].forEach(([val, key]) => {
    const row = document.createElement('div');
    row.className = 'sheet-option' + (val === cur ? ' selected' : '');
    row.innerHTML =
      `<div class="sheet-option__icon">${SVG_SORT}</div>` +
      `<span class="sheet-option__label">${I18N.t(key)}</span>` +
      `<svg class="sheet-option__check" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round"><polyline points="20 6 9 17 4 12"/></svg>`;
    row.addEventListener('click', () => {
      if (ctx === 'devices') LibQuery.devSort = val; else LibQuery.pkgSort = val;
      updateSortLabels();
      ctx === 'devices' ? renderDevices() : renderPackages();
      closeAllSheets();
    });
    body.appendChild(row);
  });
  openSheet('sortSheet');
}

function updateSortLabels() {
  const dl = document.querySelector('#libDevices .lib-sort__label');
  const pl = document.querySelector('#libPackages .lib-sort__label');
  const find = (ctx, val) => (SORT_OPTS[ctx].find(o => o[0] === val) || [])[1];
  if (dl) dl.textContent = I18N.t(find('devices', LibQuery.devSort));
  if (pl) pl.textContent = I18N.t(find('packages', LibQuery.pkgSort));
}

/* ─── Controls (sort pill + package filter chips), injected per panel ─── */
function buildControls() {
  const sortPill = ctx => `
    <button class="lib-sort" data-sort-ctx="${ctx}" type="button">
      ${SVG_SORT}<span class="lib-sort__label"></span>
    </button>`;

  const devSearch = document.querySelector('#libDevices .lib-searchrow');
  if (devSearch && !document.querySelector('#libDevices .lib-controls')) {
    devSearch.insertAdjacentHTML('afterend', `<div class="lib-controls">${sortPill('devices')}</div>`);
  }
  const pkgSearch = document.querySelector('#libPackages .lib-searchrow');
  if (pkgSearch && !document.querySelector('#libPackages .lib-controls')) {
    pkgSearch.insertAdjacentHTML('afterend', `
      <div class="lib-controls">
        ${sortPill('packages')}
        <div class="lib-filters">
          <button class="fchip active" data-filter="all" data-i18n="filter_all">All</button>
          <button class="fchip" data-filter="installed" data-i18n="filter_installed">Installed</button>
          <button class="fchip" data-filter="blocked" data-i18n="filter_blocked">Blocked</button>
          <button class="fchip" data-filter="cpu_only" data-i18n="filter_cpu_only">CPU</button>
        </div>
      </div>`);
  }

  // scope to the Library panels only — the App Picker has its own .lib-sort (#apSort)
  $$('#pageLibrary .lib-sort').forEach(btn => btn.addEventListener('click', e => {
    spawnRipple(e, btn); openSortSheet(btn.dataset.sortCtx);
  }));
  $$('#libPackages .fchip').forEach(btn => btn.addEventListener('click', e => {
    spawnRipple(e, btn);
    LibQuery.pkgFilter = btn.dataset.filter;
    $$('#libPackages .fchip').forEach(b => b.classList.toggle('active', b === btn));
    renderPackages();
  }));
  updateSortLabels();
}

/* ─── Swipe between Devices / Packages ─── */
function wireSwipe() {
  const area = document.querySelector('#pageLibrary .scroll-area');
  if (!area) return;
  let x0 = 0, y0 = 0, t0 = 0, tracking = false;
  area.addEventListener('touchstart', e => {
    if (e.touches.length !== 1) { tracking = false; return; }
    // don't hijack horizontal scrolls of the filter/sort row or the search row
    if (e.target.closest('.lib-controls') || e.target.closest('.lib-searchrow')) { tracking = false; return; }
    x0 = e.touches[0].clientX; y0 = e.touches[0].clientY; t0 = Date.now(); tracking = true;
  }, { passive: true });
  area.addEventListener('touchend', e => {
    if (!tracking) return;
    tracking = false;
    const t = e.changedTouches[0];
    const dx = t.clientX - x0, dy = t.clientY - y0, dt = Date.now() - t0;
    if (dt > 600) return;
    // require a deliberate, dominantly-horizontal swipe (bigger threshold so
    // it won't fire by accident while scrolling the filter chips)
    if (Math.abs(dx) < 80 || Math.abs(dx) < Math.abs(dy) * 2) return;
    if (dx < 0 && State.activeLibTab === 'devices') switchLibTab('packages');
    else if (dx > 0 && State.activeLibTab === 'packages') switchLibTab('devices');
  }, { passive: true });
}

/* ─── Add buttons ─── */
$('#addDeviceBtn')?.addEventListener('click', e => { spawnRipple(e, e.currentTarget); Modals.openDevice(null); });
$('#addPackageBtn')?.addEventListener('click', e => { spawnRipple(e, e.currentTarget); Modals.openPackage(null); });

/* ─── Search inputs ─── */
$('#deviceSearch')?.addEventListener('input', e => { LibQuery.devices = e.target.value; renderDevices(); });
$('#packageSearch')?.addEventListener('input', e => { LibQuery.packages = e.target.value; renderPackages(); });

/* expose refresh so modals can call after saving */
window.LibRefresh = persistAndRefresh;

/* initial setup + paint */
buildControls();
wireSwipe();
renderDevices();
renderPackages();
updatePreviewBanner();
