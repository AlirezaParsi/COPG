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
    // This line is always shown (it's the user turning the feature on/off).
    if (window.Log) Log[debugToggle.checked ? 'success' : 'warn'](
      'Debug Logs ' + (debugToggle.checked ? 'enabled — UI activity will appear here.' : 'disabled.'));
  });
}

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
  // Real module description (overrides the generic localized fallback)
  const desc = document.querySelector('.about-desc');
  if (desc && si.description) { desc.textContent = si.description; desc.removeAttribute('data-i18n'); }
}
populateAbout();
