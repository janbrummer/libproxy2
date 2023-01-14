if get_option('plugin_config_env')

plugin_c = [
  'config-env.c',
]

plugin_data = [
  'config-env.plugin',
]

plugin_lib = shared_module(
  'config-env',
  plugin_c,
  include_directories: px_backend_inc,
  # link_with: px_backend_lib_dyn,
  dependencies: [peas_dep],
  install_dir: join_paths(px_module_dir, 'config-env'),
  install: true,
)

custom_target(
  'lib@0@-data'.format('config-env'),
  input: plugin_data,
  output: plugin_data,
  command: [copyfile_script, '@INPUT@', '@OUTDIR@'],
  build_by_default: true,
  install_dir: join_paths(px_module_dir, 'config-env'),
  install: true,
)

endif