/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   state.js — App state & data
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
const State = {
  activePage:   'home',
  activeLibTab: 'devices',
  theme:        'dark',
  language:     'en',
  moduleActive: true,
  rootEnv:      '—',
  rootSecondary: [],
  debugLogs:    false,
  experimental: false,

  /* devices & packages now come from COPG.json via js/copg-data.js
     (COPG.listDevices() / COPG.listPackages()) — no fake data here. */

  /* sysInfo holds ONLY neutral placeholders — never fake values. Every field is
     overwritten with the REAL on-device data by boot.js (COPG.getSystemInfo(),
     which reads getprop / uname -r / /data/adb/modules/COPG/module.prop). A
     pending or preview-mode read shows "—", which is honest, not invented data. */
  sysInfo: {
    androidVer: '—',
    abi:        '—',
    zygisk:     '—',
    rootVersionLabel: '—',
    rootVersion: '',
    kernel:     '—',
    version:    '',
    versionCode: '',
    author:     '—',
    description: 'A Zygisk module that spoofs your device — and optionally its CPU — per game or app.',
  },
};
