/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   icons.js — App icon loader (3-tier, lazy)
   For a package, resolve an icon in order:
     1) installed app icon via the KSU bridge   (ksu://icon/<pkg>)
     2) localStorage cache (incl. earlier Play fetches)
     3) best-effort Play Store scrape (COPG.fetchPlayIcon) → cache
     4) deterministic generated SVG (hash → colour + initial)
   Loading is deferred until the host element scrolls into view.
   window.Icons.load(el, pkg, label)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
(function (w) {
  const CACHE_PREFIX = 'copg-icon:';
  const memCache = new Map();   // pkg -> data URI ('' = known-miss)

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
  function genSVG(pkg, label) {
    const [c1, c2] = PALETTE[hash(pkg || label || '?') % PALETTE.length];
    const ch = initialOf(label, pkg);
    const id = 'g' + (hash(pkg) % 100000);
    const svg =
      `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 40 40">` +
      `<defs><linearGradient id="${id}" x1="0" y1="0" x2="1" y2="1">` +
      `<stop offset="0" stop-color="${c1}"/><stop offset="1" stop-color="${c2}"/></linearGradient></defs>` +
      `<rect width="40" height="40" rx="11" fill="url(#${id})"/>` +
      `<text x="20" y="21" font-family="system-ui,sans-serif" font-size="19" font-weight="700" ` +
      `fill="#0A0A12" fill-opacity="0.85" text-anchor="middle" dominant-baseline="central">${ch}</text></svg>`;
    return 'data:image/svg+xml;charset=utf-8,' + encodeURIComponent(svg);
  }

  function cacheGet(pkg) {
    if (memCache.has(pkg)) return memCache.get(pkg);
    try { const v = localStorage.getItem(CACHE_PREFIX + pkg); if (v) { memCache.set(pkg, v); return v; } } catch (_) {}
    return undefined;
  }
  function cacheSet(pkg, uri) {
    memCache.set(pkg, uri);
    try { if (uri) localStorage.setItem(CACHE_PREFIX + pkg, uri); } catch (_) {}
  }

  function paint(el, uri) {
    const img = new Image();
    img.alt = '';
    img.decoding = 'async';
    img.className = 'app-icon-img';
    img.style.opacity = '0';
    img.onload = () => { img.style.opacity = '1'; };
    img.src = uri;
    el.innerHTML = '';
    el.appendChild(img);
  }

  /* Try the installed-app icon (bridge). Resolves to data/uri or null. */
  function tryInstalledIcon(pkg) {
    return new Promise(resolve => {
      if (typeof ksu === 'undefined' && typeof $packageManager === 'undefined') return resolve(null);
      // KSU exposes installed icons through the ksu://icon/<pkg> scheme.
      const probe = new Image();
      let done = false;
      const finish = v => { if (!done) { done = true; resolve(v); } };
      probe.onload = () => finish(`ksu://icon/${pkg}`);
      probe.onerror = () => finish(null);
      probe.src = `ksu://icon/${pkg}`;
      setTimeout(() => finish(null), 2500);
    });
  }

  /* ── Serial network queue: at most ONE Play/fallback download at a time,
        with a small gap between jobs, so scrolling never floods the bridge
        with curl calls (that lagged the UI + RAM). Cheap tiers skip it. ── */
  const netQueue = [];
  let netBusy = false;
  const NET_GAP = 250;   // ms between downloads

  function enqueueNet(job) {
    return new Promise(resolve => {
      netQueue.push({ job, resolve });
      pumpNet();
    });
  }
  async function pumpNet() {
    if (netBusy) return;
    const next = netQueue.shift();
    if (!next) return;
    netBusy = true;
    let result = null;
    try { result = await next.job(); } catch (_) {}
    next.resolve(result);
    setTimeout(() => { netBusy = false; pumpNet(); }, NET_GAP);
  }

  async function resolve(el, pkg, label) {
    // 2) cache first (covers prior installed + downloaded results)
    const cached = cacheGet(pkg);
    if (cached !== undefined) { paint(el, cached || genSVG(pkg, label)); return; }

    // 1) installed icon (cheap, immediate — no queue)
    const installed = await tryInstalledIcon(pkg);
    if (installed) { paint(el, installed); return; }   // live scheme, not cached

    // 3) network fallbacks, THROTTLED through the serial queue, then cached.
    //    Order: Play Store → APKMirror/F-Droid (COPG.fetchAppIcon handles both).
    let downloaded = null;
    try {
      const set = (w.COPG && COPG.getInstalledPackages) ? await COPG.getInstalledPackages() : new Set();
      if (!set.has(pkg) && w.COPG && (COPG.fetchAppIcon || COPG.fetchPlayIcon)) {
        downloaded = await enqueueNet(() =>
          COPG.fetchAppIcon ? COPG.fetchAppIcon(pkg) : COPG.fetchPlayIcon(pkg));
      }
    } catch (_) {}
    if (downloaded) { cacheSet(pkg, downloaded); paint(el, downloaded); return; }

    // 4) generated SVG (remember the miss so we don't refetch each scroll)
    cacheSet(pkg, '');
    paint(el, genSVG(pkg, label));
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
    // instant placeholder so layout never flashes empty
    if (!el.firstChild) paint(el, genSVG(pkg, label));
    ensureObserver().observe(el);
  }

  w.Icons = { load, genSVG };
})(window);
