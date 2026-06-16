/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   de.js — Deutsch (German)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
I18N.register('de', {
  _meta: { name: 'Deutsch', flag: '🇩🇪', dir: 'ltr' },

  /* Hero */
  hero_module_id: 'Zygisk-Modul',
  hero_sub:       'Erweitertes Geräte- & App-Spoofing',
  status_active:   'Aktiv',
  status_inactive: 'Inaktiv',

  /* Section labels */
  label_quick_stats: 'Schnellstatistik',
  label_system:      'System',
  label_tools:       'Werkzeuge',
  label_appearance:  'Darstellung',
  label_developer:   'Entwickler',
  label_about:       'Über',

  /* Quick stats */
  stat_devices:  'Geräte',
  stat_packages: 'Pakete',

  /* System rows */
  sys_android: 'Android-Version',
  sys_abi:     'ABI',
  sys_zygisk:  'Zygisk-Variante',
  sys_root_version: 'Root-Version',
  sys_kernel:  'Kernel-Version',

  /* Console */
  console_open:    'Konsole öffnen',
  console_title:   'Konsole',
  console_logcat:  'Logcat',
  console_clear:   'Löschen',
  console_copy:    'Kopieren',
  console_cmd_ph:  'Shell-Befehl eingeben…',
  console_copied:  'In die Zwischenablage kopiert',
  console_copy_fail: 'Kopieren hier nicht verfügbar',
  console_cleared: 'Konsole geleert',
  console_save:      'Speichern',
  console_saved:     'Protokoll in Download/COPG/LOGS gespeichert',
  console_saved_dl:  'Protokoll heruntergeladen',
  console_save_empty:'Nichts zu speichern',
  console_save_fail: 'Protokoll speichern fehlgeschlagen',

  /* Page headers */
  page_library:  'Bibliothek',
  page_settings: 'Einstellungen',

  /* Library tabs + search */
  tab_devices:     'Geräte',
  tab_packages:    'Pakete',
  search_devices:  'Geräte suchen…',
  search_packages: 'Pakete suchen…',
  empty_devices:   'Keine Geräte gefunden',
  empty_packages:  'Keine Pakete gefunden',

  /* Settings rows */
  set_theme:        'Design',
  set_language:     'Sprache',
  set_fullscreen:   'Vollbild',
  set_debug:        'Debug-Protokolle',
  set_refresh_icons:     'App-Icons aktualisieren',
  set_refresh_icons_sub: 'Paket-Icons neu herunterladen',

  /* Theme hints */
  theme_auto:   'Auto',
  theme_dark:   'Dunkel',
  theme_light:  'Hell',
  theme_amoled: 'AMOLED',

  /* Sheets */
  sheet_theme_title: 'Design',
  sheet_lang_title:  'Sprache',
  theme_opt_auto:   'Auto (System)',
  theme_opt_dark:   'Dunkel',
  theme_opt_light:  'Hell',
  theme_opt_amoled: 'AMOLED-Schwarz',

  /* About */
  about_module:  'COPG-Modul',
  about_version: 'Version',
  about_build:   'Build-Datum',
  about_vcode:   'Versionscode',
  about_author:  'Autor',
  about_license: 'Lizenz',
  about_desc:    'Ein Zygisk-Modul, das dein Gerät — und optional dessen CPU — pro Spiel oder App vortäuscht und so höhere FPS, bessere Grafik und Premium-Funktionen freischaltet, die sonst bestimmten Handys vorbehalten sind. Apps jederzeit über diese WebUI hinzufügen oder entfernen, ohne Neustart.',

  /* Nav */
  nav_home:     'Start',
  nav_library:  'Bibliothek',
  nav_settings: 'Einstellungen',

  /* Toasts */
  toast_console:   'Konsole — demnächst',
  toast_debug_on:  'Debug-Protokolle aktiviert',
  toast_debug_off: 'Debug-Protokolle deaktiviert',
  toast_fullscreen_on:  'Vollbild an — Statusleiste ausgeblendet',
  toast_fullscreen_off: 'Vollbild aus — Statusleiste sichtbar',
  toast_icons_refreshing:      'App-Icons werden aktualisiert…',
  toast_icons_refreshed:       'App-Icons aktualisiert',
  toast_icons_refresh_preview: 'Icon-Aktualisierung funktioniert nur auf dem Gerät',
  status_connected: 'Verbunden',
  status_offline:   'Offline',

  /* Library — add buttons & card meta */
  add_device:   'Gerät hinzufügen',
  add_package:  'Paket hinzufügen',
  add_short:    'Hinzufügen',
  act_edit:     'Bearbeiten',
  act_delete:   'Löschen',
  dev_model:    'Modell',
  dev_games:    'Spiele',
  dev_apps:     'Apps',
  badge_installed: 'Installiert',
  preview_unsaved: 'Vorschaumodus — Änderungen werden nicht auf dem Gerät gespeichert.',

  /* System — root */
  sys_root: 'Root',

  /* Sort + filter */
  sort_title:     'Sortieren nach',
  sort_default:   'Standard',
  sort_name:      'Name (A→Z)',
  sort_name_za:   'Name (Z→A)',
  sort_apps:      'App-Anzahl',
  sort_brand:     'Marke',
  sort_device:    'Gerät',
  sort_type:      'Typ',
  sort_installed: 'Installierte zuerst',
  filter_all:      'Alle',
  filter_user:     'Benutzer',
  filter_system:   'System',
  filter_installed:'Installiert',
  filter_blocked:  'Blockiert',
  filter_cpu_only: 'CPU',

  /* App picker */
  pick_from_installed: 'Aus installierten Apps wählen',
  picker_title:   'Installierte Apps',
  picker_search:  'Apps suchen…',
  picker_empty:   'Keine installierten Apps gefunden',
  picker_no_results: 'Keine Apps gefunden',
  picker_preview: 'Vorschaumodus — auf dem Gerät installieren, um Apps aufzulisten',
  toast_picker_unavailable: 'App-Auswahl hier nicht verfügbar',
  devpicker_title:  'Geräteprofile',
  devpicker_search: 'Geräte suchen…',
  devpicker_empty:  'Keine Geräte — zuerst eines hinzufügen',

  /* Backup & Sync */
  lib_backup:           'Sicherung & Sync',
  backup_title:         'Sicherung & Sync',
  backup_intro:         'Sichere deine Geräte & Pakete, stelle eine frühere Sicherung wieder her oder synchronisiere die neueste Konfiguration von GitHub.',
  backup_export:        'Sicherung exportieren',
  backup_export_sub:    'Aktuelle Daten in Downloads speichern',
  backup_import:        'Importieren & wiederherstellen',
  backup_import_sub:    'Daten aus einer Sicherung wiederherstellen',
  backup_empty:         'Keine Sicherungen in Downloads/COPG gefunden',
  backup_from_file:     'Aus Datei importieren…',
  backup_confirm_title: 'Sicherung wiederherstellen?',
  backup_confirm_msg:   'Dadurch werden deine aktuellen Geräte & Pakete überschrieben mit:',
  backup_restore:       'Wiederherstellen',
  toast_backup_ok:      'Sicherung gespeichert',
  toast_backup_fail:    'Sicherung fehlgeschlagen',
  toast_restore_ok:     'Sicherung wiederhergestellt',
  toast_restore_fail:   'Wiederherstellung fehlgeschlagen',
  toast_invalid_json:   'Keine gültige JSON-Datei',

  /* Import type chooser */
  backup_kind_title:       'Welche Art Datei ist das?',
  backup_kind_config:      'Geräte & Pakete',
  backup_kind_config_sub:  'Geräte- & Paketkonfiguration (COPG.json)',
  backup_kind_list:        'Anzeigenamen',
  backup_kind_list_sub:    'Anzeigenamen der Pakete (list.json)',
  backup_kind_detected:    'erkannt',

  /* Sync from GitHub */
  backup_sync:        'Von GitHub synchronisieren',
  backup_sync_sub:    'Neueste Konfiguration aus dem Repo abrufen',
  toast_sync_ok:      'Von GitHub synchronisiert',
  toast_sync_fail:    'Sync fehlgeschlagen — prüfe deine Verbindung',
  toast_sync_preview: 'Sync funktioniert nur auf dem Gerät',

  /* Package types (chips) */
  pkgtype_device:   'Gerät',
  pkgtype_cpu_only: 'CPU-Spoof',
  pkgtype_blocked:  'CPU-Spoof blockieren',

  /* Package tags (chips) */
  tag_withcpu:  'Mit CPU',
  tag_cow:      'COW',
  tag_dnd:      'Nicht stören',
  tag_dab:      'Auto-Helligkeit',
  tag_kso:      'Bildschirm an',
  tag_nolog:    'Kein Log',

  /* Device modal */
  dev_add_title:      'Gerät hinzufügen',
  dev_edit_title:     'Gerät bearbeiten',
  dev_f_name:         'Gerätename',
  dev_f_brand:        'Marke',
  dev_f_model:        'Modell',
  dev_f_manufacturer: 'Hersteller',
  dev_f_fingerprint:  'Fingerprint',
  dev_f_android:      'Android-Version',
  dev_f_sdk:          'SDK Int',
  dev_f_serial:       'Seriennummer',
  dev_f_androidid:    'Android-ID',
  opt_optional:       '(optional)',
  serial_gen:         'Generieren',
  info_androidid_title: 'Android-ID fälschen',
  info_androidid_msg: 'Lässt Apps dieses Geräteprofils eine gefälschte ANDROID_ID lesen (die Kennung aus Settings.Secure). 16 Hex-Zeichen. Das Modul entlädt sich vor dem App-Start, sodass nichts von COPG im Speicher der App bleibt. Leer lassen, um die echte ID zu behalten. Hinweis: Eine entschlossene App kann die echte ID anders auslesen — dies ändert nur die Standard-Settings-Abfrage.',
  info_sdk_title:     'SDK Int',
  info_sdk_msg:       'SDK-Level und Android-Version gehören zusammen — es wird automatisch aus der Android-Version ausgefüllt. Wenn du ein nicht passendes SDK einstellst, erkennen manche Apps die Abweichung und stürzen ab. Bearbeite es nur von Hand für eine brandneue Android-Version, die das Modul noch nicht kennt.',

  /* Package modal */
  pkg_add_title:  'Paket hinzufügen',
  pkg_edit_title: 'Paket bearbeiten',
  pkg_f_package:  'Paketname',
  pkg_f_name:     'Anzeigename',
  pkg_f_type:     'Typ',
  pkg_f_device:   'Geräteprofil',
  pkg_pick_device: 'Gerät auswählen…',
  pkg_no_devices: 'Keine Geräte — zuerst eins hinzufügen',
  pkg_sec_spoofing: 'Spoofing',
  pkg_sec_tweaks:   'Optimierungen',
  pkg_t_withcpu:  'Mit CPU-Spoofing',
  pkg_t_cow:      'COW-Prop-Spoof',
  pkg_t_dnd:      'Nicht stören',
  pkg_t_dab:      'Auto-Helligkeit deaktivieren',
  pkg_t_kso:      'Bildschirm anlassen',
  pkg_t_nolog:    'Protokollierung deaktivieren',


  /* Toggle info popups */
  info_aria:           'Was ist das?',
  info_ok:             'Verstanden',
  info_withcpu_title:  'Mit CPU-Spoofing',
  info_withcpu_msg:    'Zusätzlich zum Vortäuschen des Gerätemodells werden auch die Prozessor-Details (CPU) deines Telefons für diese App vorgetäuscht. Aktiviere es für Spiele, die den Chipsatz prüfen. Lass es aus (Standard), dann bleibt das CPU-Spoofing blockiert — was Banking- und sensible Apps bevorzugen.',
  info_cow_title:      'COW-Prop-Spoof',
  info_cow_msg:        'Fälscht die System-Properties des Geräts (ro.product.*, ro.build.fingerprint …) für diese App, sodass selbst ihr nativer Code das gefälschte Gerät sieht. Stealth: Die Werte werden in eine private Copy-on-Write-Ansicht des Property-Bereichs geschrieben, und das Modul entlädt sich vor dem App-Start — nichts von COPG bleibt im Speicher der App, sicher gegen Anti-Cheat-Speicherscans. Für Spiele, die Properties nativ lesen.',
  info_dnd_title:      'Nicht stören',
  info_dnd_msg:        'Schaltet Anrufe und Benachrichtigungen stumm, während du in diesem Spiel bist, damit nichts dein Match unterbricht. Deine normale Einstellung kehrt beim Verlassen zurück.',
  info_dab_title:      'Auto-Helligkeit deaktivieren',
  info_dab_msg:        'Verhindert, dass sich der Bildschirm beim Spielen automatisch abdunkelt und aufhellt, sodass die Helligkeit bleibt, wie du sie eingestellt hast. Wird beim Verlassen des Spiels automatisch wiederhergestellt.',
  info_kso_title:      'Bildschirm anlassen',
  info_kso_msg:        'Hält den Bildschirm wach, während dieses Spiel geöffnet ist, auch wenn du es nicht berührst — kein Bildschirm-Aus mitten im Match mehr. Dein normales Timeout kehrt beim Verlassen zurück.',
  info_nolog_title:    'Protokollierung deaktivieren',
  info_nolog_msg:      'Pausiert den System-Protokollsammler beim Spielen, um etwas CPU und Speicher freizugeben. Schaltet sich beim Verlassen des Spiels von selbst wieder ein.',

  /* Buttons */
  btn_cancel: 'Abbrechen',
  btn_save:   'Speichern',
  btn_delete: 'Löschen',

  /* Validation messages */
  msg_required:    'Erforderlich',
  msg_duplicate:   'Duplikat',
  msg_dup_device:  'Ein Gerät mit diesem Namen existiert bereits',
  msg_dup_model:   'Ein Gerät mit diesem Modell existiert bereits',
  msg_pick_device: 'Gerät auswählen',
  msg_dup_pkg:     'Dieses Paket existiert bereits in',

  /* Confirm delete */
  confirm_del_device_title:  'Gerät löschen?',
  confirm_del_package_title: 'Paket löschen?',

  /* Toasts — CRUD */
  toast_device_added:    'Gerät hinzugefügt',
  toast_device_saved:    'Gerät gespeichert',
  toast_device_deleted:  'Gerät gelöscht',
  toast_package_added:   'Paket hinzugefügt',
  toast_package_saved:   'Paket gespeichert',
  toast_package_deleted: 'Paket gelöscht',
  toast_preview_only:    'Nur Vorschau — nicht auf dem Gerät gespeichert',
  toast_save_failed:     'Speichern fehlgeschlagen',
});
