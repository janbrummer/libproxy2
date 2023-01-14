if get_option('plugin_config_gnome')

plugin_c = [
  'config-gnome.c',
]

plugin_data = [
  'config-gnome.plugin',
]

plugin_lib = shared_module(
  'config-gnome',
  plugin_c,
  include_directories: px_backend_inc,
  dependencies: [peas_dep],
  install_dir: join_paths(px_module_dir, 'config-gnome'),
  install: true,
)

custom_target(
  'lib@0@-data'.format('config-gnome'),
  input: plugin_data,
  output: plugin_data,
  command: [copyfile_script, '@INPUT@', '@OUTDIR@'],
  build_by_default: true,
  install_dir: join_paths(px_module_dir, 'config-gnome'),
  install: true,
)

endif