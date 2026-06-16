/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   ar.js — العربية (Arabic) · RTL
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
I18N.register('ar', {
  _meta: { name: 'العربية', flag: '🇸🇦', dir: 'rtl' },

  /* Hero */
  hero_module_id: 'وحدة Zygisk',
  hero_sub:       'انتحال متقدّم للجهاز والتطبيقات',
  status_active:   'نشط',
  status_inactive: 'غير نشط',

  /* Section labels */
  label_quick_stats: 'إحصائيات سريعة',
  label_system:      'النظام',
  label_tools:       'الأدوات',
  label_appearance:  'المظهر',
  label_developer:   'المطوّر',
  label_about:       'حول',

  /* Quick stats */
  stat_devices:  'الأجهزة',
  stat_packages: 'الحزم',

  /* System rows */
  sys_android: 'إصدار أندرويد',
  sys_abi:     'ABI',
  sys_zygisk:  'نوع Zygisk',
  sys_root_version: 'إصدار الروت',
  sys_kernel:  'إصدار النواة',

  /* Console */
  console_open:    'فتح وحدة التحكم',
  console_title:   'وحدة التحكم',
  console_logcat:  'Logcat',
  console_clear:   'مسح',
  console_copy:    'نسخ',
  console_cmd_ph:  'اكتب أمر شِل…',
  console_copied:  'تم النسخ إلى الحافظة',
  console_copy_fail: 'النسخ غير متاح هنا',
  console_cleared: 'تم مسح وحدة التحكم',
  console_save:      'حفظ',
  console_saved:     'تم حفظ السجل في Download/COPG/LOGS',
  console_saved_dl:  'تم تنزيل السجل',
  console_save_empty:'لا شيء للحفظ',
  console_save_fail: 'فشل حفظ السجل',

  /* Page headers */
  page_library:  'المكتبة',
  page_settings: 'الإعدادات',

  /* Library tabs + search */
  tab_devices:     'الأجهزة',
  tab_packages:    'الحزم',
  search_devices:  'البحث في الأجهزة…',
  search_packages: 'البحث في الحزم…',
  empty_devices:   'لا توجد أجهزة',
  empty_packages:  'لا توجد حزم',

  /* Settings rows */
  set_theme:        'السمة',
  set_language:     'اللغة',
  set_fullscreen:   'ملء الشاشة',
  set_debug:        'سجلات التصحيح',
  set_refresh_icons:     'تحديث أيقونات التطبيقات',
  set_refresh_icons_sub: 'إعادة تنزيل أيقونات الحزم',

  /* Theme hints */
  theme_auto:   'تلقائي',
  theme_dark:   'داكن',
  theme_light:  'فاتح',
  theme_amoled: 'AMOLED',

  /* Sheets */
  sheet_theme_title: 'السمة',
  sheet_lang_title:  'اللغة',
  theme_opt_auto:   'تلقائي (النظام)',
  theme_opt_dark:   'داكن',
  theme_opt_light:  'فاتح',
  theme_opt_amoled: 'أسود AMOLED',

  /* About */
  about_module:  'وحدة COPG',
  about_version: 'الإصدار',
  about_build:   'تاريخ البناء',
  about_vcode:   'رقم الإصدار',
  about_author:  'المطوّر',
  about_license: 'الترخيص',
  about_desc:    'وحدة Zygisk تنتحل هوية جهازك — واختيارياً معالجه — لكل لعبة أو تطبيق، لتفتح معدّل إطارات أعلى ورسومات أفضل وميزات مدفوعة محصورة عادةً بهواتف معيّنة. أضِف أو احذف التطبيقات في أي وقت من واجهة الويب هذه، دون الحاجة لإعادة التشغيل.',

  /* Nav */
  nav_home:     'الرئيسية',
  nav_library:  'المكتبة',
  nav_settings: 'الإعدادات',

  /* Toasts */
  toast_console:   'وحدة التحكم — قريباً',
  toast_debug_on:  'تم تفعيل سجلات التصحيح',
  toast_debug_off: 'تم تعطيل سجلات التصحيح',
  toast_fullscreen_on:  'ملء الشاشة مُفعّل — شريط الحالة مخفي',
  toast_fullscreen_off: 'ملء الشاشة مُعطّل — شريط الحالة ظاهر',
  toast_icons_refreshing:      'جارٍ تحديث أيقونات التطبيقات…',
  toast_icons_refreshed:       'تم تحديث أيقونات التطبيقات',
  toast_icons_refresh_preview: 'تحديث الأيقونات يعمل على الجهاز فقط',
  status_connected: 'متصل',
  status_offline:   'غير متصل',

  /* Library — add buttons & card meta */
  add_device:   'إضافة جهاز',
  add_package:  'إضافة حزمة',
  add_short:    'إضافة',
  act_edit:     'تعديل',
  act_delete:   'حذف',
  dev_model:    'الطراز',
  dev_games:    'ألعاب',
  dev_apps:     'تطبيقات',
  badge_installed: 'مثبّت',
  preview_unsaved: 'وضع المعاينة — لا تُحفظ التغييرات على الجهاز.',

  /* System — root */
  sys_root: 'الروت',

  /* Sort + filter */
  sort_title:     'ترتيب حسب',
  sort_default:   'افتراضي',
  sort_name:      'الاسم (أ→ي)',
  sort_name_za:   'الاسم (ي→أ)',
  sort_apps:      'عدد التطبيقات',
  sort_brand:     'العلامة التجارية',
  sort_device:    'الجهاز',
  sort_type:      'النوع',
  sort_installed: 'المثبّتة أولاً',
  filter_all:      'الكل',
  filter_user:     'مستخدم',
  filter_system:   'نظام',
  filter_installed:'مثبّت',
  filter_blocked:  'محظور',
  filter_cpu_only: 'المعالج',

  /* App picker */
  pick_from_installed: 'اختر من التطبيقات المثبتة',
  picker_title:   'التطبيقات المثبتة',
  picker_search:  'البحث في التطبيقات…',
  picker_empty:   'لا توجد تطبيقات مثبتة',
  picker_no_results: 'لا توجد تطبيقات',
  picker_preview: 'وضع المعاينة — ثبّت على الجهاز لعرض التطبيقات',
  toast_picker_unavailable: 'منتقي التطبيقات غير متاح هنا',
  devpicker_title:  'ملفات الأجهزة',
  devpicker_search: 'ابحث عن الأجهزة…',
  devpicker_empty:  'لا توجد أجهزة — أضف واحدًا أولاً',

  /* Backup & Sync */
  lib_backup:           'النسخ الاحتياطي والمزامنة',
  backup_title:         'النسخ الاحتياطي والمزامنة',
  backup_intro:         'انسخ أجهزتك وحزمك احتياطياً، أو استعد نسخة سابقة، أو زامن أحدث إعداد من GitHub.',
  backup_export:        'تصدير نسخة احتياطية',
  backup_export_sub:    'حفظ البيانات الحالية في التنزيلات',
  backup_import:        'استيراد واستعادة',
  backup_import_sub:    'استعادة البيانات من نسخة احتياطية',
  backup_empty:         'لا توجد نسخ احتياطية في Downloads/COPG',
  backup_from_file:     'استيراد من ملف…',
  backup_confirm_title: 'استعادة النسخة الاحتياطية؟',
  backup_confirm_msg:   'سيؤدي هذا إلى استبدال أجهزتك وحزمك الحالية بـ:',
  backup_restore:       'استعادة',
  toast_backup_ok:      'تم حفظ النسخة الاحتياطية',
  toast_backup_fail:    'فشل النسخ الاحتياطي',
  toast_restore_ok:     'تمت الاستعادة',
  toast_restore_fail:   'فشلت الاستعادة',
  toast_invalid_json:   'ملف JSON غير صالح',

  /* Import type chooser */
  backup_kind_title:       'ما نوع هذا الملف؟',
  backup_kind_config:      'الأجهزة والحزم',
  backup_kind_config_sub:  'إعداد الأجهزة والحزم (COPG.json)',
  backup_kind_list:        'أسماء العرض',
  backup_kind_list_sub:    'أسماء عرض الحزم (list.json)',
  backup_kind_detected:    'تم الكشف',

  /* Sync from GitHub */
  backup_sync:        'مزامنة من GitHub',
  backup_sync_sub:    'جلب أحدث إعداد من المستودع',
  toast_sync_ok:      'تمت المزامنة من GitHub',
  toast_sync_fail:    'فشلت المزامنة — تحقق من اتصالك',
  toast_sync_preview: 'المزامنة تعمل على الجهاز فقط',

  /* Package types (chips) */
  pkgtype_device:   'جهاز',
  pkgtype_cpu_only: 'انتحال المعالج',
  pkgtype_blocked:  'حظر انتحال المعالج',

  /* Package tags (chips) */
  tag_withcpu:  'مع المعالج',
  tag_cow:      'COW',
  tag_dnd:      'عدم الإزعاج',
  tag_dab:      'سطوع تلقائي',
  tag_kso:      'إبقاء الشاشة',
  tag_nolog:    'بدون سجل',

  /* Device modal */
  dev_add_title:      'إضافة جهاز',
  dev_edit_title:     'تعديل الجهاز',
  dev_f_name:         'اسم الجهاز',
  dev_f_brand:        'العلامة التجارية',
  dev_f_model:        'الطراز',
  dev_f_manufacturer: 'الشركة المصنّعة',
  dev_f_fingerprint:  'البصمة',
  dev_f_android:      'إصدار أندرويد',
  dev_f_sdk:          'SDK Int',
  dev_f_serial:       'الرقم التسلسلي',
  dev_f_androidid:    'معرّف أندرويد',
  opt_optional:       '(اختياري)',
  serial_gen:         'توليد',
  info_androidid_title: 'تزييف معرّف أندرويد',
  info_androidid_msg: 'يجعل تطبيقات هذا الملف للجهاز تقرأ ANDROID_ID مزيّفًا (المعرّف من Settings.Secure). ١٦ خانة سداسية. تُفرَّغ الوحدة قبل تشغيل التطبيق، فلا يبقى أثر من COPG في ذاكرة التطبيق. اتركه فارغًا للإبقاء على المعرّف الحقيقي. ملاحظة: قد يقرأ تطبيق مصمَّم المعرّف الحقيقي بوسائل أخرى — هذا يغيّر فقط بحث Settings القياسي.',
  info_sdk_title:     'SDK Int',
  info_sdk_msg:       'مستوى SDK وإصدار أندرويد زوج متطابق — يُملأ تلقائياً من إصدار أندرويد. إذا ضبطت SDK لا يطابق الإصدار، فقد تكتشف بعض التطبيقات عدم التطابق وتتعطّل. عدّله يدوياً فقط لإصدار أندرويد جديد لا تعرفه الوحدة بعد.',

  /* Package modal */
  pkg_add_title:  'إضافة حزمة',
  pkg_edit_title: 'تعديل الحزمة',
  pkg_f_package:  'اسم الحزمة',
  pkg_f_name:     'اسم العرض',
  pkg_f_type:     'النوع',
  pkg_f_device:   'ملف الجهاز',
  pkg_pick_device: 'اختر جهازًا…',
  pkg_no_devices: 'لا توجد أجهزة — أضِف واحداً أولاً',
  pkg_sec_spoofing: 'الانتحال',
  pkg_sec_tweaks:   'التعديلات',
  pkg_t_withcpu:  'انتحال المعالج',
  pkg_t_cow:      'تزييف خصائص COW',
  pkg_t_dnd:      'عدم الإزعاج',
  pkg_t_dab:      'تعطيل السطوع التلقائي',
  pkg_t_kso:      'إبقاء الشاشة مضاءة',
  pkg_t_nolog:    'تعطيل التسجيل',


  /* Toggle info popups */
  info_aria:           'ما هذا؟',
  info_ok:             'فهمت',
  info_withcpu_title:  'انتحال المعالج',
  info_withcpu_msg:    'بالإضافة إلى تزييف طراز الجهاز، يزيّف هذا أيضاً تفاصيل معالج هاتفك (CPU) لهذا التطبيق. فعّله للألعاب التي تفحص الشريحة. اتركه مُطفأً (الافتراضي) ويبقى تزييف المعالج محظوراً — وهو ما تفضّله التطبيقات البنكية والحساسة.',
  info_cow_title:      'تزييف خصائص COW',
  info_cow_msg:        'يزيّف خصائص نظام الجهاز (ro.product.*، ro.build.fingerprint…) لهذا التطبيق حتى يرى كوده الأصلي (native) جهازًا مزيّفًا. خفيّ: تُكتب القيم في نسخة خاصة بنظام النسخ-عند-الكتابة من منطقة الخصائص، وتُفرَّغ الوحدة قبل تشغيل التطبيق، فلا يبقى أثر من COPG في ذاكرة التطبيق — آمن ضد فحص ذاكرة مكافحة الغش. استخدمه للألعاب التي تقرأ الخصائص أصليًا.',
  info_dnd_title:      'عدم الإزعاج',
  info_dnd_msg:        'يُسكت المكالمات والإشعارات أثناء وجودك في هذه اللعبة، فلا يقاطع شيء مباراتك. يعود إعدادك المعتاد عند الخروج.',
  info_dab_title:      'تعطيل السطوع التلقائي',
  info_dab_msg:        'يمنع الشاشة من التعتيم والإضاءة التلقائية أثناء اللعب، فيبقى السطوع كما ضبطته. يُستعاد تلقائياً عند مغادرة اللعبة.',
  info_kso_title:      'إبقاء الشاشة مضاءة',
  info_kso_msg:        'يُبقي الشاشة مضاءة أثناء فتح هذه اللعبة حتى لو لم تلمسها — لا مزيد من انطفاء الشاشة وسط المباراة. تعود مهلتك المعتادة عند الخروج.',
  info_nolog_title:    'تعطيل التسجيل',
  info_nolog_msg:      'يوقف مؤقتاً جامع سجلات النظام أثناء لعبك لتوفير قليل من المعالج والتخزين. يعود للعمل تلقائياً عند مغادرة اللعبة.',

  /* Buttons */
  btn_cancel: 'إلغاء',
  btn_save:   'حفظ',
  btn_delete: 'حذف',

  /* Validation messages */
  msg_required:    'مطلوب',
  msg_duplicate:   'مكرر',
  msg_dup_device:  'يوجد جهاز بهذا الاسم بالفعل',
  msg_dup_model:   'يوجد جهاز بهذا الطراز بالفعل',
  msg_pick_device: 'اختر جهازاً',
  msg_dup_pkg:     'هذه الحزمة موجودة بالفعل في',

  /* Confirm delete */
  confirm_del_device_title:  'حذف الجهاز؟',
  confirm_del_package_title: 'حذف الحزمة؟',

  /* Toasts — CRUD */
  toast_device_added:    'تمت إضافة الجهاز',
  toast_device_saved:    'تم حفظ الجهاز',
  toast_device_deleted:  'تم حذف الجهاز',
  toast_package_added:   'تمت إضافة الحزمة',
  toast_package_saved:   'تم حفظ الحزمة',
  toast_package_deleted: 'تم حذف الحزمة',
  toast_preview_only:    'معاينة فقط — لم تُحفظ على الجهاز',
  toast_save_failed:     'فشل الحفظ',
});
