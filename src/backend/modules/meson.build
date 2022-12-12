api_version = '0.1'

module_deps = [
  dependency('gio-2.0'),
  dependency('gmodule-2.0'),
  ws2_32_dep,
]

env_sources = [
  'config-env.c',
  '../px-module.c'
]

env_module = shared_module('config_env',
                             env_sources,
                             dependencies: module_deps,
                             include_directories: ['..'],
                             install: true,
                             install_dir: px_module_dir,
                           )

gnome_sources = [
  'config-gnome.c',
  '../px-module.c'
]

gnome_module = shared_module('config_gnome',
                             gnome_sources,
                             dependencies: module_deps,
                             include_directories: ['..'],
                             install: true,
                             install_dir: px_module_dir,
                             )

duktape_sources = [
  'pacrunner-duktape.c',
  '../px-module.c'
]

duktape_deps = [
  module_deps,
  dependency('duktape'),
  cc.find_library('m', required : false)
]

duktape_module = shared_module('pacrunner_duktape',
                             duktape_sources,
                             dependencies: duktape_deps,
                             include_directories: ['..'],
                             install: true,
                             install_dir: px_module_dir,
                           )