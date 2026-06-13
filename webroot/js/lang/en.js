/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   en.js — English (reference language)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
I18N.register('en', {
  _meta: { name: 'English', flag: '🇬🇧', dir: 'ltr' },

  /* Hero */
  hero_module_id: 'Zygisk Module',
  hero_sub:       'Advanced Device & App Spoofing',
  status_active:   'Active',
  status_inactive: 'Inactive',

  /* Section labels */
  label_quick_stats: 'Quick Stats',
  label_system:      'System',
  label_tools:       'Tools',
  label_appearance:  'Appearance',
  label_developer:   'Developer',
  label_about:       'About',

  /* Quick stats */
  stat_devices:  'Devices',
  stat_packages: 'Packages',

  /* System rows */
  sys_android: 'Android Version',
  sys_abi:     'ABI',
  sys_zygisk:  'Zygisk Variant',
  sys_root_version: 'Root Version',
  sys_kernel:  'Kernel Version',

  /* Console */
  console_open:    'Open Console',
  console_title:   'Console',
  console_logcat:  'Logcat',
  console_clear:   'Clear',
  console_copy:    'Copy',
  console_cmd_ph:  'Type a shell command…',
  console_copied:  'Copied to clipboard',
  console_copy_fail: 'Copy not available here',
  console_cleared: 'Console cleared',
  console_save:      'Save',
  console_saved:     'Log saved to Download/COPG/LOGS',
  console_saved_dl:  'Log downloaded',
  console_save_empty:'Nothing to save',
  console_save_fail: 'Saving log failed',

  /* Page headers */
  page_library:  'Library',
  page_settings: 'Settings',

  /* Library tabs + search */
  tab_devices:     'Devices',
  tab_packages:    'Packages',
  search_devices:  'Search devices…',
  search_packages: 'Search packages…',
  empty_devices:   'No devices found',
  empty_packages:  'No packages found',

  /* Settings rows */
  set_theme:        'Theme',
  set_language:     'Language',
  set_debug:        'Debug Logs',

  /* Theme hints (settings row) */
  theme_auto:   'Auto',
  theme_dark:   'Dark',
  theme_light:  'Light',
  theme_amoled: 'AMOLED',

  /* Sheets */
  sheet_theme_title: 'Theme',
  sheet_lang_title:  'Language',
  theme_opt_auto:   'Auto (System)',
  theme_opt_dark:   'Dark',
  theme_opt_light:  'Light',
  theme_opt_amoled: 'AMOLED Black',

  /* About (module info) */
  about_module:  'COPG Module',
  about_version: 'Version',
  about_build:   'Build Date',
  about_vcode:   'Version Code',
  about_author:  'Author',
  about_license: 'License',
  about_desc:    'A Zygisk module that spoofs your device — and optionally its CPU — per game or app, unlocking higher FPS, better graphics, and premium features otherwise locked to specific phones. Add or remove apps anytime from this WebUI, no reboot needed.',

  /* Nav */
  nav_home:     'Home',
  nav_library:  'Library',
  nav_settings: 'Settings',

  /* Toasts */
  toast_console:   'Console — coming soon',
  toast_debug_on:  'Debug logs enabled',
  toast_debug_off: 'Debug logs disabled',
  status_connected: 'Connected',
  status_offline:   'Offline',

  /* Library — add buttons & card meta */
  add_device:   'Add Device',
  add_package:  'Add Package',
  add_short:    'Add',
  act_edit:     'Edit',
  act_delete:   'Delete',
  dev_model:    'Model',
  dev_games:    'games',
  dev_apps:     'apps',
  badge_installed: 'Installed',
  preview_unsaved: 'Preview mode — changes are not saved to the device.',

  /* System — root */
  sys_root: 'Root',

  /* Sort + filter */
  sort_title:     'Sort by',
  sort_default:   'Default',
  sort_name:      'Name (A→Z)',
  sort_name_za:   'Name (Z→A)',
  sort_apps:      'App count',
  sort_brand:     'Brand',
  sort_device:    'Device',
  sort_type:      'Type',
  sort_installed: 'Installed first',
  filter_all:      'All',
  filter_user:     'User',
  filter_system:   'System',
  filter_installed:'Installed',
  filter_blocked:  'Blocked',
  filter_cpu_only: 'CPU',

  /* App picker */
  pick_from_installed: 'Pick from installed apps',
  picker_title:   'Installed Apps',
  picker_search:  'Search apps…',
  picker_empty:   'No installed apps found',
  picker_no_results: 'No apps found',
  picker_preview: 'Preview mode — install on device to list apps',
  toast_picker_unavailable: 'App picker unavailable here',

  /* Backup & Sync */
  lib_backup:           'Backup & Sync',
  backup_title:         'Backup & Sync',
  backup_intro:         'Back up your devices & packages, restore a previous backup, or sync the latest config from GitHub.',
  backup_export:        'Export backup',
  backup_export_sub:    'Save current data to Downloads',
  backup_import:        'Import & restore',
  backup_import_sub:    'Restore data from a backup',
  backup_empty:         'No backups found in Downloads/COPG',
  backup_from_file:     'Import from file…',
  backup_confirm_title: 'Restore backup?',
  backup_confirm_msg:   'This will overwrite your current devices & packages with:',
  backup_restore:       'Restore',
  toast_backup_ok:      'Backup saved',
  toast_backup_fail:    'Backup failed',
  toast_restore_ok:     'Backup restored',
  toast_restore_fail:   'Restore failed',
  toast_invalid_json:   'Not a valid JSON file',

  /* Import type chooser */
  backup_kind_title:       'What kind of file is this?',
  backup_kind_config:      'Devices & Packages',
  backup_kind_config_sub:  'Device & package config (COPG.json)',
  backup_kind_list:        'Display Names',
  backup_kind_list_sub:    'Package display names (list.json)',
  backup_kind_detected:    'detected',

  /* Sync from GitHub */
  backup_sync:        'Sync from GitHub',
  backup_sync_sub:    'Fetch the latest config from the repo',
  toast_sync_ok:      'Synced from GitHub',
  toast_sync_fail:    'Sync failed — check your connection',
  toast_sync_preview: 'Sync only works on the device',

  /* Package types (chips) */
  pkgtype_device:   'Device',
  pkgtype_cpu_only: 'CPU Spoof',
  pkgtype_blocked:  'Block CPU Spoof',

  /* Package tags (chips) */
  tag_withcpu:  'With CPU',
  tag_got:      'GOT',
  tag_dnd:      'DND',
  tag_dab:      'Auto-Bright',
  tag_kso:      'Screen On',
  tag_nolog:    'No Log',

  /* Device modal */
  dev_add_title:      'Add Device',
  dev_edit_title:     'Edit Device',
  dev_f_name:         'Device Name',
  dev_f_brand:        'Brand',
  dev_f_model:        'Model',
  dev_f_manufacturer: 'Manufacturer',
  dev_f_fingerprint:  'Fingerprint',
  dev_f_android:      'Android Version',
  dev_f_sdk:          'SDK Int',

  /* Package modal */
  pkg_add_title:  'Add Package',
  pkg_edit_title: 'Edit Package',
  pkg_f_package:  'Package Name',
  pkg_f_name:     'Display Name',
  pkg_f_type:     'Type',
  pkg_f_device:   'Device Profile',
  pkg_no_devices: 'No devices — add one first',
  pkg_sec_spoofing: 'Spoofing',
  pkg_sec_tweaks:   'Tweaks',
  pkg_t_withcpu:  'With CPU Spoofing',
  pkg_t_got:      'GOT Hooking',
  pkg_t_dnd:      'Do Not Disturb',
  pkg_t_dab:      'Disable Auto-Brightness',
  pkg_t_kso:      'Keep Screen On',
  pkg_t_nolog:    'Disable Logging',

  /* GOT Hooking warning */
  got_warn_title: 'Enable GOT Hooking?',
  got_warn_msg:   'GOT Hooking keeps the module resident in the app or game’s memory. This can be detected and may even get your game account banned.\n\nAre you sure you want to enable it? We recommend asking the community before deciding.',
  got_warn_ok:    'Enable anyway',
  got_warn_link:  'COPG community on Telegram',

  /* Toggle info popups (the ⓘ buttons) */
  info_aria:           'What’s this?',
  info_ok:             'Got it',
  info_withcpu_title:  'With CPU Spoofing',
  info_withcpu_msg:    'On top of faking the device model, this also fakes your phone’s processor (CPU) details for this app. Turn it on for games that check the chipset. Leave it off (the default) and the CPU fake stays blocked — which banking and sensitive apps prefer.',
  info_got_title:      'GOT Hooking',
  info_got_msg:        'A deeper, more aggressive way to apply the spoof from inside the app’s memory. Stronger, but riskier: it can be detected and may even get a game account banned. Only enable it if you know you need it.',
  info_dnd_title:      'Do Not Disturb',
  info_dnd_msg:        'Silences calls and notifications while you’re in this game, so nothing interrupts your match. Your normal setting comes back when you leave.',
  info_dab_title:      'Disable Auto-Brightness',
  info_dab_msg:        'Stops the screen from auto-dimming and brightening while you play, so the brightness stays where you set it. Restored automatically when you leave the game.',
  info_kso_title:      'Keep Screen On',
  info_kso_msg:        'Keeps the screen awake while this game is open, even if you don’t touch it — no more screen-off mid-match. Your normal timeout returns when you leave.',
  info_nolog_title:    'Disable Logging',
  info_nolog_msg:      'Pauses the system log collector while you game to free up a little CPU and storage. It turns back on by itself when you leave the game.',

  /* Buttons */
  btn_cancel: 'Cancel',
  btn_save:   'Save',
  btn_delete: 'Delete',

  /* Validation messages */
  msg_required:    'Required',
  msg_duplicate:   'Duplicate',
  msg_dup_device:  'A device with this name already exists',
  msg_dup_model:   'A device with this model already exists',
  msg_pick_device: 'Select a device',
  msg_dup_pkg:     'This package already exists in',

  /* Confirm delete */
  confirm_del_device_title:  'Delete Device?',
  confirm_del_package_title: 'Delete Package?',

  /* Toasts — CRUD */
  toast_device_added:    'Device added',
  toast_device_saved:    'Device saved',
  toast_device_deleted:  'Device deleted',
  toast_package_added:   'Package added',
  toast_package_saved:   'Package saved',
  toast_package_deleted: 'Package deleted',
  toast_preview_only:    'Preview only — not saved to device',
  toast_save_failed:     'Save failed',
});
