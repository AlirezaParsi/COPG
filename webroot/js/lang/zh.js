/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   zh.js — 中文 (Simplified Chinese)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
I18N.register('zh', {
  _meta: { name: '中文', flag: '🇨🇳', dir: 'ltr' },

  /* Hero */
  hero_module_id: 'Zygisk 模块',
  hero_sub:       '高级设备与应用伪装',
  status_active:   '已启用',
  status_inactive: '未启用',

  /* Section labels */
  label_quick_stats: '快速统计',
  label_system:      '系统',
  label_tools:       '工具',
  label_appearance:  '外观',
  label_developer:   '开发者',
  label_about:       '关于',

  /* Quick stats */
  stat_devices:  '设备',
  stat_packages: '应用包',

  /* System rows */
  sys_android: 'Android 版本',
  sys_abi:     'ABI',
  sys_zygisk:  'Zygisk 类型',
  sys_root_version: 'Root 版本',
  sys_kernel:  '内核版本',

  /* Console */
  console_open:    '打开控制台',
  console_title:   '控制台',
  console_logcat:  'Logcat',
  console_clear:   '清除',
  console_copy:    '复制',
  console_cmd_ph:  '输入 shell 命令…',
  console_copied:  '已复制到剪贴板',
  console_copy_fail: '此处无法复制',
  console_cleared: '控制台已清除',
  console_save:      '保存',
  console_saved:     '日志已保存到 Download/COPG/LOGS',
  console_saved_dl:  '日志已下载',
  console_save_empty:'没有可保存的内容',
  console_save_fail: '保存日志失败',

  /* Page headers */
  page_library:  '库',
  page_settings: '设置',

  /* Library tabs + search */
  tab_devices:     '设备',
  tab_packages:    '应用包',
  search_devices:  '搜索设备…',
  search_packages: '搜索应用包…',
  empty_devices:   '未找到设备',
  empty_packages:  '未找到应用包',

  /* Settings rows */
  set_theme:        '主题',
  set_language:     '语言',
  set_fullscreen:   '全屏',
  set_debug:        '调试日志',

  /* Theme hints */
  theme_auto:   '自动',
  theme_dark:   '深色',
  theme_light:  '浅色',
  theme_amoled: 'AMOLED',

  /* Sheets */
  sheet_theme_title: '主题',
  sheet_lang_title:  '语言',
  theme_opt_auto:   '自动（系统）',
  theme_opt_dark:   '深色',
  theme_opt_light:  '浅色',
  theme_opt_amoled: 'AMOLED 纯黑',

  /* About */
  about_module:  'COPG 模块',
  about_version: '版本',
  about_build:   '构建日期',
  about_vcode:   '版本号',
  about_author:  '作者',
  about_license: '许可证',
  about_desc:    '一个 Zygisk 模块，可按游戏或应用伪装你的设备——并可选伪装其 CPU，从而解锁更高帧率、更好画质，以及通常仅限特定机型的高级功能。随时可从此 WebUI 添加或移除应用，无需重启。',

  /* Nav */
  nav_home:     '主页',
  nav_library:  '库',
  nav_settings: '设置',

  /* Toasts */
  toast_console:   '控制台——即将推出',
  toast_debug_on:  '已启用调试日志',
  toast_debug_off: '已禁用调试日志',
  toast_fullscreen_on:  '全屏已开启 — 状态栏已隐藏',
  toast_fullscreen_off: '全屏已关闭 — 状态栏已显示',
  status_connected: '已连接',
  status_offline:   '离线',

  /* Library — add buttons & card meta */
  add_device:   '添加设备',
  add_package:  '添加应用包',
  add_short:    '添加',
  act_edit:     '编辑',
  act_delete:   '删除',
  dev_model:    '型号',
  dev_games:    '游戏',
  dev_apps:     '应用',
  badge_installed: '已安装',
  preview_unsaved: '预览模式——更改不会保存到设备。',

  /* System — root */
  sys_root: 'Root',

  /* Sort + filter */
  sort_title:     '排序方式',
  sort_default:   '默认',
  sort_name:      '名称 (A→Z)',
  sort_name_za:   '名称 (Z→A)',
  sort_apps:      '应用数量',
  sort_brand:     '品牌',
  sort_device:    '设备',
  sort_type:      '类型',
  sort_installed: '已安装优先',
  filter_all:      '全部',
  filter_user:     '用户',
  filter_system:   '系统',
  filter_installed:'已安装',
  filter_blocked:  '已屏蔽',
  filter_cpu_only: 'CPU',

  /* App picker */
  pick_from_installed: '从已安装应用中选择',
  picker_title:   '已安装应用',
  picker_search:  '搜索应用…',
  picker_empty:   '未找到已安装应用',
  picker_no_results: '未找到应用',
  picker_preview: '预览模式——请在设备上安装以列出应用',
  toast_picker_unavailable: '此处无法使用应用选择器',

  /* Backup & Sync */
  lib_backup:           '备份与同步',
  backup_title:         '备份与同步',
  backup_intro:         '备份你的设备和应用包，恢复以前的备份，或从 GitHub 同步最新配置。',
  backup_export:        '导出备份',
  backup_export_sub:    '将当前数据保存到下载',
  backup_import:        '导入并恢复',
  backup_import_sub:    '从备份恢复数据',
  backup_empty:         '在 Downloads/COPG 中未找到备份',
  backup_from_file:     '从文件导入…',
  backup_confirm_title: '恢复备份？',
  backup_confirm_msg:   '这将用以下内容覆盖你当前的设备和应用包：',
  backup_restore:       '恢复',
  toast_backup_ok:      '备份已保存',
  toast_backup_fail:    '备份失败',
  toast_restore_ok:     '备份已恢复',
  toast_restore_fail:   '恢复失败',
  toast_invalid_json:   '不是有效的 JSON 文件',

  /* Import type chooser */
  backup_kind_title:       '这是什么类型的文件？',
  backup_kind_config:      '设备与应用包',
  backup_kind_config_sub:  '设备与应用包配置 (COPG.json)',
  backup_kind_list:        '显示名称',
  backup_kind_list_sub:    '应用包显示名称 (list.json)',
  backup_kind_detected:    '已检测',

  /* Sync from GitHub */
  backup_sync:        '从 GitHub 同步',
  backup_sync_sub:    '从仓库获取最新配置',
  toast_sync_ok:      '已从 GitHub 同步',
  toast_sync_fail:    '同步失败——请检查网络连接',
  toast_sync_preview: '同步仅在设备上可用',

  /* Package types (chips) */
  pkgtype_device:   '设备',
  pkgtype_cpu_only: 'CPU 伪装',
  pkgtype_blocked:  '屏蔽 CPU 伪装',

  /* Package tags (chips) */
  tag_withcpu:  '含 CPU',
  tag_got:      'GOT',
  tag_dnd:      '勿扰',
  tag_dab:      '自动亮度',
  tag_kso:      '屏幕常亮',
  tag_nolog:    '无日志',

  /* Device modal */
  dev_add_title:      '添加设备',
  dev_edit_title:     '编辑设备',
  dev_f_name:         '设备名称',
  dev_f_brand:        '品牌',
  dev_f_model:        '型号',
  dev_f_manufacturer: '制造商',
  dev_f_fingerprint:  '指纹 (fingerprint)',
  dev_f_android:      'Android 版本',
  dev_f_sdk:          'SDK Int',
  dev_f_serial:       '序列号',
  opt_optional:       '（可选）',
  serial_gen:         '生成',
  info_sdk_title:     'SDK Int',
  info_sdk_msg:       'SDK 级别与 Android 版本是配对的——它会根据 Android 版本自动填充。如果设置了与版本不匹配的 SDK，部分应用会检测到不一致并崩溃。仅当模块尚不认识某个全新 Android 版本时才手动编辑。',

  /* Package modal */
  pkg_add_title:  '添加应用包',
  pkg_edit_title: '编辑应用包',
  pkg_f_package:  '包名',
  pkg_f_name:     '显示名称',
  pkg_f_type:     '类型',
  pkg_f_device:   '设备配置',
  pkg_no_devices: '没有设备——请先添加一个',
  pkg_sec_spoofing: '伪装',
  pkg_sec_tweaks:   '微调',
  pkg_t_withcpu:  '伪装 CPU',
  pkg_t_got:      'GOT Hooking',
  pkg_t_dnd:      '勿扰模式',
  pkg_t_dab:      '关闭自动亮度',
  pkg_t_kso:      '保持屏幕常亮',
  pkg_t_nolog:    '关闭日志记录',

  /* GOT Hooking warning */
  got_warn_title: '启用 GOT Hooking？',
  got_warn_msg:   'GOT Hooking 会让模块常驻于应用或游戏的内存中。这可能被检测到，甚至可能导致你的游戏账号被封。\n\n确定要启用吗？建议在决定前先咨询社区。',
  got_warn_ok:    '仍然启用',
  got_warn_link:  'Telegram 上的 COPG 社区',

  /* Toggle info popups */
  info_aria:           '这是什么？',
  info_ok:             '知道了',
  info_withcpu_title:  '伪装 CPU',
  info_withcpu_msg:    '在伪装设备型号之外，这还会为此应用伪装手机的处理器 (CPU) 信息。对于会检查芯片的游戏请开启。保持关闭（默认）则 CPU 伪装被屏蔽——银行和敏感应用更喜欢这样。',
  info_got_title:      'GOT Hooking',
  info_got_msg:        '一种更深入、更激进的方式，从应用内存内部应用伪装。更强但更冒险：可能被检测到，甚至可能导致游戏账号被封。仅在你确定需要时才启用。',
  info_dnd_title:      '勿扰模式',
  info_dnd_msg:        '在你玩此游戏期间静音来电和通知，让任何东西都不会打断你的对局。退出后恢复你的常规设置。',
  info_dab_title:      '关闭自动亮度',
  info_dab_msg:        '游戏时阻止屏幕自动变暗或变亮，让亮度保持在你设定的值。退出游戏后自动恢复。',
  info_kso_title:      '保持屏幕常亮',
  info_kso_msg:        '此游戏打开期间保持屏幕常亮，即使你不触碰——不再在对局中途熄屏。退出后恢复你的常规超时。',
  info_nolog_title:    '关闭日志记录',
  info_nolog_msg:      '游戏时暂停系统日志收集器，以释放少量 CPU 和存储。退出游戏后自动重新开启。',

  /* Buttons */
  btn_cancel: '取消',
  btn_save:   '保存',
  btn_delete: '删除',

  /* Validation messages */
  msg_required:    '必填',
  msg_duplicate:   '重复',
  msg_dup_device:  '已存在同名设备',
  msg_dup_model:   '已存在同型号设备',
  msg_pick_device: '请选择设备',
  msg_dup_pkg:     '此应用包已存在于',

  /* Confirm delete */
  confirm_del_device_title:  '删除设备？',
  confirm_del_package_title: '删除应用包？',

  /* Toasts — CRUD */
  toast_device_added:    '已添加设备',
  toast_device_saved:    '已保存设备',
  toast_device_deleted:  '已删除设备',
  toast_package_added:   '已添加应用包',
  toast_package_saved:   '已保存应用包',
  toast_package_deleted: '已删除应用包',
  toast_preview_only:    '仅预览——未保存到设备',
  toast_save_failed:     '保存失败',
});
