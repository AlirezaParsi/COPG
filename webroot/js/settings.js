/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   settings.js — Developer toggles + About card (module info)
   (theme/lang in theme.js)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
const debugToggle = $('#debugToggle');

if (debugToggle) {
  // Restore persisted state into BOTH the checkbox and State (so the Console's
  // debug gate is correct from first paint, not only after a manual toggle).
  try { debugToggle.checked = localStorage.getItem('copg-debug') === '1'; } catch(_) {}
  State.debugLogs = debugToggle.checked;
  debugToggle.addEventListener('change', () => {
    State.debugLogs = debugToggle.checked;
    showToast(I18N.t(debugToggle.checked ? 'toast_debug_on' : 'toast_debug_off'));
    try { localStorage.setItem('copg-debug', debugToggle.checked ? '1' : '0'); } catch(_) {}
    // Always-visible confirmation (console.warn is ungated by the debug gate).
    try {
      console.warn(debugToggle.checked
        ? '[debug] Debug Logs ON — UI activity is now logged here.'
        : '[debug] Debug Logs OFF.');
    } catch(_) {}
  });
}

/* ─── Fullscreen (immersive) toggle ───
   ON = fullscreen (status bar hidden), OFF = status bar shown. Default ON (like
   ReZygisk's WebUI). Persisted as `copg-fullscreen`; applied via the KSU bridge
   (`COPG.setFullscreen` → `ksu.fullScreen`) both now (parse time) and on change.
   No-op in browser preview (no bridge). */
const fsToggle = $('#fullscreenToggle');

if (fsToggle) {
  let on = true; // default fullscreen ON
  try { const v = localStorage.getItem('copg-fullscreen'); if (v !== null) on = v === '1'; } catch(_) {}
  fsToggle.checked = on;
  State.fullscreen = on;
  try { COPG.setFullscreen(on); } catch(_) {}   // apply persisted pref at startup
  fsToggle.addEventListener('change', () => {
    State.fullscreen = fsToggle.checked;
    try { COPG.setFullscreen(fsToggle.checked); } catch(_) {}
    try { localStorage.setItem('copg-fullscreen', fsToggle.checked ? '1' : '0'); } catch(_) {}
    showToast(I18N.t(fsToggle.checked ? 'toast_fullscreen_on' : 'toast_fullscreen_off'));
  });
}

/* ─── Toggle fix + always-on report ───
   The switch (`label.toggle`) is a small tap target on the row's right edge, so
   tapping the row's icon/label did nothing — that's why toggles felt dead. Make
   the WHOLE `.setting-row--toggle` flip its checkbox (taps that land on the
   switch are left to the native checkbox so we don't double-toggle).
   `console.warn` is captured by the in-app Console UNGATED (visible even with
   Debug Logs off), so these lines are an always-on debugging trail. */
function toggleReport(msg) { try { console.warn('[toggle] ' + msg); } catch (_) {} }

$$('.setting-row--toggle').forEach(row => {
  row.addEventListener('click', e => {
    if (e.target.closest('label.toggle')) return;   // tap on the switch → native checkbox handles it
    const cb = row.querySelector('input[type="checkbox"]');
    if (!cb) return;
    cb.checked = !cb.checked;                        // tap on the row text → flip it ourselves
    cb.dispatchEvent(new Event('change', { bubbles: true }));
  });
});

// One ungated line per actual toggle change (visible even with Debug Logs off).
document.addEventListener('change', e => {
  const cb = e.target;
  if (cb && cb.matches && cb.matches('.toggle input[type="checkbox"]'))
    toggleReport((cb.id || '?') + ' = ' + cb.checked);
}, true);

/* ─── About card expand/collapse ─── */
const aboutCard = $('#aboutCard');
aboutCard?.addEventListener('click', () => aboutCard.classList.toggle('expanded'));

/* ─── Populate About rows (module info — real data from module.prop) ─── */
function populateAbout() {
  const si = State.sysInfo;
  const ver = si.version || '';
  const set = (id, val) => { const el = document.getElementById(id); if (el) el.textContent = val; };
  set('aboutVersion',     ver ? 'v' + ver : '—');
  set('aboutVersion2',    ver || '—');
  set('aboutVersionCode', si.versionCode || '—');
  set('aboutAuthor',      si.author || '—');
  // Description stays the localized about_desc (data-i18n) — not module.prop's short text.
}
populateAbout();
