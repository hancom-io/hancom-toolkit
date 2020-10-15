subdir('icons')

desktop_autostart = i18n.merge_file ('desktop-autostart',
                                     input: 'kr.hancom.toolkit.autostart.desktop.in',
                                     output: 'kr.hancom.toolkit.autostart.desktop',
                                     install: true,
                                     install_dir: join_paths (get_option('sysconfdir'), 'xdg', 'autostart'),
                                     po_dir: '../po',
                                     type: 'desktop')

#desktop_file = i18n.merge_file(
#  input: 'kr.hancom.toolkit.desktop.in',
#  output: 'kr.hancom.toolkit.desktop',
#  type: 'desktop',
#  po_dir: '../po',
#  install: true,
#  install_dir: join_paths(get_option('datadir'), 'applications')
#)
#
#desktop_utils = find_program('desktop-file-validate', required: false)
#if desktop_utils.found()
#  test('Validate desktop file', desktop_utils,
#    args: [desktop_file]
#  )
#endif

appstream_file = i18n.merge_file(
  input: 'kr.hancom.toolkit.appdata.xml.in',
  output: 'kr.hancom.toolkit.appdata.xml',
  po_dir: '../po',
  install: true,
  install_dir: join_paths(get_option('datadir'), 'appdata')
)

appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test('Validate appstream file', appstream_util,
    args: ['validate', appstream_file]
  )
endif

install_data('kr.hancom.toolkit.policy',
  install_dir: join_paths(get_option('datadir'), 'polkit-1/actions')
)

compile_schemas = find_program('glib-compile-schemas', required: false)
if compile_schemas.found()
  test('Validate schema file', compile_schemas,
    args: ['--strict', '--dry-run', meson.current_source_dir()]
  )
endif
install_data('kr.hancom.toolkit.gschema.xml',
  install_dir: join_paths(get_option('datadir'), 'glib-2.0/schemas')
)


