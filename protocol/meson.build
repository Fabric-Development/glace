wayland_dep = dependency('wayland-client')

protocol_name = 'wlr-foreign-toplevel-management-unstable-v1'
protocol_file = files(protocol_name + '.xml')

wayland_scanner_dep = dependency('wayland-scanner', native: true)
wayland_scanner = find_program(
	wayland_scanner_dep.get_variable(
		pkgconfig: 'wayland_scanner'
	)
)

protocol_sources = []
protocol_sources += custom_target(
	protocol_name + '.h',
	command: [ wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@' ],
	input: protocol_file,
	output: protocol_name + '.h',
)

output_type = 'private-code'
if wayland_scanner_dep.version().version_compare('< 1.14.91')
    output_type = 'code'
endif

protocol_sources += custom_target(
	protocol_name + '.c',
	command: [ wayland_scanner, output_type, '@INPUT@', '@OUTPUT@' ],
	input: protocol_file,
	output: protocol_name + '.c',
)

toplevel_management_dep = declare_dependency(
    dependencies: [
        wayland_dep,
    ],
    include_directories: include_directories('.'),
    sources: protocol_sources
)
