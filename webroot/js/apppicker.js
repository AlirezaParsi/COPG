/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   apppicker.js — "Pick from installed apps" bottom-sheet
   Simple list: search + tappable rows (icon + label + package). Apps come from
   COPG.listInstalledApps() (ksu.listPackages → pm fallback) with real labels
   filled in afterwards via COPG.enrichApps() so the list shows immediately.
   Selecting a row calls back with {pkg,label}. Shares #sheetOverlay.
   window.AppPicker.open(cb)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
(function (w) {
  document.body.insertAdjacentHTML('beforeend', `
    <div class="bottom-sheet sheet--picker" id="appPickerSheet" role="dialog" aria-modal="true" aria-label="Installed apps">
      <div class="sheet-handle"></div>
      <div class="sheet-header">
        <span class="sheet-header__title" data-i18n="picker_title">Installed Apps</span>
        <button class="sheet-header__close" id="apClose" aria-label="Close">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>
        </button>
      </div>
      <div class="picker-search">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><circle cx="11" cy="11" r="7"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg>
        <input type="search" id="apSearch" autocomplete="off" data-i18n-attr="placeholder:picker_search" placeholder="Search apps…" />
      </div>
      <div class="picker-controls">
        <div class="pk-filters" id="apFilters">
          <button type="button" class="pk-chip" data-filter="all" data-i18n="filter_all">All</button>
          <button type="button" class="pk-chip selected" data-filter="user" data-i18n="filter_user">User</button>
          <button type="button" class="pk-chip" data-filter="system" data-i18n="filter_system">System</button>
        </div>
        <button type="button" class="pk-sort" id="apSortBtn" aria-label="Sort A→Z / Z→A">
          <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M7 5v14"/><path d="M4 8l3-3 3 3"/><path d="M14 7h6M14 12h4M14 17h2"/></svg>
          <span id="apSortLabel" data-i18n="sort_name">Name (A→Z)</span>
        </button>
      </div>
      <div class="picker-body">
        <div class="picker-list" id="apList"></div>
        <div class="picker-empty" id="apEmpty" hidden>
          <svg width="34" height="34" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><circle cx="11" cy="11" r="7"/><line x1="21" y1="21" x2="16.65" y2="16.65"/><line x1="8.5" y1="11" x2="13.5" y2="11"/></svg>
          <span id="apEmptyMsg"></span>
        </div>
      </div>
    </div>`);

  const $a = s => document.querySelector(s);
  const overlay = () => document.getElementById('sheetOverlay');
  const sheet = () => document.getElementById('appPickerSheet');

  let cb = null;
  let loaded = false;
  let rowData = [];            // [{el, label, search, system}]
  let query = '';
  let filter = 'user';         // 'all' | 'user' | 'system' (default: user apps)
  let sortDir = 'az';          // 'az' | 'za'

  function close() {
    const s = sheet();
    if (s) { s.classList.remove('open'); s.style.transition = ''; s.style.transform = ''; }
    if (![...document.querySelectorAll('.bottom-sheet.open')].length) overlay()?.classList.remove('visible');
  }
  $a('#apClose').addEventListener('click', close);

  /* Centered empty/no-results state — toggled WITHOUT touching the row list, so
     the picker never collapses or reloads. */
  function showEmpty(msg) { $a('#apEmptyMsg').textContent = msg; $a('#apEmpty').hidden = false; }
  function hideEmpty() { $a('#apEmpty').hidden = true; }

  function render(apps) {
    const list = $a('#apList');
    list.innerHTML = '';
    rowData = [];
    if (!apps.length) {
      showEmpty(I18N.t(COPG.hasBridge() ? 'picker_empty' : 'picker_preview'));
      return;
    }
    hideEmpty();
    const frag = document.createDocumentFragment();
    apps.forEach(app => {
      const label = app.label || app.pkg;
      const row = document.createElement('button');
      row.type = 'button';
      row.className = 'app-row';
      row.dataset.pkg = app.pkg;
      row.innerHTML =
        `<span class="app-row__icon" data-icon></span>` +
        `<span class="app-row__text"><span class="app-row__label"></span><span class="app-row__pkg"></span></span>`;
      row.querySelector('.app-row__label').textContent = label;
      row.querySelector('.app-row__pkg').textContent = app.pkg;
      if (w.Icons) Icons.load(row.querySelector('[data-icon]'), app.pkg, label);
      row.addEventListener('click', () => {
        const lbl = row.querySelector('.app-row__label').textContent;
        const fn = cb; cb = null;
        close();
        if (fn) fn({ pkg: app.pkg, label: lbl });
      });
      frag.appendChild(row);
      rowData.push({ el: row, label, search: (label + ' ' + app.pkg).toLowerCase(), system: !!app.system });
    });
    list.appendChild(frag);
    sortRows();
    applyView();
  }

  function matchesFilter(r) {
    if (filter === 'all')    return true;
    if (filter === 'system') return r.system;
    return !r.system;                 // 'user'
  }

  /* Search + filter — show/hide rows in place, never reload. */
  function applyView() {
    let visible = 0;
    rowData.forEach(r => {
      const show = (!query || r.search.includes(query)) && matchesFilter(r);
      r.el.style.display = show ? '' : 'none';
      if (show) visible++;
    });
    if (rowData.length && visible === 0) showEmpty(I18N.t('picker_no_results'));
    else hideEmpty();
  }

  /* Re-order the DOM rows (and rowData) by label, honouring sortDir. */
  function sortRows() {
    const dir = sortDir === 'za' ? -1 : 1;
    rowData.sort((a, b) => dir * a.label.toLowerCase().localeCompare(b.label.toLowerCase()));
    const list = $a('#apList');
    rowData.forEach(r => list.appendChild(r.el));
  }

  function updateSortLabel() {
    const span = $a('#apSortLabel');
    if (!span) return;
    const key = sortDir === 'az' ? 'sort_name' : 'sort_name_za';
    span.dataset.i18n = key;          // keep data-i18n in sync so I18N.apply() is correct
    span.textContent = I18N.t(key);
  }

  function setFilter(f) {
    filter = f;
    const fz = $a('#apFilters');
    if (fz) fz.querySelectorAll('.pk-chip').forEach(c => c.classList.toggle('selected', c.dataset.filter === f));
  }

  async function loadList() {
    const list = $a('#apList');
    hideEmpty();
    list.innerHTML = '<div class="picker-loading"><span class="spin"></span></div>';
    if (window.logEvent) logEvent('App picker: loading installed apps…', 'info');
    let apps = [];
    try {
      apps = (w.COPG && COPG.listInstalledApps) ? await COPG.listInstalledApps() : [];
    } catch (err) {
      if (window.logEvent) logEvent('App picker: load failed — ' + err, 'error');
    }
    render(apps);                       // show immediately (fast labels)
    loaded = true;
    if (window.logEvent) logEvent('App picker: ' + apps.length + ' apps loaded.', apps.length ? 'success' : 'warn');
    if (apps.length) enrichInBackground(apps.map(a => a.pkg));
  }

  /* After the list is on screen, fill in real labels without a reload — update
     text in place, re-sort A→Z, re-apply the search. Never blocks the render. */
  async function enrichInBackground(pkgs) {
    if (!(w.COPG && COPG.enrichApps)) return;
    let data;
    try { data = await COPG.enrichApps(pkgs); }
    catch (err) { if (window.logEvent) logEvent('App picker: enrich failed — ' + err, 'warn'); return; }
    const labels = (data && data.labels) || {};
    const sys    = (data && data.system) || new Set();
    let changed = 0;
    rowData.forEach(r => {
      const pkg = r.el.dataset.pkg;
      if (labels[pkg] && labels[pkg] !== r.label) {
        r.label = labels[pkg];
        r.search = (r.label + ' ' + pkg).toLowerCase();
        r.el.querySelector('.app-row__label').textContent = r.label;
        changed++;
      }
      r.system = sys.has(pkg);          // user/system now known → drives the filter
    });
    // Always re-sort (labels may have changed) and re-apply (the system filter
    // could not be honoured until the system set arrived from enrichment).
    sortRows();
    applyView();
    if (window.logEvent) logEvent('App picker: enriched ' + changed + ' labels, ' + sys.size + ' system.', 'info');
  }

  $a('#apSearch').addEventListener('input', e => { query = e.target.value.trim().toLowerCase(); applyView(); });

  /* Filter chips (All / User / System) — instant, preserves search + scroll. */
  $a('#apFilters').addEventListener('click', e => {
    const chip = e.target.closest('.pk-chip');
    if (!chip) return;
    setFilter(chip.dataset.filter);
    applyView();
  });

  /* Sort toggle (Name A→Z ⇄ Z→A). */
  $a('#apSortBtn').addEventListener('click', () => {
    sortDir = sortDir === 'az' ? 'za' : 'az';
    updateSortLabel();
    sortRows();
    applyView();
  });

  function open(callback) {
    cb = callback || null;
    query = '';
    setFilter('user');                 // default filter = user apps
    sortDir = 'az'; updateSortLabel();
    const s = sheet();
    const inp = $a('#apSearch'); if (inp) inp.value = '';
    overlay()?.classList.add('visible');
    // Clear any residual inline transform/transition left by a drag-to-dismiss
    // gesture. An inline `transform:translateY(Npx)` BEATS the `.bottom-sheet.open`
    // stylesheet rule, so without this the sheet would stay off-screen and could
    // never visually reopen after the first drag (no error, no log → "nothing happens").
    if (s) { s.style.transition = ''; s.style.transform = ''; s.classList.remove('open'); }
    requestAnimationFrame(() => s && s.classList.add('open'));
    if (window.logEvent) logEvent('App picker opened.', 'info');
    if (!loaded) loadList();
    else { sortRows(); applyView(); }    // reuse cached rows (no reload)
  }

  w.AppPicker = { open };
})(window);
