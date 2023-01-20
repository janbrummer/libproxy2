api_version = '1.0'

src_inc = include_directories('.')

libproxy_sources = []
libproxy_link_with = []

if get_option('dbus')
libproxy_sources += [
  'proxy-dbus.c',
]

else
libproxy_sources += [
  'proxy.c',
]

libproxy_link_with += [
  px_backend_lib
]

endif

libproxy_headers = [
  'proxy.h',
]

libproxy_deps = [
  glib_dep,
  gio_dep,
]

libproxy_lib = shared_library('proxy-' + api_version,
  libproxy_sources,
  link_with: libproxy_link_with,
  include_directories: px_backend_inc,
  dependencies: libproxy_deps,
       install: true,
)

libproxy_dep = declare_dependency (
  include_directories: src_inc,
  link_with: libproxy_lib,
  dependencies: libproxy_deps
)

install_headers(libproxy_headers, subdir: 'libproxy')

pkg = import('pkgconfig')
pkg.generate(
  description: 'A shared library for libproxy',
    libraries: libproxy_lib,
         name: 'libproxy',
     filebase: 'libproxy-' + api_version,
      version: meson.project_version(),
      subdirs: 'libproxy',
     requires: 'gio-2.0',
  install_dir: join_paths(get_option('libdir'), 'pkgconfig')
)

gnome = import('gnome')
libproxy_gir = gnome.generate_gir(libproxy_lib,
              sources: libproxy_headers + libproxy_sources,
            nsversion: api_version,
            namespace: 'Libproxy',
        symbol_prefix: 'libproxy',
    identifier_prefix: 'Libproxy',
             includes: [ 'Gio-2.0' ],
              install: true,
)