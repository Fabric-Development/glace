gnome = import('gnome')
pkg_config = import('pkgconfig')

glace_version = meson.project_version()
glace_description = 'a library for managing wayland clients'
glace_url = 'https://github.com/Fabric-Development/glace'

glace_library_name = 'lib' + meson.project_name()

glace_namespace = 'Glace'
glace_package_name = glace_namespace + '-' + glace_version

glace_public_src = files(
    'libglace-manager.c',
    'libglace-client.c',
    'libglace.c',
)

glace_private_src = files(
    'libglace-manager-private.c',
    'libglace-client-private.c',
)

glace_sources = glace_public_src + glace_private_src

glace_public_headers = files(
    'libglace-manager.h',
    'libglace-client.h',
    'libglace.h',
)

glace_private_headers = files(
    'libglace-manager-private.h',
    'libglace-client-private.h',
    'libglace-private.h',
)

glace_headers = glace_public_headers + glace_private_headers

dependencies = [
    dependency('glib-2.0'),
    dependency('gtk+-3.0'),
    dependency('gobject-2.0'),
    dependency('gdk-wayland-3.0'),
]

all_dependencies = [
    wayland_dep,
    toplevel_management_dep
] + dependencies

libglace = library(
    glace_library_name,
    glace_sources,
    dependencies: all_dependencies,
    install: true,
)

glace_gir = gnome.generate_gir(
  libglace,
  header: 'libglace.h',
  sources: glace_public_src + glace_public_headers,
  namespace: glace_namespace,
  nsversion: glace_version,
  identifier_prefix: glace_namespace,
  includes: ['GObject-2.0', 'Gtk-3.0'],
  dependencies: dependencies,
  header: 'libglace/libglace.h',
  symbol_prefix: 'glace',
  install: true,
)


libgpaste_vapi = gnome.generate_vapi(
    glace_package_name,
    sources: [glace_gir[0]],
    packages: [ 'gtk+-3.0', 'gio-2.0', 'glib-2.0', 'gobject-2.0' ],
    install: true,
)

install_headers(
    glace_headers ,
    subdir: glace_library_name,   
)

pkg_config.generate(
    name: meson.project_name(),
    filebase: glace_library_name,
    version: meson.project_version(),
    libraries: libglace,
    requires: dependencies,
    description: glace_description,
    url: glace_url,
)
