/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   id.js — Bahasa Indonesia (Indonesian)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
I18N.register('id', {
  _meta: { name: 'Indonesia', flag: '🇮🇩', dir: 'ltr' },

  /* Hero */
  hero_module_id: 'Modul Zygisk',
  hero_sub:       'Pemalsuan Perangkat & Aplikasi Tingkat Lanjut',
  status_active:   'Aktif',
  status_inactive: 'Nonaktif',

  /* Section labels */
  label_quick_stats: 'Statistik Cepat',
  label_system:      'Sistem',
  label_tools:       'Alat',
  label_appearance:  'Tampilan',
  label_developer:   'Pengembang',
  label_about:       'Tentang',

  /* Quick stats */
  stat_devices:  'Perangkat',
  stat_packages: 'Paket',

  /* System rows */
  sys_android: 'Versi Android',
  sys_abi:     'ABI',
  sys_zygisk:  'Varian Zygisk',
  sys_root_version: 'Versi Root',
  sys_kernel:  'Versi Kernel',

  /* Console */
  console_open:    'Buka Konsol',
  console_title:   'Konsol',
  console_logcat:  'Logcat',
  console_clear:   'Bersihkan',
  console_copy:    'Salin',
  console_cmd_ph:  'Ketik perintah shell…',
  console_copied:  'Disalin ke papan klip',
  console_copy_fail: 'Salin tidak tersedia di sini',
  console_cleared: 'Konsol dibersihkan',
  console_save:      'Simpan',
  console_saved:     'Log disimpan ke Download/COPG/LOGS',
  console_saved_dl:  'Log diunduh',
  console_save_empty:'Tidak ada yang disimpan',
  console_save_fail: 'Gagal menyimpan log',

  /* Page headers */
  page_library:  'Pustaka',
  page_settings: 'Pengaturan',

  /* Library tabs + search */
  tab_devices:     'Perangkat',
  tab_packages:    'Paket',
  search_devices:  'Cari perangkat…',
  search_packages: 'Cari paket…',
  empty_devices:   'Tidak ada perangkat',
  empty_packages:  'Tidak ada paket',

  /* Settings rows */
  set_theme:        'Tema',
  set_language:     'Bahasa',
  set_fullscreen:   'Layar Penuh',
  set_debug:        'Log Debug',
  set_refresh_icons:     'Segarkan Ikon Aplikasi',
  set_refresh_icons_sub: 'Unduh ulang ikon paket',

  /* Theme hints */
  theme_auto:   'Otomatis',
  theme_dark:   'Gelap',
  theme_light:  'Terang',
  theme_amoled: 'AMOLED',

  /* Sheets */
  sheet_theme_title: 'Tema',
  sheet_lang_title:  'Bahasa',
  theme_opt_auto:   'Otomatis (Sistem)',
  theme_opt_dark:   'Gelap',
  theme_opt_light:  'Terang',
  theme_opt_amoled: 'AMOLED Hitam',

  /* About */
  about_module:  'Modul COPG',
  about_version: 'Versi',
  about_build:   'Tanggal Build',
  about_vcode:   'Kode Versi',
  about_author:  'Pembuat',
  about_license: 'Lisensi',
  about_desc:    'Modul Zygisk yang memalsukan perangkatmu — dan opsional CPU-nya — per game atau aplikasi, membuka FPS lebih tinggi, grafik lebih baik, dan fitur premium yang biasanya terkunci untuk ponsel tertentu. Tambah atau hapus aplikasi kapan saja dari WebUI ini, tanpa perlu reboot.',

  /* Nav */
  nav_home:     'Beranda',
  nav_library:  'Pustaka',
  nav_settings: 'Pengaturan',

  /* Toasts */
  toast_console:   'Konsol — segera hadir',
  toast_debug_on:  'Log debug diaktifkan',
  toast_debug_off: 'Log debug dinonaktifkan',
  toast_fullscreen_on:  'Layar penuh aktif — bilah status disembunyikan',
  toast_fullscreen_off: 'Layar penuh nonaktif — bilah status ditampilkan',
  toast_icons_refreshing:      'Menyegarkan ikon aplikasi…',
  toast_icons_refreshed:       'Ikon aplikasi disegarkan',
  toast_icons_refresh_preview: 'Penyegaran ikon hanya berfungsi di perangkat',
  status_connected: 'Terhubung',
  status_offline:   'Luring',

  /* Library — add buttons & card meta */
  add_device:   'Tambah Perangkat',
  add_package:  'Tambah Paket',
  add_short:    'Tambah',
  act_edit:     'Edit',
  act_delete:   'Hapus',
  dev_model:    'Model',
  dev_games:    'game',
  dev_apps:     'aplikasi',
  badge_installed: 'Terpasang',
  preview_unsaved: 'Mode pratinjau — perubahan tidak disimpan ke perangkat.',

  /* System — root */
  sys_root: 'Root',

  /* Sort + filter */
  sort_title:     'Urutkan',
  sort_default:   'Bawaan',
  sort_name:      'Nama (A→Z)',
  sort_name_za:   'Nama (Z→A)',
  sort_apps:      'Jumlah aplikasi',
  sort_brand:     'Merek',
  sort_device:    'Perangkat',
  sort_type:      'Tipe',
  sort_installed: 'Terpasang dulu',
  filter_all:      'Semua',
  filter_user:     'Pengguna',
  filter_system:   'Sistem',
  filter_installed:'Terpasang',
  filter_blocked:  'Diblokir',
  filter_cpu_only: 'CPU',

  /* App picker */
  pick_from_installed: 'Pilih dari aplikasi terpasang',
  picker_title:   'Aplikasi Terpasang',
  picker_search:  'Cari aplikasi…',
  picker_empty:   'Tidak ada aplikasi terpasang',
  picker_no_results: 'Aplikasi tidak ditemukan',
  picker_preview: 'Mode pratinjau — pasang di perangkat untuk melihat aplikasi',
  toast_picker_unavailable: 'Pemilih aplikasi tidak tersedia di sini',
  devpicker_title:  'Profil Perangkat',
  devpicker_search: 'Cari perangkat…',
  devpicker_empty:  'Tidak ada perangkat — tambahkan dulu',

  /* Backup & Sync */
  lib_backup:           'Cadangkan & Sinkron',
  backup_title:         'Cadangkan & Sinkron',
  backup_intro:         'Cadangkan perangkat & paketmu, pulihkan cadangan sebelumnya, atau sinkronkan konfigurasi terbaru dari GitHub.',
  backup_export:        'Ekspor cadangan',
  backup_export_sub:    'Simpan data saat ini ke Download',
  backup_import:        'Impor & pulihkan',
  backup_import_sub:    'Pulihkan data dari cadangan',
  backup_empty:         'Tidak ada cadangan di Downloads/COPG',
  backup_from_file:     'Impor dari file…',
  backup_confirm_title: 'Pulihkan cadangan?',
  backup_confirm_msg:   'Ini akan menimpa perangkat & paketmu saat ini dengan:',
  backup_restore:       'Pulihkan',
  toast_backup_ok:      'Cadangan disimpan',
  toast_backup_fail:    'Pencadangan gagal',
  toast_restore_ok:     'Cadangan dipulihkan',
  toast_restore_fail:   'Pemulihan gagal',
  toast_invalid_json:   'Bukan file JSON yang valid',

  /* Import type chooser */
  backup_kind_title:       'File jenis apa ini?',
  backup_kind_config:      'Perangkat & Paket',
  backup_kind_config_sub:  'Konfigurasi perangkat & paket (COPG.json)',
  backup_kind_list:        'Nama Tampilan',
  backup_kind_list_sub:    'Nama tampilan paket (list.json)',
  backup_kind_detected:    'terdeteksi',

  /* Sync from GitHub */
  backup_sync:        'Sinkron dari GitHub',
  backup_sync_sub:    'Ambil konfigurasi terbaru dari repo',
  toast_sync_ok:      'Tersinkron dari GitHub',
  toast_sync_fail:    'Sinkron gagal — periksa koneksimu',
  toast_sync_preview: 'Sinkron hanya bekerja di perangkat',

  /* Package types (chips) */
  pkgtype_device:   'Perangkat',
  pkgtype_cpu_only: 'Palsukan CPU',
  pkgtype_blocked:  'Blokir Palsu CPU',

  /* Package tags (chips) */
  tag_withcpu:  'Dengan CPU',
  tag_cow:      'COW',
  tag_dnd:      'DND',
  tag_dab:      'Cerah Auto',
  tag_kso:      'Layar Nyala',
  tag_nolog:    'Tanpa Log',

  /* Device modal */
  dev_add_title:      'Tambah Perangkat',
  dev_edit_title:     'Edit Perangkat',
  dev_f_name:         'Nama Perangkat',
  dev_f_brand:        'Merek',
  dev_f_model:        'Model',
  dev_f_manufacturer: 'Produsen',
  dev_f_fingerprint:  'Fingerprint',
  dev_f_android:      'Versi Android',
  dev_f_sdk:          'SDK Int',
  dev_f_serial:       'Nomor Seri',
  dev_f_androidid:    'Android ID',
  opt_optional:       '(opsional)',
  serial_gen:         'Buat',
  info_androidid_title: 'Palsukan Android ID',
  info_androidid_msg: 'Membuat aplikasi pada profil perangkat ini membaca ANDROID_ID palsu (identifier dari Settings.Secure). 16 karakter heksa. Modul dibongkar sebelum aplikasi berjalan, sehingga tidak ada jejak COPG di memori aplikasi. Kosongkan untuk memakai ID asli. Catatan: aplikasi yang gigih masih bisa membaca ID asli lewat cara lain — ini hanya mengubah pencarian Settings standar.',
  info_sdk_title:     'SDK Int',
  info_sdk_msg:       'Level SDK dan Versi Android adalah pasangan yang cocok — terisi otomatis dari Versi Android. Jika kamu menyetel SDK yang tidak cocok, sebagian aplikasi mendeteksi ketidakcocokan dan crash. Edit manual hanya untuk versi Android baru yang belum dikenal modul.',

  /* Package modal */
  pkg_add_title:  'Tambah Paket',
  pkg_edit_title: 'Edit Paket',
  pkg_f_package:  'Nama Paket',
  pkg_f_name:     'Nama Tampilan',
  pkg_f_type:     'Tipe',
  pkg_f_device:   'Profil Perangkat',
  pkg_pick_device: 'Pilih perangkat…',
  pkg_no_devices: 'Tidak ada perangkat — tambah dulu',
  pkg_sec_spoofing: 'Pemalsuan',
  pkg_sec_tweaks:   'Penyesuaian',
  pkg_t_withcpu:  'Dengan Pemalsuan CPU',
  pkg_t_cow:      'Spoof Prop COW',
  pkg_t_dnd:      'Jangan Ganggu',
  pkg_t_dab:      'Matikan Kecerahan Otomatis',
  pkg_t_kso:      'Biarkan Layar Menyala',
  pkg_t_nolog:    'Matikan Logging',


  /* Toggle info popups */
  info_aria:           'Apa ini?',
  info_ok:             'Mengerti',
  info_withcpu_title:  'Dengan Pemalsuan CPU',
  info_withcpu_msg:    'Selain memalsukan model perangkat, ini juga memalsukan detail prosesor (CPU) ponselmu untuk aplikasi ini. Aktifkan untuk game yang memeriksa chipset. Biarkan mati (bawaan) dan pemalsuan CPU tetap diblokir — yang disukai aplikasi perbankan dan sensitif.',
  info_cow_title:      'Spoof Prop COW',
  info_cow_msg:        'Memalsukan properti sistem perangkat (ro.product.*, ro.build.fingerprint …) untuk aplikasi ini sehingga bahkan kode native-nya melihat perangkat palsu. Stealth: nilai ditulis ke tampilan copy-on-write privat dari area properti dan modul dibongkar sebelum aplikasi berjalan — tidak ada jejak COPG di memori aplikasi, aman dari pemindaian memori anti-cheat. Gunakan untuk game yang membaca props secara native.',
  info_dnd_title:      'Jangan Ganggu',
  info_dnd_msg:        'Membisukan panggilan dan notifikasi selama kamu di game ini, agar tidak ada yang mengganggu pertandinganmu. Pengaturan normalmu kembali saat keluar.',
  info_dab_title:      'Matikan Kecerahan Otomatis',
  info_dab_msg:        'Mencegah layar meredup dan menerangi sendiri saat bermain, sehingga kecerahan tetap sesuai setelanmu. Dipulihkan otomatis saat keluar game.',
  info_kso_title:      'Biarkan Layar Menyala',
  info_kso_msg:        'Menjaga layar tetap menyala selama game ini terbuka, meski tidak disentuh — tidak ada lagi layar mati di tengah pertandingan. Batas waktu normalmu kembali saat keluar.',
  info_nolog_title:    'Matikan Logging',
  info_nolog_msg:      'Menjeda pengumpul log sistem saat bermain untuk menghemat sedikit CPU dan penyimpanan. Menyala lagi otomatis saat keluar game.',

  /* Buttons */
  btn_cancel: 'Batal',
  btn_save:   'Simpan',
  btn_delete: 'Hapus',

  /* Validation messages */
  msg_required:    'Wajib',
  msg_duplicate:   'Duplikat',
  msg_dup_device:  'Perangkat dengan nama ini sudah ada',
  msg_dup_model:   'Perangkat dengan model ini sudah ada',
  msg_pick_device: 'Pilih perangkat',
  msg_dup_pkg:     'Paket ini sudah ada di',

  /* Confirm delete */
  confirm_del_device_title:  'Hapus Perangkat?',
  confirm_del_package_title: 'Hapus Paket?',

  /* Toasts — CRUD */
  toast_device_added:    'Perangkat ditambahkan',
  toast_device_saved:    'Perangkat disimpan',
  toast_device_deleted:  'Perangkat dihapus',
  toast_package_added:   'Paket ditambahkan',
  toast_package_saved:   'Paket disimpan',
  toast_package_deleted: 'Paket dihapus',
  toast_preview_only:    'Hanya pratinjau — tidak disimpan ke perangkat',
  toast_save_failed:     'Gagal menyimpan',
});
