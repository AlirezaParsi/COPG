/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   es.js — Español (Spanish)
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
I18N.register('es', {
  _meta: { name: 'Español', flag: '🇪🇸', dir: 'ltr' },

  /* Hero */
  hero_module_id: 'Módulo Zygisk',
  hero_sub:       'Suplantación avanzada de dispositivo y apps',
  status_active:   'Activo',
  status_inactive: 'Inactivo',

  /* Section labels */
  label_quick_stats: 'Estadísticas',
  label_system:      'Sistema',
  label_tools:       'Herramientas',
  label_appearance:  'Apariencia',
  label_developer:   'Desarrollador',
  label_about:       'Acerca de',

  /* Quick stats */
  stat_devices:  'Dispositivos',
  stat_packages: 'Paquetes',

  /* System rows */
  sys_android: 'Versión de Android',
  sys_abi:     'ABI',
  sys_zygisk:  'Variante de Zygisk',
  sys_root_version: 'Versión de root',
  sys_kernel:  'Versión del kernel',

  /* Console */
  console_open:    'Abrir consola',
  console_title:   'Consola',
  console_logcat:  'Logcat',
  console_clear:   'Borrar',
  console_copy:    'Copiar',
  console_cmd_ph:  'Escribe un comando de shell…',
  console_copied:  'Copiado al portapapeles',
  console_copy_fail: 'Copiar no disponible aquí',
  console_cleared: 'Consola borrada',
  console_save:      'Guardar',
  console_saved:     'Registro guardado en Download/COPG/LOGS',
  console_saved_dl:  'Registro descargado',
  console_save_empty:'Nada que guardar',
  console_save_fail: 'Error al guardar el registro',

  /* Page headers */
  page_library:  'Biblioteca',
  page_settings: 'Ajustes',

  /* Library tabs + search */
  tab_devices:     'Dispositivos',
  tab_packages:    'Paquetes',
  search_devices:  'Buscar dispositivos…',
  search_packages: 'Buscar paquetes…',
  empty_devices:   'No se encontraron dispositivos',
  empty_packages:  'No se encontraron paquetes',

  /* Settings rows */
  set_theme:        'Tema',
  set_language:     'Idioma',
  set_debug:        'Registros de depuración',

  /* Theme hints */
  theme_auto:   'Auto',
  theme_dark:   'Oscuro',
  theme_light:  'Claro',
  theme_amoled: 'AMOLED',

  /* Sheets */
  sheet_theme_title: 'Tema',
  sheet_lang_title:  'Idioma',
  theme_opt_auto:   'Auto (Sistema)',
  theme_opt_dark:   'Oscuro',
  theme_opt_light:  'Claro',
  theme_opt_amoled: 'Negro AMOLED',

  /* About */
  about_module:  'Módulo COPG',
  about_version: 'Versión',
  about_build:   'Fecha de compilación',
  about_vcode:   'Código de versión',
  about_author:  'Autor',
  about_license: 'Licencia',
  about_desc:    'Un módulo Zygisk que suplanta tu dispositivo — y opcionalmente su CPU — por juego o app, desbloqueando más FPS, mejores gráficos y funciones premium normalmente limitadas a ciertos teléfonos. Añade o quita apps cuando quieras desde esta WebUI, sin reiniciar.',

  /* Nav */
  nav_home:     'Inicio',
  nav_library:  'Biblioteca',
  nav_settings: 'Ajustes',

  /* Toasts */
  toast_console:   'Consola — próximamente',
  toast_debug_on:  'Registros de depuración activados',
  toast_debug_off: 'Registros de depuración desactivados',
  status_connected: 'Conectado',
  status_offline:   'Sin conexión',

  /* Library — add buttons & card meta */
  add_device:   'Añadir dispositivo',
  add_package:  'Añadir paquete',
  add_short:    'Añadir',
  act_edit:     'Editar',
  act_delete:   'Eliminar',
  dev_model:    'Modelo',
  dev_games:    'juegos',
  dev_apps:     'apps',
  badge_installed: 'Instalada',
  preview_unsaved: 'Modo vista previa — los cambios no se guardan en el dispositivo.',

  /* System — root */
  sys_root: 'Root',

  /* Sort + filter */
  sort_title:     'Ordenar por',
  sort_default:   'Predeterminado',
  sort_name:      'Nombre (A→Z)',
  sort_name_za:   'Nombre (Z→A)',
  sort_apps:      'Nº de apps',
  sort_brand:     'Marca',
  sort_device:    'Dispositivo',
  sort_type:      'Tipo',
  sort_installed: 'Instaladas primero',
  filter_all:      'Todas',
  filter_user:     'Usuario',
  filter_system:   'Sistema',
  filter_installed:'Instaladas',
  filter_blocked:  'Bloqueadas',
  filter_cpu_only: 'CPU',

  /* App picker */
  pick_from_installed: 'Elegir de las apps instaladas',
  picker_title:   'Apps instaladas',
  picker_search:  'Buscar apps…',
  picker_empty:   'No hay apps instaladas',
  picker_no_results: 'No se encontraron apps',
  picker_preview: 'Modo vista previa — instala en el dispositivo para ver apps',
  toast_picker_unavailable: 'Selector de apps no disponible aquí',

  /* Backup & Sync */
  lib_backup:           'Copia y sincronización',
  backup_title:         'Copia y sincronización',
  backup_intro:         'Haz copia de tus dispositivos y paquetes, restaura una copia anterior o sincroniza la última configuración desde GitHub.',
  backup_export:        'Exportar copia',
  backup_export_sub:    'Guardar los datos actuales en Descargas',
  backup_import:        'Importar y restaurar',
  backup_import_sub:    'Restaurar datos desde una copia',
  backup_empty:         'No hay copias en Downloads/COPG',
  backup_from_file:     'Importar desde archivo…',
  backup_confirm_title: '¿Restaurar copia?',
  backup_confirm_msg:   'Esto sobrescribirá tus dispositivos y paquetes actuales con:',
  backup_restore:       'Restaurar',
  toast_backup_ok:      'Copia guardada',
  toast_backup_fail:    'Error en la copia',
  toast_restore_ok:     'Copia restaurada',
  toast_restore_fail:   'Error al restaurar',
  toast_invalid_json:   'Archivo JSON no válido',

  /* Import type chooser */
  backup_kind_title:       '¿Qué tipo de archivo es?',
  backup_kind_config:      'Dispositivos y paquetes',
  backup_kind_config_sub:  'Config de dispositivos y paquetes (COPG.json)',
  backup_kind_list:        'Nombres visibles',
  backup_kind_list_sub:    'Nombres visibles de paquetes (list.json)',
  backup_kind_detected:    'detectado',

  /* Sync from GitHub */
  backup_sync:        'Sincronizar desde GitHub',
  backup_sync_sub:    'Obtener la última configuración del repositorio',
  toast_sync_ok:      'Sincronizado desde GitHub',
  toast_sync_fail:    'Error de sincronización — revisa tu conexión',
  toast_sync_preview: 'La sincronización solo funciona en el dispositivo',

  /* Package types (chips) */
  pkgtype_device:   'Dispositivo',
  pkgtype_cpu_only: 'Suplantar CPU',
  pkgtype_blocked:  'Bloquear suplantación de CPU',

  /* Package tags (chips) */
  tag_withcpu:  'Con CPU',
  tag_got:      'GOT',
  tag_dnd:      'No molestar',
  tag_dab:      'Brillo auto',
  tag_kso:      'Pantalla on',
  tag_nolog:    'Sin log',

  /* Device modal */
  dev_add_title:      'Añadir dispositivo',
  dev_edit_title:     'Editar dispositivo',
  dev_f_name:         'Nombre del dispositivo',
  dev_f_brand:        'Marca',
  dev_f_model:        'Modelo',
  dev_f_manufacturer: 'Fabricante',
  dev_f_fingerprint:  'Huella (fingerprint)',
  dev_f_android:      'Versión de Android',
  dev_f_sdk:          'SDK Int',
  dev_f_serial:       'Número de serie',
  opt_optional:       '(opcional)',
  serial_gen:         'Generar',
  info_sdk_title:     'SDK Int',
  info_sdk_msg:       'El nivel de SDK y la versión de Android van emparejados — se rellena automáticamente desde la versión de Android. Si pones un SDK que no coincide, algunas apps detectan el desajuste y se cierran. Edítalo a mano solo para una versión de Android nueva que el módulo aún no conozca.',

  /* Package modal */
  pkg_add_title:  'Añadir paquete',
  pkg_edit_title: 'Editar paquete',
  pkg_f_package:  'Nombre del paquete',
  pkg_f_name:     'Nombre visible',
  pkg_f_type:     'Tipo',
  pkg_f_device:   'Perfil de dispositivo',
  pkg_no_devices: 'Sin dispositivos — añade uno primero',
  pkg_sec_spoofing: 'Suplantación',
  pkg_sec_tweaks:   'Ajustes',
  pkg_t_withcpu:  'Con suplantación de CPU',
  pkg_t_got:      'GOT Hooking',
  pkg_t_dnd:      'No molestar',
  pkg_t_dab:      'Desactivar brillo automático',
  pkg_t_kso:      'Mantener pantalla encendida',
  pkg_t_nolog:    'Desactivar registro',

  /* GOT Hooking warning */
  got_warn_title: '¿Activar GOT Hooking?',
  got_warn_msg:   'GOT Hooking mantiene el módulo residente en la memoria de la app o juego. Esto puede detectarse e incluso provocar el baneo de tu cuenta del juego.\n\n¿Seguro que quieres activarlo? Te recomendamos preguntar a la comunidad antes de decidir.',
  got_warn_ok:    'Activar de todos modos',
  got_warn_link:  'Comunidad COPG en Telegram',

  /* Toggle info popups */
  info_aria:           '¿Qué es esto?',
  info_ok:             'Entendido',
  info_withcpu_title:  'Con suplantación de CPU',
  info_withcpu_msg:    'Además de falsear el modelo del dispositivo, esto también falsea los detalles del procesador (CPU) de tu teléfono para esta app. Actívalo para juegos que comprueban el chipset. Déjalo desactivado (por defecto) y la suplantación de CPU queda bloqueada — lo que prefieren las apps bancarias y sensibles.',
  info_got_title:      'GOT Hooking',
  info_got_msg:        'Una forma más profunda y agresiva de aplicar la suplantación desde dentro de la memoria de la app. Más potente pero más arriesgada: puede detectarse e incluso provocar el baneo de una cuenta de juego. Actívala solo si sabes que la necesitas.',
  info_dnd_title:      'No molestar',
  info_dnd_msg:        'Silencia llamadas y notificaciones mientras estás en este juego, para que nada interrumpa tu partida. Tu ajuste normal vuelve al salir.',
  info_dab_title:      'Desactivar brillo automático',
  info_dab_msg:        'Evita que la pantalla se atenúe y se ilumine sola mientras juegas, manteniendo el brillo que fijaste. Se restaura automáticamente al salir del juego.',
  info_kso_title:      'Mantener pantalla encendida',
  info_kso_msg:        'Mantiene la pantalla activa mientras este juego está abierto, aunque no la toques — sin apagones a mitad de la partida. Tu tiempo de espera normal vuelve al salir.',
  info_nolog_title:    'Desactivar registro',
  info_nolog_msg:      'Pausa el recolector de registros del sistema mientras juegas para liberar algo de CPU y almacenamiento. Se reactiva solo al salir del juego.',

  /* Buttons */
  btn_cancel: 'Cancelar',
  btn_save:   'Guardar',
  btn_delete: 'Eliminar',

  /* Validation messages */
  msg_required:    'Obligatorio',
  msg_duplicate:   'Duplicado',
  msg_dup_device:  'Ya existe un dispositivo con este nombre',
  msg_dup_model:   'Ya existe un dispositivo con este modelo',
  msg_pick_device: 'Selecciona un dispositivo',
  msg_dup_pkg:     'Este paquete ya existe en',

  /* Confirm delete */
  confirm_del_device_title:  '¿Eliminar dispositivo?',
  confirm_del_package_title: '¿Eliminar paquete?',

  /* Toasts — CRUD */
  toast_device_added:    'Dispositivo añadido',
  toast_device_saved:    'Dispositivo guardado',
  toast_device_deleted:  'Dispositivo eliminado',
  toast_package_added:   'Paquete añadido',
  toast_package_saved:   'Paquete guardado',
  toast_package_deleted: 'Paquete eliminado',
  toast_preview_only:    'Solo vista previa — no guardado en el dispositivo',
  toast_save_failed:     'Error al guardar',
});
