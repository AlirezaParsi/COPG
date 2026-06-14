/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   icons.js — App icon loader (lazy, local-file based, single-batch download)
   The lag was ONE ksu.exec (ksud fork) per icon. So we do ZERO per-icon bridge
   calls: instead every missing, non-installed icon is collected and ONE detached
   background shell (COPG.batchFetchIcons) downloads them all to
   webroot/icons/<pkg>.png. The UI only ever shows:
     • installed apps  → <img src="ksu://icon/<pkg>">  (bridge scheme, instant)
     • non-installed    → <img src="icons/<pkg>.png">    (local file the batch fills)
     • anything missing → inline <svg> fallback (data: URIs are CSP-blocked, so a
       data:image/svg fallback would render broken — inline DOM is immune)
   As the background batch fills files, a few timed retries upgrade SVG→real icon.
   Fully offline once files exist; covers user-added packages; no UI jank.
   window.Icons.load(el, pkg, label)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
(function (w) {
  /* ── deterministic colour + initial fallback ── */
  const PALETTE = [
    ['#818CF8', '#A78BFA'], ['#34D399', '#10B981'], ['#67E8F9', '#22D3EE'],
    ['#F472B6', '#EC4899'], ['#FBBF24', '#F59E0B'], ['#F87171', '#EF4444'],
    ['#A3E635', '#84CC16'], ['#60A5FA', '#3B82F6'],
  ];
  function hash(str) {
    let h = 0;
    for (let i = 0; i < str.length; i++) { h = (h << 5) - h + str.charCodeAt(i); h |= 0; }
    return Math.abs(h);
  }
  function initialOf(label, pkg) {
    const s = (label || '').trim() || (pkg || '').split('.').filter(Boolean).pop() || '?';
    const ch = s.replace(/[^\p{L}\p{N}]/u, '').charAt(0) || s.charAt(0) || '?';
    return ch.toUpperCase();
  }
  function escapeXml(s) {
    return String(s).replace(/[<>&]/g, c => (c === '<' ? '&lt;' : c === '>' ? '&gt;' : '&amp;'));
  }
  /* Raw <svg> markup for INLINE DOM injection (NOT a data: URI — CSP-safe). */
  function genSVGMarkup(pkg, label) {
    const [c1, c2] = PALETTE[hash(pkg || label || '?') % PALETTE.length];
    const ch = escapeXml(initialOf(label, pkg));
    const id = 'g' + (hash(pkg || label || '?') % 100000);
    return (
      `<svg class="app-icon-img" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 40 40">` +
      `<defs><linearGradient id="${id}" x1="0" y1="0" x2="1" y2="1">` +
      `<stop offset="0" stop-color="${c1}"/><stop offset="1" stop-color="${c2}"/></linearGradient></defs>` +
      `<rect width="40" height="40" rx="11" fill="url(#${id})"/>` +
      `<text x="20" y="21" font-family="system-ui,sans-serif" font-size="19" font-weight="700" ` +
      `fill="#0A0A12" fill-opacity="0.85" text-anchor="middle" dominant-baseline="central">${ch}</text></svg>`
    );
  }

  function safeName(pkg) { return String(pkg).replace(/[^A-Za-z0-9._-]/g, '_'); }
  function localUrl(pkg, bust) { return 'icons/' + safeName(pkg) + '.png' + (bust ? ('?v=' + bust) : ''); }

  /* Inline <svg> fallback — CSP-safe, always renders. */
  function paintSVG(el, pkg, label) { el.innerHTML = genSVGMarkup(pkg, label); }

  /* <img src> for an icon URL/path; broken load → inline SVG (never a broken img). */
  function paintImg(el, uri, pkg, label) {
    const img = new Image();
    img.alt = '';
    img.decoding = 'async';
    img.className = 'app-icon-img';
    img.style.opacity = '0';
    img.onload = () => { img.style.opacity = '1'; };
    img.onerror = () => { paintSVG(el, pkg, label); };
    img.src = uri;
    el.innerHTML = '';
    el.appendChild(img);
  }

  /* ── Batch download coordination ──
     Non-installed rows whose local file isn't there yet register as "watchers"
     and enqueue their pkg. A debounce fires ONE COPG.batchFetchIcons for the
     whole set; timed retries then re-attempt the local file as the worker fills
     them (no extra bridge polling). ── */
  const WATCH = new Map();        // pkg -> { el, label } rows on SVG awaiting a file
  const PENDING = new Set();      // pkgs queued for the next batch
  let batchTimer = null;
  const RETRY_AT = [5000, 13000, 25000, 42000, 65000];  // ms after a batch fires

  function bridgeReady() {
    return !!(w.COPG && COPG.batchFetchIcons && COPG.hasBridge && COPG.hasBridge());
  }
  function enqueueBatch(pkg) {
    if (!bridgeReady()) return;
    PENDING.add(pkg);
    clearTimeout(batchTimer);
    batchTimer = setTimeout(flushBatch, 1200);
  }
  function flushBatch() {
    const pkgs = [...PENDING]; PENDING.clear();
    if (!pkgs.length || !bridgeReady()) return;
    COPG.batchFetchIcons(pkgs);                  // fire-and-forget, returns fast
    RETRY_AT.forEach(t => setTimeout(retryWatchers, t));
  }
  function retryWatchers() {
    if (!WATCH.size) return;
    const bust = Date.now();
    WATCH.forEach((row, pkg) => tryLocal(row.el, pkg, row.label, bust));
  }

  /* Attempt the local file; success paints it (+clears watcher), failure shows
     the SVG fallback, registers a watcher, and enqueues the pkg for the batch. */
  function tryLocal(el, pkg, label, bust) {
    const img = new Image();
    img.alt = '';
    img.decoding = 'async';
    img.className = 'app-icon-img';
    img.style.opacity = '0';
    img.onload = () => {
      img.style.opacity = '1';
      el.innerHTML = ''; el.appendChild(img);
      WATCH.delete(pkg);
    };
    img.onerror = () => {
      if (!el.firstChild) paintSVG(el, pkg, label);
      else if (!el.querySelector('svg')) paintSVG(el, pkg, label);
      WATCH.set(pkg, { el, label });
      enqueueBatch(pkg);
    };
    img.src = localUrl(pkg, bust);
  }

  async function resolve(el, pkg, label) {
    // installed apps → bridge icon scheme (instant, always current)
    let installed = false;
    try {
      const set = (w.COPG && COPG.getInstalledPackages) ? await COPG.getInstalledPackages() : null;
      installed = !!(set && set.has(pkg));
    } catch (_) {}
    if (installed) { paintImg(el, `ksu://icon/${pkg}`, pkg, label); return; }
    // non-installed → local file (batch fills it); onerror → SVG + queue batch
    tryLocal(el, pkg, label);
  }

  /* Shared lazy observer */
  let observer = null;
  function ensureObserver() {
    if (observer) return observer;
    observer = new IntersectionObserver(entries => {
      entries.forEach(e => {
        if (!e.isIntersecting) return;
        const el = e.target;
        observer.unobserve(el);
        resolve(el, el.dataset.pkg, el.dataset.label || '');
      });
    }, { rootMargin: '200px', threshold: 0.01 });
    return observer;
  }

  function load(el, pkg, label) {
    if (!el) return;
    el.dataset.pkg = pkg;
    el.dataset.label = label || '';
    // instant inline-SVG placeholder so layout never flashes empty
    if (!el.firstChild) paintSVG(el, pkg, label);
    ensureObserver().observe(el);
  }

  w.Icons = { load, genSVGMarkup };
})(window);
