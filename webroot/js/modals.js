/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   modals.js — Device & Package add/edit/delete modals
   Built as bottom-sheets (same visual family as theme/lang sheets).
   Data goes through COPG.* (js/copg-data.js); on save calls
   window.LibRefresh() (defined in library.js) to persist + re-render.
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
(function (w) {

  /* ─── DOM scaffold: inject three sheets + reuse #sheetOverlay ─── */
  const mount = document.getElementById('modalRoot') || document.body;

  /* little info "ⓘ" used on the cpu/got toggle rows → tap for a plain explanation */
  const INFO_SVG =
    `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><line x1="12" y1="16" x2="12" y2="12"/><line x1="12" y1="8" x2="12.01" y2="8"/></svg>`;

  mount.insertAdjacentHTML('beforeend', `
    <!-- DEVICE MODAL -->
    <div class="bottom-sheet sheet--form" id="deviceModal" role="dialog" aria-modal="true">
      <div class="sheet-handle"></div>
      <div class="sheet-header">
        <span class="sheet-header__title" id="deviceModalTitle" data-i18n="dev_add_title">Add Device</span>
        <button class="sheet-header__close" data-close="deviceModal" aria-label="Close">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>
        </button>
      </div>
      <form class="sheet-body form-body" id="deviceForm" autocomplete="off">
        <label class="field"><span class="field__label" data-i18n="dev_f_name">Device Name</span>
          <input class="field__input" id="dfName" type="text" required /></label>
        <label class="field"><span class="field__label" data-i18n="dev_f_brand">Brand</span>
          <input class="field__input" id="dfBrand" type="text" /></label>
        <label class="field"><span class="field__label" data-i18n="dev_f_model">Model</span>
          <input class="field__input" id="dfModel" type="text" required /></label>
        <label class="field"><span class="field__label" data-i18n="dev_f_manufacturer">Manufacturer</span>
          <input class="field__input" id="dfManufacturer" type="text" required /></label>
        <label class="field"><span class="field__label" data-i18n="dev_f_fingerprint">Fingerprint</span>
          <input class="field__input field__input--mono" id="dfFingerprint" type="text" required /></label>
        <div class="field-row">
          <label class="field"><span class="field__label" data-i18n="dev_f_android">Android Version</span>
            <input class="field__input" id="dfAndroid" type="text" inputmode="decimal" /></label>
          <label class="field"><span class="field__label" data-i18n="dev_f_sdk">SDK Int</span>
            <input class="field__input" id="dfSdk" type="text" inputmode="numeric" /></label>
        </div>
        <div class="form-buttons">
          <button type="button" class="btn btn--ghost" data-close="deviceModal" data-i18n="btn_cancel">Cancel</button>
          <button type="submit" class="btn btn--primary" data-i18n="btn_save">Save</button>
        </div>
      </form>
    </div>

    <!-- PACKAGE MODAL -->
    <div class="bottom-sheet sheet--form" id="packageModal" role="dialog" aria-modal="true">
      <div class="sheet-handle"></div>
      <div class="sheet-header">
        <span class="sheet-header__title" id="packageModalTitle" data-i18n="pkg_add_title">Add Package</span>
        <button class="sheet-header__close" data-close="packageModal" aria-label="Close">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>
        </button>
      </div>
      <form class="sheet-body form-body" id="packageForm" autocomplete="off">
        <label class="field"><span class="field__label" data-i18n="pkg_f_package">Package Name</span>
          <div class="field-with-btn">
            <input class="field__input field__input--mono" id="pfPackage" type="text" required placeholder="com.example.app" />
            <button type="button" class="field-pick-btn" id="pfPickBtn" data-i18n-attr="aria-label:pick_from_installed" aria-label="Pick from installed apps">
              <svg width="19" height="19" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="7" height="7" rx="2.2" fill="currentColor" stroke="none"/><rect x="14" y="3" width="7" height="7" rx="2.2"/><rect x="3" y="14" width="7" height="7" rx="2.2"/><rect x="14" y="14" width="7" height="7" rx="2.2"/></svg>
            </button>
          </div></label>
        <label class="field"><span class="field__label" data-i18n="pkg_f_name">Display Name</span>
          <input class="field__input" id="pfName" type="text" /></label>

        <div class="field"><span class="field__label" data-i18n="pkg_f_type">Type</span>
          <div class="seg" id="pfType">
            <button type="button" class="seg__opt active" data-type="device"   data-i18n="pkgtype_device">Device</button>
            <button type="button" class="seg__opt"        data-type="cpu_only"  data-i18n="pkgtype_cpu_only">CPU Only</button>
            <button type="button" class="seg__opt"        data-type="blocked"   data-i18n="pkgtype_blocked">Blocked</button>
          </div>
        </div>

        <label class="field" id="pfDeviceField"><span class="field__label" data-i18n="pkg_f_device">Device Profile</span>
          <select class="field__input field__select" id="pfDevice"></select></label>

        <div class="toggles" id="pfToggles">
          <div class="toggles__head" id="pfSpoofHead" data-i18n="pkg_sec_spoofing">Spoofing</div>
          <label class="trow" id="pfWithCpuRow">
            <span class="trow__label"><span data-i18n="pkg_t_withcpu">With CPU Spoofing</span>
              <span class="trow__info" role="button" tabindex="0" data-info="withcpu" data-i18n-attr="aria-label:info_aria" aria-label="What's this?">${INFO_SVG}</span></span>
            <span class="toggle"><input type="checkbox" id="pfWithCpu"><span class="toggle__track"><span class="toggle__thumb"></span></span></span></label>
          <label class="trow" id="pfGotRow">
            <span class="trow__label"><span data-i18n="pkg_t_got">GOT Hooking</span>
              <span class="trow__info" role="button" tabindex="0" data-info="got" data-i18n-attr="aria-label:info_aria" aria-label="What's this?">${INFO_SVG}</span></span>
            <span class="toggle"><input type="checkbox" id="pfGot"><span class="toggle__track"><span class="toggle__thumb"></span></span></span></label>

          <div class="toggles__head has-divider" id="pfTweakHead" data-i18n="pkg_sec_tweaks">Tweaks</div>
          <label class="trow" id="pfDndRow">
            <span class="trow__label"><span data-i18n="pkg_t_dnd">Do Not Disturb</span>
              <span class="trow__info" role="button" tabindex="0" data-info="dnd" data-i18n-attr="aria-label:info_aria" aria-label="What's this?">${INFO_SVG}</span></span>
            <span class="toggle"><input type="checkbox" id="pfDnd"><span class="toggle__track"><span class="toggle__thumb"></span></span></span></label>
          <label class="trow" id="pfDabRow">
            <span class="trow__label"><span data-i18n="pkg_t_dab">Disable Auto-Brightness</span>
              <span class="trow__info" role="button" tabindex="0" data-info="dab" data-i18n-attr="aria-label:info_aria" aria-label="What's this?">${INFO_SVG}</span></span>
            <span class="toggle"><input type="checkbox" id="pfDab"><span class="toggle__track"><span class="toggle__thumb"></span></span></span></label>
          <label class="trow" id="pfKsoRow">
            <span class="trow__label"><span data-i18n="pkg_t_kso">Keep Screen On</span>
              <span class="trow__info" role="button" tabindex="0" data-info="kso" data-i18n-attr="aria-label:info_aria" aria-label="What's this?">${INFO_SVG}</span></span>
            <span class="toggle"><input type="checkbox" id="pfKso"><span class="toggle__track"><span class="toggle__thumb"></span></span></span></label>
          <label class="trow" id="pfNologRow">
            <span class="trow__label"><span data-i18n="pkg_t_nolog">Disable Logging</span>
              <span class="trow__info" role="button" tabindex="0" data-info="nolog" data-i18n-attr="aria-label:info_aria" aria-label="What's this?">${INFO_SVG}</span></span>
            <span class="toggle"><input type="checkbox" id="pfNolog"><span class="toggle__track"><span class="toggle__thumb"></span></span></span></label>
        </div>

        <div class="form-error" id="pfError" hidden></div>
        <div class="form-buttons">
          <button type="button" class="btn btn--ghost" data-close="packageModal" data-i18n="btn_cancel">Cancel</button>
          <button type="submit" class="btn btn--primary" data-i18n="btn_save">Save</button>
        </div>
      </form>
    </div>

    <!-- CONFIRM DELETE -->
    <div class="bottom-sheet sheet--confirm" id="confirmModal" role="dialog" aria-modal="true">
      <div class="sheet-handle"></div>
      <div class="sheet-header">
        <span class="sheet-header__title" id="confirmTitle">Delete</span>
      </div>
      <div class="sheet-body">
        <p class="confirm-msg" id="confirmMsg"></p>
        <a class="confirm-link" id="confirmLink" target="_blank" rel="noopener noreferrer" hidden></a>
        <div class="form-buttons">
          <button type="button" class="btn btn--ghost" id="confirmCancel" data-close="confirmModal" data-i18n="btn_cancel">Cancel</button>
          <button type="button" class="btn btn--danger" id="confirmOk" data-i18n="btn_delete">Delete</button>
        </div>
      </div>
    </div>
  `);

  const $m = s => document.querySelector(s);
  const overlay = () => document.getElementById('sheetOverlay');

  // Clearing any inline transform/transition is essential: a drag-to-dismiss
  // gesture (gestures.js) leaves an inline `transform:translateY()` that BEATS
  // the `.bottom-sheet.open` rule, so a sheet could otherwise never visually
  // reopen after being dragged (no error, no log — just "nothing happens").
  function clearDrag(el) { if (el) { el.style.transition = ''; el.style.transform = ''; } }

  function openModal(id) {
    overlay()?.classList.add('visible');
    const sheet = document.getElementById(id);
    clearDrag(sheet);
    requestAnimationFrame(() => sheet?.classList.add('open'));
  }
  function closeModal(id) {
    const sheet = document.getElementById(id);
    sheet?.classList.remove('open');
    clearDrag(sheet);
    if (![...document.querySelectorAll('.bottom-sheet.open')].length) overlay()?.classList.remove('visible');
  }
  function closeAll() {
    document.querySelectorAll('.bottom-sheet').forEach(s => { s.classList.remove('open'); clearDrag(s); });
    overlay()?.classList.remove('visible');
  }

  // close buttons + overlay
  document.querySelectorAll('[data-close]').forEach(b =>
    b.addEventListener('click', () => closeModal(b.dataset.close)));
  overlay()?.addEventListener('click', closeAll);

  /* field error helpers */
  function clearErrors(form) {
    form.querySelectorAll('.field--error').forEach(f => f.classList.remove('field--error'));
    form.querySelectorAll('.field-msg').forEach(m => m.remove());
  }
  function markError(inputEl, msg) {
    const field = inputEl.closest('.field') || inputEl.parentNode;
    field.classList.add('field--error');
    const span = document.createElement('span');
    span.className = 'field-msg';
    span.textContent = msg;
    field.appendChild(span);
  }

  /* ════════════ DEVICE MODAL ════════════ */
  let editingDeviceKey = null;

  function openDevice(deviceKey) {
    const form = $m('#deviceForm');
    clearErrors(form);
    editingDeviceKey = deviceKey || null;
    $m('#deviceModalTitle').textContent = I18N.t(deviceKey ? 'dev_edit_title' : 'dev_add_title');

    if (deviceKey) {
      const d = COPG.config[deviceKey] || {};
      $m('#dfName').value = d.DEVICE || '';
      $m('#dfBrand').value = d.BRAND || '';
      $m('#dfModel').value = d.MODEL || '';
      $m('#dfManufacturer').value = d.MANUFACTURER || '';
      $m('#dfFingerprint').value = d.FINGERPRINT || '';
      $m('#dfAndroid').value = d.ANDROID_VERSION || '';
      $m('#dfSdk').value = d.SDK_INT || '';
    } else {
      form.reset();
    }
    openModal('deviceModal');
  }

  /* SDK is derived from + LOCKED to the Android version: typing 16 → 36,
     clearing/unknown Android → SDK clears too (prevents stale-SDK mistakes). */
  function wireAndroidSdk() {
    const a = $m('#dfAndroid'), s = $m('#dfSdk');
    if (!a || !s) return;
    s.readOnly = true;
    s.classList.add('field__input--locked');
    const sync = () => {
      const v = a.value.trim();
      const sug = v ? COPG.sdkFromAndroid(v) : null;
      const next = (sug != null) ? String(sug) : '';
      if (next !== s.value) { s.value = next; if (next) flash(s); }
    };
    a.addEventListener('input', sync);
  }
  function flash(el) {
    el.classList.add('field__input--suggested');
    setTimeout(() => el.classList.remove('field__input--suggested'), 2500);
  }

  $m('#deviceForm').addEventListener('submit', async e => {
    e.preventDefault();
    const form = e.currentTarget;
    clearErrors(form);
    const fields = {
      name: $m('#dfName'), model: $m('#dfModel'),
      manufacturer: $m('#dfManufacturer'), fingerprint: $m('#dfFingerprint'),
    };
    let bad = false;
    for (const [, el] of Object.entries(fields)) {
      if (!el.value.trim()) { markError(el, I18N.t('msg_required')); bad = true; }
    }
    if (bad) return;

    const name = fields.name.value.trim();
    const model = fields.model.value.trim();
    const dup = COPG.deviceNameExists(name, model, editingDeviceKey);
    if (dup && !editingDeviceKey) {
      if (dup.field === 'name')  markError(fields.name,  I18N.t('msg_dup_device'));
      if (dup.field === 'model') markError(fields.model, I18N.t('msg_dup_model'));
      return;
    }

    COPG.upsertDevice({
      name, brand: $m('#dfBrand').value, model,
      manufacturer: fields.manufacturer.value, fingerprint: fields.fingerprint.value,
      android: $m('#dfAndroid').value, sdk: $m('#dfSdk').value,
    }, editingDeviceKey);

    closeAll();
    await w.LibRefresh(I18N.t(editingDeviceKey ? 'toast_device_saved' : 'toast_device_added'));
  });

  /* ════════════ PACKAGE MODAL ════════════ */
  let editingPkg = null;  // { clean, type, deviceKey } | null

  function populateDeviceSelect(selectedKey) {
    const sel = $m('#pfDevice');
    sel.innerHTML = '';
    const devs = COPG.listDevices();
    if (!devs.length) {
      const o = document.createElement('option');
      o.value = ''; o.textContent = I18N.t('pkg_no_devices'); o.disabled = true;
      sel.appendChild(o);
      return;
    }
    devs.forEach(d => {
      const o = document.createElement('option');
      // value must be the package-array key (PACKAGES_X) — that's what listPackages
      // tags onto pkg.deviceKey. Using d.key (PACKAGES_X_DEVICE) broke preselect, so
      // an edited package silently re-homed to the first device (and to list end).
      o.value = d.pkgKey; o.textContent = d.name;
      if (d.pkgKey === selectedKey) o.selected = true;
      sel.appendChild(o);
    });
  }

  // Spoof toggles (with_cpu/blocked/got) are device-type only. Tweak toggles
  // (dnd/dab/kso/nolog) apply to device AND cpu_only. 'blocked' type gets neither.
  const SPOOF_EL = ['pfSpoofHead', 'pfWithCpuRow', 'pfGotRow'];
  const SPOOF_CB = ['pfWithCpu', 'pfGot'];
  const TWEAK_EL = ['pfTweakHead', 'pfDndRow', 'pfDabRow', 'pfKsoRow', 'pfNologRow'];
  const TWEAK_CB = ['pfDnd', 'pfDab', 'pfKso', 'pfNolog'];
  function setType(type) {
    $$('#pfType .seg__opt').forEach(b => b.classList.toggle('active', b.dataset.type === type));
    const isDevice  = type === 'device';
    const showSpoof = isDevice;
    const showTweak = isDevice || type === 'cpu_only';
    $m('#pfDeviceField').style.display = isDevice ? '' : 'none';
    $m('#pfToggles').style.display     = (showSpoof || showTweak) ? '' : 'none';
    SPOOF_EL.forEach(id => { $m('#' + id).style.display = showSpoof ? '' : 'none'; });
    TWEAK_EL.forEach(id => { $m('#' + id).style.display = showTweak ? '' : 'none'; });
    // divider above Tweaks only when the Spoofing group sits above it
    $m('#pfTweakHead').classList.toggle('has-divider', showSpoof);
    if (!showSpoof) SPOOF_CB.forEach(id => { $m('#' + id).checked = false; });
    if (!showTweak) TWEAK_CB.forEach(id => { $m('#' + id).checked = false; });
  }
  function currentType() {
    return ($m('#pfType .seg__opt.active') || {}).dataset?.type || 'device';
  }

  // segmented control
  $$('#pfType .seg__opt').forEach(b => b.addEventListener('click', () => setType(b.dataset.type)));
  // CPU default is BLOCK (unmount) module-side; "With CPU Spoofing" off = block, so no
  // separate block toggle / mutual-exclusion needed anymore.

  // GOT Hooking keeps the module resident in the target's memory → detectable,
  // can get a game account banned. Gate ENABLING behind a confirm; disabling is
  // free. Done on `click` + preventDefault (reliable in the KSU webview) — NOT by
  // reverting inside `change`, which could leave the toggle stuck (won't turn on).
  $m('#pfGot').addEventListener('click', e => {
    if (!$m('#pfGot').checked) return;          // turning it off → allow immediately
    e.preventDefault();                          // cancel the toggle until confirmed
    confirm({
      title:   I18N.t('got_warn_title'),
      message: I18N.t('got_warn_msg'),
      okLabel: I18N.t('got_warn_ok'),
      danger:  true,
      link:    { href: 'https://t.me/COPG_chat', label: I18N.t('got_warn_link') },
      onOk:    () => { $m('#pfGot').checked = true; },
    });
  });

  // ⓘ on the cpu/got rows → plain-language explanation. preventDefault +
  // stopPropagation so tapping the icon doesn't toggle the row's checkbox.
  const INFO_KEYS = {
    withcpu:  ['info_withcpu_title',  'info_withcpu_msg'],
    got:      ['info_got_title',      'info_got_msg'],
    dnd:      ['info_dnd_title',      'info_dnd_msg'],
    dab:      ['info_dab_title',      'info_dab_msg'],
    kso:      ['info_kso_title',      'info_kso_msg'],
    nolog:    ['info_nolog_title',    'info_nolog_msg'],
  };
  function openInfo(el, e) {
    e.preventDefault(); e.stopPropagation();    // don't let the row's label toggle the checkbox
    const k = INFO_KEYS[el.dataset.info]; if (!k) return;
    info({ title: I18N.t(k[0]), message: I18N.t(k[1]) });
  }
  $$('#pfToggles .trow__info').forEach(el => {
    el.addEventListener('click', e => openInfo(el, e));
    el.addEventListener('keydown', e => { if (e.key === 'Enter' || e.key === ' ') openInfo(el, e); });
  });

  function openPackage(pkg) {
    const form = $m('#packageForm');
    clearErrors(form);
    $m('#pfError').hidden = true;
    editingPkg = pkg ? { clean: pkg.clean, type: pkg.type, deviceKey: pkg.deviceKey } : null;
    $m('#packageModalTitle').textContent = I18N.t(pkg ? 'pkg_edit_title' : 'pkg_add_title');

    if (pkg) {
      $m('#pfPackage').value = pkg.clean;
      $m('#pfName').value = pkg.name && pkg.name !== pkg.clean ? pkg.name : (COPG.names[pkg.clean] || '');
      setType(pkg.type);
      populateDeviceSelect(pkg.deviceKey);
      $m('#pfWithCpu').checked  = !!pkg.with_cpu;
      $m('#pfGot').checked      = !!pkg.got;
      $m('#pfDnd').checked      = !!pkg.dnd;
      $m('#pfDab').checked      = !!pkg.dab;
      $m('#pfKso').checked      = !!pkg.kso;
      $m('#pfNolog').checked    = !!pkg.nolog;
    } else {
      form.reset();
      setType('device');
      populateDeviceSelect(null);
    }
    openModal('packageModal');
  }

  $m('#packageForm').addEventListener('submit', async e => {
    e.preventDefault();
    const form = e.currentTarget;
    clearErrors(form);
    const errBox = $m('#pfError'); errBox.hidden = true; errBox.textContent = '';

    const pkgEl = $m('#pfPackage');
    const raw = pkgEl.value.trim();
    const type = currentType();
    const devSel = $m('#pfDevice');
    const deviceKey = devSel.value || null;

    let bad = false;
    if (!raw) { markError(pkgEl, I18N.t('msg_required')); bad = true; }
    if (type === 'device' && !deviceKey) { markError(devSel, I18N.t('msg_pick_device')); bad = true; }
    if (bad) return;

    const cleanName = COPG.clean(raw);
    // cross-config duplicate (skip the package being edited)
    const where = COPG.locate(cleanName, editingPkg ? editingPkg.clean : null);
    if (where) {
      errBox.textContent = I18N.t('msg_dup_pkg') + ' ' + where;
      errBox.hidden = false;
      markError(pkgEl, I18N.t('msg_duplicate'));
      return;
    }

    COPG.upsertPackage({
      pkg: raw, name: $m('#pfName').value, type, deviceKey,
      with_cpu: $m('#pfWithCpu').checked,
      got: $m('#pfGot').checked,
      dnd: $m('#pfDnd').checked,
      dab: $m('#pfDab').checked,
      kso: $m('#pfKso').checked,
      nolog: $m('#pfNolog').checked,
    }, editingPkg);

    closeAll();
    await w.LibRefresh(I18N.t(editingPkg ? 'toast_package_saved' : 'toast_package_added'));
  });

  /* ════════════ CONFIRM DELETE ════════════ */
  let confirmCb = null;
  /* Generic confirm sheet. `danger` paints the OK button red (delete);
     otherwise it's the primary accent (restore, and any future confirm). */
  function confirm(opts) {
    $m('#confirmTitle').textContent = opts.title || '';
    $m('#confirmMsg').textContent   = opts.message || '';
    // optional clickable link (e.g. the community group for the GOT warning) —
    // confirmMsg is textContent so it can't hold an anchor; this row carries it.
    const link = $m('#confirmLink');
    if (opts.link && opts.link.href) {
      link.href = opts.link.href;
      link.textContent = opts.link.label || opts.link.href;
      link.hidden = false;
    } else { link.hidden = true; link.removeAttribute('href'); }
    // info-only: a single acknowledge button, no Cancel (pure explanation sheet)
    $m('#confirmCancel').hidden = !!opts.infoOnly;
    const ok = $m('#confirmOk');
    ok.textContent = opts.okLabel || I18N.t('btn_save');
    ok.removeAttribute('data-i18n');           // set explicitly each open → no stale i18n revert
    ok.classList.toggle('btn--danger',  !!opts.danger);
    ok.classList.toggle('btn--primary', !opts.danger);
    confirmCb = opts.onOk || null;
    openModal('confirmModal');
  }
  function confirmDelete(title, message, cb) {
    confirm({ title, message, okLabel: I18N.t('btn_delete'), danger: true, onOk: cb });
  }
  /* Plain-language explanation sheet (reuses the confirm sheet, no Cancel). */
  function info(opts) {
    confirm({ title: opts.title, message: opts.message, okLabel: opts.okLabel || I18N.t('info_ok'), infoOnly: true });
  }
  $m('#confirmOk').addEventListener('click', async () => {
    const cb = confirmCb; confirmCb = null;
    closeModal('confirmModal');                 // close only the confirm, not the sheet underneath
    if (cb) await cb();                          // callback decides whether to close the rest
  });

  /* Pick from installed apps → fills package + display name */
  $m('#pfPickBtn').addEventListener('click', () => {
    if (!w.AppPicker || typeof w.AppPicker.open !== 'function') {
      // warn level → visible even with Debug Logs off, so a missing picker is obvious
      if (w.logEvent) logEvent('App picker unavailable (AppPicker=' + (typeof w.AppPicker) + ').', 'warn');
      showToast(I18N.t('toast_picker_unavailable')); return;
    }
    w.AppPicker.open(app => {
      $m('#pfPackage').value = app.pkg;
      if (app.label) $m('#pfName').value = app.label;
      $m('#pfPackage').closest('.field')?.classList.remove('field--error');
    });
  });

  /* Keep the focused field above the on-screen keyboard */
  ['#deviceForm', '#packageForm'].forEach(sel => {
    const form = $m(sel);
    form?.addEventListener('focusin', e => {
      const t = e.target;
      if (t.matches('input, select, textarea')) {
        setTimeout(() => t.scrollIntoView({ block: 'center', behavior: 'smooth' }), 300);
      }
    });
  });

  /* init */
  wireAndroidSdk();

  w.Modals = { openDevice, openPackage, confirm, confirmDelete, info, closeAll };

})(window);
