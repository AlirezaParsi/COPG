/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   th.js — ไทย (Thai)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
I18N.register('th', {
  _meta: { name: 'ไทย', flag: '🇹🇭', dir: 'ltr' },

  /* Hero */
  hero_module_id: 'โมดูล Zygisk',
  hero_sub:       'การปลอมแปลงอุปกรณ์และแอปขั้นสูง',
  status_active:   'เปิดใช้งาน',
  status_inactive: 'ปิดใช้งาน',

  /* Section labels */
  label_quick_stats: 'สถิติด่วน',
  label_system:      'ระบบ',
  label_tools:       'เครื่องมือ',
  label_appearance:  'รูปลักษณ์',
  label_developer:   'นักพัฒนา',
  label_about:       'เกี่ยวกับ',

  /* Quick stats */
  stat_devices:  'อุปกรณ์',
  stat_packages: 'แพ็กเกจ',

  /* System rows */
  sys_android: 'เวอร์ชัน Android',
  sys_abi:     'ABI',
  sys_zygisk:  'ชนิด Zygisk',
  sys_root_version: 'เวอร์ชัน Root',
  sys_kernel:  'เวอร์ชันเคอร์เนล',

  /* Console */
  console_open:    'เปิดคอนโซล',
  console_title:   'คอนโซล',
  console_logcat:  'Logcat',
  console_clear:   'ล้าง',
  console_copy:    'คัดลอก',
  console_cmd_ph:  'พิมพ์คำสั่งเชลล์…',
  console_copied:  'คัดลอกไปยังคลิปบอร์ดแล้ว',
  console_copy_fail: 'คัดลอกไม่ได้ที่นี่',
  console_cleared: 'ล้างคอนโซลแล้ว',
  console_save:      'บันทึก',
  console_saved:     'บันทึกล็อกไปที่ Download/COPG/LOGS แล้ว',
  console_saved_dl:  'ดาวน์โหลดล็อกแล้ว',
  console_save_empty:'ไม่มีอะไรให้บันทึก',
  console_save_fail: 'บันทึกล็อกล้มเหลว',

  /* Page headers */
  page_library:  'คลัง',
  page_settings: 'การตั้งค่า',

  /* Library tabs + search */
  tab_devices:     'อุปกรณ์',
  tab_packages:    'แพ็กเกจ',
  search_devices:  'ค้นหาอุปกรณ์…',
  search_packages: 'ค้นหาแพ็กเกจ…',
  empty_devices:   'ไม่พบอุปกรณ์',
  empty_packages:  'ไม่พบแพ็กเกจ',

  /* Settings rows */
  set_theme:        'ธีม',
  set_language:     'ภาษา',
  set_debug:        'ล็อกดีบัก',

  /* Theme hints */
  theme_auto:   'อัตโนมัติ',
  theme_dark:   'มืด',
  theme_light:  'สว่าง',
  theme_amoled: 'AMOLED',

  /* Sheets */
  sheet_theme_title: 'ธีม',
  sheet_lang_title:  'ภาษา',
  theme_opt_auto:   'อัตโนมัติ (ระบบ)',
  theme_opt_dark:   'มืด',
  theme_opt_light:  'สว่าง',
  theme_opt_amoled: 'ดำ AMOLED',

  /* About */
  about_module:  'โมดูล COPG',
  about_version: 'เวอร์ชัน',
  about_build:   'วันที่สร้าง',
  about_vcode:   'รหัสเวอร์ชัน',
  about_author:  'ผู้พัฒนา',
  about_license: 'สัญญาอนุญาต',
  about_desc:    'โมดูล Zygisk ที่ปลอมแปลงอุปกรณ์ของคุณ — และปลอม CPU ได้ตามต้องการ — แยกตามเกมหรือแอป เพื่อปลดล็อก FPS ที่สูงขึ้น กราฟิกที่ดีขึ้น และฟีเจอร์พรีเมียมที่ปกติจำกัดเฉพาะบางรุ่น เพิ่มหรือลบแอปได้ทุกเมื่อจาก WebUI นี้ โดยไม่ต้องรีบูต',

  /* Nav */
  nav_home:     'หน้าแรก',
  nav_library:  'คลัง',
  nav_settings: 'การตั้งค่า',

  /* Toasts */
  toast_console:   'คอนโซล — เร็ว ๆ นี้',
  toast_debug_on:  'เปิดล็อกดีบักแล้ว',
  toast_debug_off: 'ปิดล็อกดีบักแล้ว',
  status_connected: 'เชื่อมต่อแล้ว',
  status_offline:   'ออฟไลน์',

  /* Library — add buttons & card meta */
  add_device:   'เพิ่มอุปกรณ์',
  add_package:  'เพิ่มแพ็กเกจ',
  add_short:    'เพิ่ม',
  act_edit:     'แก้ไข',
  act_delete:   'ลบ',
  dev_model:    'รุ่น',
  dev_games:    'เกม',
  dev_apps:     'แอป',
  badge_installed: 'ติดตั้งแล้ว',
  preview_unsaved: 'โหมดดูตัวอย่าง — การเปลี่ยนแปลงไม่ถูกบันทึกลงอุปกรณ์',

  /* System — root */
  sys_root: 'Root',

  /* Sort + filter */
  sort_title:     'เรียงตาม',
  sort_default:   'ค่าเริ่มต้น',
  sort_name:      'ชื่อ (A→Z)',
  sort_name_za:   'ชื่อ (Z→A)',
  sort_apps:      'จำนวนแอป',
  sort_brand:     'แบรนด์',
  sort_device:    'อุปกรณ์',
  sort_type:      'ประเภท',
  sort_installed: 'ติดตั้งแล้วก่อน',
  filter_all:      'ทั้งหมด',
  filter_user:     'ผู้ใช้',
  filter_system:   'ระบบ',
  filter_installed:'ติดตั้งแล้ว',
  filter_blocked:  'ถูกบล็อก',
  filter_cpu_only: 'CPU',

  /* App picker */
  pick_from_installed: 'เลือกจากแอปที่ติดตั้ง',
  picker_title:   'แอปที่ติดตั้ง',
  picker_search:  'ค้นหาแอป…',
  picker_empty:   'ไม่พบแอปที่ติดตั้ง',
  picker_no_results: 'ไม่พบแอป',
  picker_preview: 'โหมดดูตัวอย่าง — ติดตั้งบนอุปกรณ์เพื่อดูรายการแอป',
  toast_picker_unavailable: 'ตัวเลือกแอปใช้ไม่ได้ที่นี่',

  /* Backup & Sync */
  lib_backup:           'สำรองและซิงค์',
  backup_title:         'สำรองและซิงค์',
  backup_intro:         'สำรองอุปกรณ์และแพ็กเกจของคุณ คืนค่าจากข้อมูลสำรองก่อนหน้า หรือซิงค์การตั้งค่าล่าสุดจาก GitHub',
  backup_export:        'ส่งออกข้อมูลสำรอง',
  backup_export_sub:    'บันทึกข้อมูลปัจจุบันไปยังดาวน์โหลด',
  backup_import:        'นำเข้าและคืนค่า',
  backup_import_sub:    'คืนค่าข้อมูลจากข้อมูลสำรอง',
  backup_empty:         'ไม่พบข้อมูลสำรองใน Downloads/COPG',
  backup_from_file:     'นำเข้าจากไฟล์…',
  backup_confirm_title: 'คืนค่าข้อมูลสำรอง?',
  backup_confirm_msg:   'การทำเช่นนี้จะเขียนทับอุปกรณ์และแพ็กเกจปัจจุบันด้วย:',
  backup_restore:       'คืนค่า',
  toast_backup_ok:      'บันทึกข้อมูลสำรองแล้ว',
  toast_backup_fail:    'สำรองข้อมูลล้มเหลว',
  toast_restore_ok:     'คืนค่าข้อมูลสำรองแล้ว',
  toast_restore_fail:   'คืนค่าล้มเหลว',
  toast_invalid_json:   'ไม่ใช่ไฟล์ JSON ที่ถูกต้อง',

  /* Import type chooser */
  backup_kind_title:       'ไฟล์นี้เป็นชนิดใด?',
  backup_kind_config:      'อุปกรณ์และแพ็กเกจ',
  backup_kind_config_sub:  'การตั้งค่าอุปกรณ์และแพ็กเกจ (COPG.json)',
  backup_kind_list:        'ชื่อที่แสดง',
  backup_kind_list_sub:    'ชื่อที่แสดงของแพ็กเกจ (list.json)',
  backup_kind_detected:    'ตรวจพบ',

  /* Sync from GitHub */
  backup_sync:        'ซิงค์จาก GitHub',
  backup_sync_sub:    'ดึงการตั้งค่าล่าสุดจากที่เก็บ',
  toast_sync_ok:      'ซิงค์จาก GitHub แล้ว',
  toast_sync_fail:    'ซิงค์ล้มเหลว — ตรวจสอบการเชื่อมต่อ',
  toast_sync_preview: 'การซิงค์ใช้ได้บนอุปกรณ์เท่านั้น',

  /* Package types (chips) */
  pkgtype_device:   'อุปกรณ์',
  pkgtype_cpu_only: 'ปลอม CPU',
  pkgtype_blocked:  'บล็อกการปลอม CPU',

  /* Package tags (chips) */
  tag_withcpu:  'พร้อม CPU',
  tag_got:      'GOT',
  tag_dnd:      'ห้ามรบกวน',
  tag_dab:      'สว่างอัตโนมัติ',
  tag_kso:      'จอติด',
  tag_nolog:    'ไม่บันทึก',

  /* Device modal */
  dev_add_title:      'เพิ่มอุปกรณ์',
  dev_edit_title:     'แก้ไขอุปกรณ์',
  dev_f_name:         'ชื่ออุปกรณ์',
  dev_f_brand:        'แบรนด์',
  dev_f_model:        'รุ่น',
  dev_f_manufacturer: 'ผู้ผลิต',
  dev_f_fingerprint:  'ลายนิ้วมือ (fingerprint)',
  dev_f_android:      'เวอร์ชัน Android',
  dev_f_sdk:          'SDK Int',
  dev_f_serial:       'หมายเลขซีเรียล',
  opt_optional:       '(ไม่บังคับ)',
  serial_gen:         'สร้าง',
  info_sdk_title:     'SDK Int',
  info_sdk_msg:       'ระดับ SDK และเวอร์ชัน Android เป็นคู่ที่ต้องตรงกัน — จะกรอกอัตโนมัติจากเวอร์ชัน Android หากตั้ง SDK ที่ไม่ตรงกัน บางแอปจะตรวจพบความไม่ตรงและขัดข้อง แก้ไขด้วยตนเองเฉพาะเวอร์ชัน Android ใหม่ที่โมดูลยังไม่รู้จักเท่านั้น',

  /* Package modal */
  pkg_add_title:  'เพิ่มแพ็กเกจ',
  pkg_edit_title: 'แก้ไขแพ็กเกจ',
  pkg_f_package:  'ชื่อแพ็กเกจ',
  pkg_f_name:     'ชื่อที่แสดง',
  pkg_f_type:     'ประเภท',
  pkg_f_device:   'โปรไฟล์อุปกรณ์',
  pkg_no_devices: 'ไม่มีอุปกรณ์ — เพิ่มก่อน',
  pkg_sec_spoofing: 'การปลอมแปลง',
  pkg_sec_tweaks:   'การปรับแต่ง',
  pkg_t_withcpu:  'ปลอม CPU ด้วย',
  pkg_t_got:      'GOT Hooking',
  pkg_t_dnd:      'ห้ามรบกวน',
  pkg_t_dab:      'ปิดความสว่างอัตโนมัติ',
  pkg_t_kso:      'เปิดหน้าจอค้างไว้',
  pkg_t_nolog:    'ปิดการบันทึกล็อก',

  /* GOT Hooking warning */
  got_warn_title: 'เปิด GOT Hooking?',
  got_warn_msg:   'GOT Hooking จะทำให้โมดูลคงอยู่ในหน่วยความจำของแอปหรือเกม สิ่งนี้อาจถูกตรวจพบและอาจทำให้บัญชีเกมของคุณถูกแบนได้\n\nแน่ใจหรือไม่ว่าต้องการเปิด? เราแนะนำให้สอบถามชุมชนก่อนตัดสินใจ',
  got_warn_ok:    'เปิดต่อไป',
  got_warn_link:  'ชุมชน COPG บน Telegram',

  /* Toggle info popups */
  info_aria:           'นี่คืออะไร?',
  info_ok:             'เข้าใจแล้ว',
  info_withcpu_title:  'ปลอม CPU ด้วย',
  info_withcpu_msg:    'นอกจากปลอมรุ่นอุปกรณ์แล้ว ยังปลอมรายละเอียดหน่วยประมวลผล (CPU) ของเครื่องสำหรับแอปนี้ด้วย เปิดสำหรับเกมที่ตรวจสอบชิปเซ็ต ปล่อยปิดไว้ (ค่าเริ่มต้น) การปลอม CPU จะถูกบล็อก — ซึ่งแอปธนาคารและแอปที่ละเอียดอ่อนชอบแบบนั้น',
  info_got_title:      'GOT Hooking',
  info_got_msg:        'วิธีที่ลึกและรุนแรงกว่าในการปลอมจากภายในหน่วยความจำของแอป แรงกว่าแต่เสี่ยงกว่า: อาจถูกตรวจพบและอาจทำให้บัญชีเกมถูกแบน เปิดเฉพาะเมื่อคุณรู้ว่าต้องใช้เท่านั้น',
  info_dnd_title:      'ห้ามรบกวน',
  info_dnd_msg:        'ปิดเสียงสายและการแจ้งเตือนขณะอยู่ในเกมนี้ เพื่อไม่ให้มีอะไรมารบกวนแมตช์ของคุณ การตั้งค่าปกติจะกลับมาเมื่อออก',
  info_dab_title:      'ปิดความสว่างอัตโนมัติ',
  info_dab_msg:        'หยุดไม่ให้หน้าจอหรี่และสว่างเองขณะเล่น เพื่อให้ความสว่างคงที่ตามที่ตั้งไว้ คืนค่าอัตโนมัติเมื่อออกจากเกม',
  info_kso_title:      'เปิดหน้าจอค้างไว้',
  info_kso_msg:        'เปิดหน้าจอค้างไว้ขณะเกมนี้เปิดอยู่ แม้ไม่ได้แตะ — ไม่มีจอดับกลางแมตช์อีกต่อไป เวลาหมดเวลาปกติจะกลับมาเมื่อออก',
  info_nolog_title:    'ปิดการบันทึกล็อก',
  info_nolog_msg:      'หยุดตัวเก็บล็อกระบบชั่วคราวขณะเล่นเพื่อประหยัด CPU และพื้นที่เล็กน้อย จะเปิดกลับเองเมื่อออกจากเกม',

  /* Buttons */
  btn_cancel: 'ยกเลิก',
  btn_save:   'บันทึก',
  btn_delete: 'ลบ',

  /* Validation messages */
  msg_required:    'จำเป็น',
  msg_duplicate:   'ซ้ำ',
  msg_dup_device:  'มีอุปกรณ์ชื่อนี้อยู่แล้ว',
  msg_dup_model:   'มีอุปกรณ์รุ่นนี้อยู่แล้ว',
  msg_pick_device: 'เลือกอุปกรณ์',
  msg_dup_pkg:     'แพ็กเกจนี้มีอยู่แล้วใน',

  /* Confirm delete */
  confirm_del_device_title:  'ลบอุปกรณ์?',
  confirm_del_package_title: 'ลบแพ็กเกจ?',

  /* Toasts — CRUD */
  toast_device_added:    'เพิ่มอุปกรณ์แล้ว',
  toast_device_saved:    'บันทึกอุปกรณ์แล้ว',
  toast_device_deleted:  'ลบอุปกรณ์แล้ว',
  toast_package_added:   'เพิ่มแพ็กเกจแล้ว',
  toast_package_saved:   'บันทึกแพ็กเกจแล้ว',
  toast_package_deleted: 'ลบแพ็กเกจแล้ว',
  toast_preview_only:    'ดูตัวอย่างเท่านั้น — ไม่ได้บันทึกลงอุปกรณ์',
  toast_save_failed:     'บันทึกล้มเหลว',
});
