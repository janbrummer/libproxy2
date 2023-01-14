if get_option('plugin_pacrunner_duktape')

duktape_dep = dependency('duktape')

plugin_c = [
  'pacrunner-duktape.c',
]

plugin_data = [
  'pacrunner-duktape.plugin',
]

plugin_lib = shared_module(
  'pacrunner-duktape',
  plugin_c,
  include_directories: px_backend_inc,
  # link_with: px_backend_lib,
  dependencies: [peas_dep, duktape_dep],
  install_dir: join_paths(px_module_dir, 'pacrunner-duktape'),
  install: true,
)

custom_target(
  'lib@0@-data'.format('pacrunner-duktape'),
  input: plugin_data,
  output: plugin_data,
  command: [copyfile_script, '@INPUT@', '@OUTDIR@'],
  build_by_default: true,
  install_dir: join_paths(px_module_dir, 'pacrunner-duktape'),
  install: true,
)

endif