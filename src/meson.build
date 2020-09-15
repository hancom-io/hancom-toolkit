hancom_toolkit_sources = [
  'utils.c',
  'main.c',
  'htoolkit-application.c',
  'htoolkit-window.c',
  'htoolkit-app.c',
  'htoolkit-app-row.c',
  'htoolkit-app-list.c',
  'htoolkit-controller.c',
]

hancom_toolkit_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('glib-2.0',    version: '>=2.56.0'),
  dependency('json-glib-1.0', version : '>= 1.2.0'),
  dependency('libcurl')
]

gnome = import('gnome')

resources = gnome.compile_resources(
  'hancom-toolkit-resources',
  join_paths(
    'resources', 'hancom-toolkit.gresource.xml'
  ),
  source_dir: 'resources',
  c_name: 'hancom_toolkit',
)

hancom_toolkit_sources += resources[0]

executable('hancom-toolkit', hancom_toolkit_sources,
  dependencies: hancom_toolkit_deps,
  install: true,
  link_args: '-rdynamic'
)

install_data('htoolkit-check',
             install_dir : join_paths(get_option('libdir'), 'hancom-toolkit'))

install_data('htoolkit-install',
             install_dir : join_paths(get_option('libdir'), 'hancom-toolkit'))
