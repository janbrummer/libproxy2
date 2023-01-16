api_version = '1.0'

px_backend_sources = [
  'px-manager.c',
  'px-manager.h',
  'px-plugin-config.c',
  'px-plugin-config.h',
  'px-plugin-pacrunner.c',
  'px-plugin-pacrunner.h',
]

px_backend_deps = [
  gio_dep,
  glib_dep,
  soup_dep,
  peas_dep,
]

px_backend_inc = include_directories('.')

px_backend_lib = static_library('pxbackend',
                                 px_backend_sources,
                                 dependencies: px_backend_deps,
                                 install: false)

# Also create shared library for gir generation on msys
px_backend_lib_dyn = shared_library('pxbackend',
                                 px_backend_sources,
                                 dependencies: px_backend_deps,
                                 install: true)

gnome = import('gnome')
px_gir = gnome.generate_gir(px_backend_lib_dyn,
              sources: px_backend_sources,
            nsversion: api_version,
            namespace: 'Libproxy',
        symbol_prefix: 'px',
    identifier_prefix: 'Px',
             includes: [ 'Gio-2.0' ],
              install: true,
)

libpxbackend_dep = declare_dependency(
  include_directories: px_backend_inc,
  link_with: px_backend_lib_dyn,
  dependencies: px_backend_deps
)

subdir('plugins')

if get_option('dbus')
  gdbus_codegen = find_program('gdbus-codegen')

  px_interface = [
    'org.libproxy.proxy.xml'
  ]

  px_interface_h = custom_target(
    'px-interface.h',
    input: px_interface,
    output: ['px-interface.h'],
    command: [gdbus_codegen,
              '--interface-info-header',
              '--output', '@OUTPUT@',
              '@INPUT@']
  )

  px_interface_c = custom_target(
    'libproxy-iface.c',
    input: px_interface,
    output: ['px-interface.c'],
    command: [gdbus_codegen,
              '--interface-info-body',
              '--output', '@OUTPUT@',
              '@INPUT@'],
  )

  proxyd_sources = [
    px_interface_c,
    px_interface_h,
    'dbus.c',
  ]

  proxyd_deps = [
    libpxbackend_dep
  ]

  executable('proxyd', proxyd_sources, dependencies: proxyd_deps, install: true)
endif
