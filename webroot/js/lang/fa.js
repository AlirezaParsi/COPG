/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   fa.js — فارسی (Persian) · RTL
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
I18N.register('fa', {
  _meta: { name: 'فارسی', flag: '🇮🇷', dir: 'rtl' },

  /* Hero */
  hero_module_id: 'ماژول Zygisk',
  hero_sub:       'جعل پیشرفته دستگاه و برنامه',
  status_active:   'فعال',
  status_inactive: 'غیرفعال',

  /* Section labels */
  label_quick_stats: 'آمار سریع',
  label_system:      'سیستم',
  label_tools:       'ابزارها',
  label_appearance:  'ظاهر',
  label_developer:   'توسعه‌دهنده',
  label_about:       'درباره',

  /* Quick stats */
  stat_devices:  'دستگاه‌ها',
  stat_packages: 'بسته‌ها',

  /* System rows */
  sys_android: 'نسخه اندروید',
  sys_abi:     'معماری (ABI)',
  sys_zygisk:  'نوع Zygisk',
  sys_root_version: 'نسخه روت',
  sys_kernel:  'نسخه کرنل',

  /* Console */
  console_open:    'باز کردن کنسول',
  console_title:   'کنسول',
  console_logcat:  'لاگ‌کت',
  console_clear:   'پاک‌سازی',
  console_copy:    'کپی',
  console_cmd_ph:  'یک دستور شل بنویسید…',
  console_copied:  'در کلیپ‌بورد کپی شد',
  console_copy_fail: 'کپی اینجا در دسترس نیست',
  console_cleared: 'کنسول پاک شد',
  console_save:      'ذخیره',
  console_saved:     'لاگ در Download/COPG/LOGS ذخیره شد',
  console_saved_dl:  'لاگ دانلود شد',
  console_save_empty:'چیزی برای ذخیره نیست',
  console_save_fail: 'ذخیرهٔ لاگ ناموفق بود',

  /* Page headers */
  page_library:  'کتابخانه',
  page_settings: 'تنظیمات',

  /* Library tabs + search */
  tab_devices:     'دستگاه‌ها',
  tab_packages:    'بسته‌ها',
  search_devices:  'جستجوی دستگاه‌ها…',
  search_packages: 'جستجوی بسته‌ها…',
  empty_devices:   'دستگاهی یافت نشد',
  empty_packages:  'بسته‌ای یافت نشد',

  /* Settings rows */
  set_theme:        'پوسته',
  set_language:     'زبان',
  set_fullscreen:   'تمام‌صفحه',
  set_debug:        'گزارش‌های اشکال‌زدایی',

  /* Theme hints (settings row) */
  theme_auto:   'خودکار',
  theme_dark:   'تیره',
  theme_light:  'روشن',
  theme_amoled: 'آمولد',

  /* Sheets */
  sheet_theme_title: 'پوسته',
  sheet_lang_title:  'زبان',
  theme_opt_auto:   'خودکار (سیستم)',
  theme_opt_dark:   'تیره',
  theme_opt_light:  'روشن',
  theme_opt_amoled: 'مشکی AMOLED',

  /* About (module info) */
  about_module:  'ماژول COPG',
  about_version: 'نسخه',
  about_build:   'تاریخ ساخت',
  about_vcode:   'کد نسخه',
  about_author:  'سازنده',
  about_license: 'مجوز',
  about_desc:    'یک ماژول Zygisk که دستگاه — و در صورت نیاز CPU — را برای هر بازی یا برنامه جعل می‌کند تا FPS بالاتر، گرافیک بهتر و قابلیت‌های ویژه‌ای که مخصوص گوشی‌های دیگر است باز شود. افزودن یا حذف برنامه‌ها هر زمان از همین WebUI، بدون نیاز به ریبوت.',

  /* Nav */
  nav_home:     'خانه',
  nav_library:  'کتابخانه',
  nav_settings: 'تنظیمات',

  /* Toasts */
  toast_console:   'کنسول — به‌زودی',
  toast_debug_on:  'گزارش‌های اشکال‌زدایی فعال شد',
  toast_debug_off: 'گزارش‌های اشکال‌زدایی غیرفعال شد',
  toast_fullscreen_on:  'تمام‌صفحه روشن — نوار وضعیت پنهان شد',
  toast_fullscreen_off: 'تمام‌صفحه خاموش — نوار وضعیت نمایش داده شد',
  status_connected: 'متصل',
  status_offline:   'آفلاین',

  /* Library — add buttons & card meta */
  add_device:   'افزودن دستگاه',
  add_package:  'افزودن بسته',
  add_short:    'افزودن',
  act_edit:     'ویرایش',
  act_delete:   'حذف',
  dev_model:    'مدل',
  dev_games:    'بازی',
  dev_apps:     'اپ',
  badge_installed: 'نصب‌شده',
  preview_unsaved: 'حالت پیش‌نمایش — تغییرات روی دستگاه ذخیره نمی‌شوند.',

  /* System — root */
  sys_root: 'روت',

  /* Sort + filter */
  sort_title:     'مرتب‌سازی بر اساس',
  sort_default:   'پیش‌فرض',
  sort_name:      'نام (A→Z)',
  sort_name_za:   'نام (Z→A)',
  sort_apps:      'تعداد اپ',
  sort_brand:     'برند',
  sort_device:    'دستگاه',
  sort_type:      'نوع',
  sort_installed: 'اول نصب‌شده‌ها',
  filter_all:      'همه',
  filter_user:     'کاربر',
  filter_system:   'سیستم',
  filter_installed:'نصب‌شده',
  filter_blocked:  'مسدود',
  filter_cpu_only: 'CPU',

  /* App picker */
  pick_from_installed: 'انتخاب از اپ‌های نصب‌شده',
  picker_title:   'اپ‌های نصب‌شده',
  picker_search:  'جستجوی اپ‌ها…',
  picker_empty:   'اپ نصب‌شده‌ای یافت نشد',
  picker_no_results: 'اپی یافت نشد',
  picker_preview: 'حالت پیش‌نمایش — برای لیست اپ‌ها روی دستگاه نصب کنید',
  toast_picker_unavailable: 'انتخاب‌گر اپ اینجا در دسترس نیست',

  /* Backup & Sync */
  lib_backup:           'پشتیبان‌گیری و همگام‌سازی',
  backup_title:         'پشتیبان‌گیری و همگام‌سازی',
  backup_intro:         'از دستگاه‌ها و بسته‌های خود نسخهٔ پشتیبان بگیرید، یک پشتیبان قبلی را بازیابی کنید یا آخرین پیکربندی را از گیت‌هاب همگام‌سازی کنید.',
  backup_export:        'تهیهٔ پشتیبان',
  backup_export_sub:    'ذخیرهٔ داده‌های فعلی در Downloads',
  backup_import:        'وارد کردن و بازیابی',
  backup_import_sub:    'بازیابی داده‌ها از یک پشتیبان',
  backup_empty:         'پشتیبانی در Downloads/COPG یافت نشد',
  backup_from_file:     'وارد کردن از فایل…',
  backup_confirm_title: 'بازیابی پشتیبان؟',
  backup_confirm_msg:   'این کار دستگاه‌ها و بسته‌های فعلی شما را با این جایگزین می‌کند:',
  backup_restore:       'بازیابی',
  toast_backup_ok:      'پشتیبان ذخیره شد',
  toast_backup_fail:    'پشتیبان‌گیری ناموفق بود',
  toast_restore_ok:     'پشتیبان بازیابی شد',
  toast_restore_fail:   'بازیابی ناموفق بود',
  toast_invalid_json:   'فایل JSON معتبر نیست',

  /* Import type chooser */
  backup_kind_title:       'این فایل چه نوعی است؟',
  backup_kind_config:      'دستگاه‌ها و بسته‌ها',
  backup_kind_config_sub:  'پیکربندی دستگاه و بسته (COPG.json)',
  backup_kind_list:        'نام‌های نمایشی',
  backup_kind_list_sub:    'نام نمایشی بسته‌ها (list.json)',
  backup_kind_detected:    'تشخیص داده شد',

  /* Sync from GitHub */
  backup_sync:        'همگام‌سازی از گیت‌هاب',
  backup_sync_sub:    'دریافت آخرین پیکربندی از مخزن',
  toast_sync_ok:      'از گیت‌هاب همگام شد',
  toast_sync_fail:    'همگام‌سازی ناموفق بود — اتصال را بررسی کنید',
  toast_sync_preview: 'همگام‌سازی فقط روی دستگاه کار می‌کند',

  /* Package types (chips) */
  pkgtype_device:   'دستگاه',
  pkgtype_cpu_only: 'جعل CPU',
  pkgtype_blocked:  'مسدودسازی جعل CPU',

  /* Package tags (chips) */
  tag_withcpu:  'با جعل CPU',
  tag_got:      'GOT',
  tag_dnd:      'مزاحم نشوید',
  tag_dab:      'روشنایی خودکار',
  tag_kso:      'صفحه روشن',
  tag_nolog:    'بدون لاگ',

  /* Device modal */
  dev_add_title:      'افزودن دستگاه',
  dev_edit_title:     'ویرایش دستگاه',
  dev_f_name:         'نام دستگاه',
  dev_f_brand:        'برند',
  dev_f_model:        'مدل',
  dev_f_manufacturer: 'سازنده',
  dev_f_fingerprint:  'فینگرپرینت',
  dev_f_android:      'نسخه اندروید',
  dev_f_sdk:          'SDK Int',
  dev_f_serial:       'شماره سریال',
  opt_optional:       '(اختیاری)',
  serial_gen:         'تولید',
  info_sdk_title:     'SDK Int',
  info_sdk_msg:       'سطح SDK و نسخهٔ اندروید یک جفت هماهنگ‌اند و این مقدار خودکار از نسخهٔ اندروید پر می‌شود. اگر SDK ناهماهنگ با نسخه بگذاری، بعضی برنامه‌ها این ناهماهنگی را تشخیص می‌دهند و کرش می‌کنند. فقط برای نسخهٔ اندروید جدیدی که ماژول هنوز نمی‌شناسد دستی واردش کن.',

  /* Package modal */
  pkg_add_title:  'افزودن بسته',
  pkg_edit_title: 'ویرایش بسته',
  pkg_f_package:  'نام بسته',
  pkg_f_name:     'نام نمایشی',
  pkg_f_type:     'نوع',
  pkg_f_device:   'پروفایل دستگاه',
  pkg_no_devices: 'دستگاهی نیست — ابتدا یکی اضافه کنید',
  pkg_sec_spoofing: 'جعل',
  pkg_sec_tweaks:   'بهینه‌سازی‌ها',
  pkg_t_withcpu:  'همراه با جعل CPU',
  pkg_t_got:      'هوک GOT',
  pkg_t_dnd:      'مزاحم نشوید',
  pkg_t_dab:      'غیرفعال‌سازی روشنایی خودکار',
  pkg_t_kso:      'روشن نگه‌داشتن صفحه',
  pkg_t_nolog:    'غیرفعال‌سازی لاگ‌گیری',

  /* GOT Hooking warning */
  got_warn_title: 'فعال‌سازی GOT Hooking؟',
  got_warn_msg:   'GOT Hooking باعث باز ماندن ماژول روی حافظهٔ برنامه یا بازی می‌شود و ممکن است باعث تشخیص یا حتی بن‌شدن حساب شما در بازی شود.\n\nآیا مطمئنید می‌خواهید این گزینه را فعال کنید؟ توصیه می‌کنیم پیش از تصمیم با کامیونیتی مشورت کنید.',
  got_warn_ok:    'با این حال فعال کن',
  got_warn_link:  'گروه گفتگوی COPG در تلگرام',

  /* Toggle info popups (the ⓘ buttons) */
  info_aria:           'این چیه؟',
  info_ok:             'متوجه شدم',
  info_withcpu_title:  'همراه با جعل CPU',
  info_withcpu_msg:    'علاوه بر جعل مدل دستگاه، اطلاعات پردازندهٔ (CPU) گوشی را هم برای این برنامه جعل می‌کند. برای بازی‌هایی که چیپست را بررسی می‌کنند روشنش کن. خاموش بگذار (حالت پیش‌فرض) تا جعل CPU مسدود بماند — چیزی که برنامه‌های بانکی و حساس ترجیح می‌دهند.',
  info_got_title:      'هوک GOT',
  info_got_msg:        'روشی عمیق‌تر و تهاجمی‌تر برای اعمال جعل از داخل حافظهٔ برنامه. قوی‌تر اما پرخطرتر: ممکن است شناسایی شود و حتی حساب بازی بن شود. فقط وقتی واقعاً لازم داری فعالش کن.',
  info_dnd_title:      'مزاحم نشوید',
  info_dnd_msg:        'وقتی داخل این بازی هستی تماس‌ها و اعلان‌ها را بی‌صدا می‌کند تا چیزی وسط بازی مزاحمت نشود. با خروج از بازی، تنظیم عادی‌ات برمی‌گردد.',
  info_dab_title:      'غیرفعال‌سازی روشنایی خودکار',
  info_dab_msg:        'جلوی کم و زیاد شدن خودکار روشناییِ صفحه را حین بازی می‌گیرد تا روشنایی همان‌جا که تنظیم کرده‌ای بماند. هنگام خروج از بازی خودکار بازگردانده می‌شود.',
  info_kso_title:      'روشن نگه‌داشتن صفحه',
  info_kso_msg:        'تا وقتی این بازی باز است صفحه را روشن نگه می‌دارد، حتی اگر دست نزنی — دیگر خاموش‌شدن صفحه وسط بازی نداری. با خروج، تایم‌اوت عادی برمی‌گردد.',
  info_nolog_title:    'غیرفعال‌سازی لاگ‌گیری',
  info_nolog_msg:      'جمع‌آورندهٔ لاگ سیستم را حین بازی موقتاً متوقف می‌کند تا کمی CPU و فضای ذخیره آزاد شود. هنگام خروج از بازی خودش دوباره روشن می‌شود.',

  /* Buttons */
  btn_cancel: 'انصراف',
  btn_save:   'ذخیره',
  btn_delete: 'حذف',

  /* Validation messages */
  msg_required:    'الزامی',
  msg_duplicate:   'تکراری',
  msg_dup_device:  'دستگاهی با این نام از قبل وجود دارد',
  msg_dup_model:   'دستگاهی با این مدل از قبل وجود دارد',
  msg_pick_device: 'یک دستگاه انتخاب کنید',
  msg_dup_pkg:     'این بسته از قبل وجود دارد در',

  /* Confirm delete */
  confirm_del_device_title:  'حذف دستگاه؟',
  confirm_del_package_title: 'حذف بسته؟',

  /* Toasts — CRUD */
  toast_device_added:    'دستگاه افزوده شد',
  toast_device_saved:    'دستگاه ذخیره شد',
  toast_device_deleted:  'دستگاه حذف شد',
  toast_package_added:   'بسته افزوده شد',
  toast_package_saved:   'بسته ذخیره شد',
  toast_package_deleted: 'بسته حذف شد',
  toast_preview_only:    'فقط پیش‌نمایش — روی دستگاه ذخیره نشد',
  toast_save_failed:     'ذخیره ناموفق بود',
});
