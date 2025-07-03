project(
	'reinstall', 
	['cpp'],
	version: '6.0.1',
	default_options : ['cpp_std=c++17'],
	license: 'GPL-3.0-or-later',
)

project_description = 'Instalar/reinstalar terminal'
project_domain = 'br.com.bb'

pkg_version = meson.project_version()
version_array = pkg_version.split('.')
pkg_major_version = version_array[0].to_int()
pkg_minor_version = version_array[1].to_int()
pkg_micro_version = version_array[2].to_int()

libversion = '@0@.@1@'.format(pkg_major_version,pkg_minor_version)

cxx = meson.get_compiler('cpp')

subdir('po')

#
# Compiler flags
#
enable_debug = get_option('debug') or get_option('buildtype').contains('debug')

time_epoch = run_command('sh', '-c', 'echo "$SOURCE_DATE_EPOCH"', check : true).stdout().strip()
if time_epoch == ''
  time_epoch = run_command('date','+%Y%m%d', check: true).stdout().strip()
endif

compiler_flags_common = [
	'-DBUILD_DATE='+time_epoch,
]

if enable_debug
  compiler_flags_common += [
    '-ggdb3',
	'-DDEBUG=1'
  ]
else
  compiler_flags_common += [
	'-DNDEBUG=1'
  ]
endif

global_cxxflags = cxx.get_supported_arguments(compiler_flags_common)

add_project_arguments(global_cxxflags, language: 'cpp')

#
# Dependencies
#
libslp = cxx.find_library('slp')
gtkmm = dependency('gtkmm-4.0', required: false)

deps = [
	dependency(
		'libudjat',
		static: true,
		version : '>=2.0.0'
	),
	dependency(
		'libudjathttp',
		version : '>=2.0.0',
		static: true,
	),
	dependency(
		'libudjatdbus',
		static: true,
		version : '>=2.0.0',
	),
	gtkmm,
	dependency('libzip'),
	dependency(
		'libfatfs',
		static: true,
	),
	dependency('zlib'),
	dependency('libisofs-1'),
	libslp,
]

#
# Configuration
#
conf_data = configuration_data()
conf_data.set_quoted('PACKAGE_NAME', meson.project_name())
conf_data.set_quoted('PACKAGE_VERSION', meson.project_version())
conf_data.set_quoted('PACKAGE_DESCRIPTION', project_description)
conf_data.set_quoted('PACKAGE_STRING','@0@ @1@'.format(meson.project_name(), meson.project_version()))
conf_data.set('PACKAGE_DOMAIN','@0@.@1@'.format(project_domain,meson.project_name()))
conf_data.set_quoted('PRODUCT_ID', meson.project_name())
conf_data.set_quoted('MODULE_VERSION',libversion)

conf_data.set('application_description',project_description)
conf_data.set('application_icon','@0@.@1@'.format(project_domain,meson.project_name()))
conf_data.set('application_name', meson.project_name())

conf_data.set('HAVE_FATFS', 1)
conf_data.set('HAVE_ISOFS', 1)
conf_data.set('HAVE_ZLIB', 1)
conf_data.set('HAVE_LIBZIP', 1)

if gtkmm.found()
  conf_data.set('HAVE_GTKMM', 1)
endif

if libslp.found()
  conf_data.set('HAVE_LIBSLP', 1)
endif

if cxx.compiles('#include <unistd.h>', name : 'unistd.h')
  conf_data.set('HAVE_UNISTD_H', 1)
endif

if cxx.compiles('#include <getopt.h>', name : 'getopt.h')
  conf_data.set('HAVE_GETOPT', 1)
endif

configure_file(output : 
	'config.h', 
	configuration : conf_data
)

includes_dir = include_directories('../src/include')

app_src = [
	'src/application/main.cc',
	'src/application/application.cc',
	'src/ui/console/application.cc',
]

if gtkmm.found()
	app_src += [
		'src/ui/gtk4/toplevel.cc',
		'src/ui/gtk4/dialog.cc',
		'src/ui/gtk4/writer.cc',
		'src/ui/gtk4/gtkremovabledevicedialog.cc',
	]
endif

lib_src = [
	'src/library/group.cc',
	'src/library/action.cc',
	'src/library/kernelparameter.cc',
	'src/library/slpclient.cc',
	'src/library/script.cc',
	'src/library/writer.cc',
	'src/library/image.cc',
	'src/library/template.cc',
	'src/library/builder.cc',
	'src/library/efiboot.cc',
	'src/library/ui/dialog.cc',
	'src/library/disk/abstract.cc',
	'src/library/disk/fat.cc',
	'src/library/repository/factory.cc',
	'src/library/repository/repository.cc',
	'src/library/repository/kparm.cc',
	'src/library/source/data.cc',
	'src/library/source/file.cc',
	'src/library/source/load.cc',
	'src/library/source/tempfile.cc',
	'src/library/os/linux/writer.cc',
	'src/modules/grub2.cc',
	'src/modules/isowriter.cc',
	'src/modules/isobuilder/module.cc',
	'src/modules/isobuilder/iso9660/image.cc',
	'src/modules/isobuilder/iso9660/settings.cc',
	'src/modules/isobuilder/fatfs/image.cc',
	'src/modules/isobuilder/fatfs/settings.cc',
]

executable(
	meson.project_name(),
	app_src + lib_src,
	install: true,
	dependencies: deps,
	include_directories: includes_dir
)

custom_target(
	input: 'icons/logo.svg',
	output: '@0@.@1@.svg'.format(project_domain,meson.project_name()),
	install: true,
	install_dir: '/usr/share/icons/hicolor/scalable/apps',
	command: [ 'scour', '@INPUT@', '@OUTPUT@']
)

custom_target(
	input: 'icons/logo.svg',
	output: 'logo.svg',
	install: true,
	install_dir:'/usr/share/@0@/icons'.format(meson.project_name()),
	command: [ 'scour', '@INPUT@', '@OUTPUT@']
)

install_data(
	'stylesheet.css',
	install_dir:'/usr/share/@0@'.format(meson.project_name())
)

install_data(
configure_file(
        input: 'sysconf/launcher.desktop.in',
        output: '@0@.@1@.desktop'.format(project_domain,meson.project_name()), 
        configuration: conf_data,
	),
	install_dir:'/usr/share/applications' 
)

install_data(
configure_file(
        input: 'sysconf/tmpfiles.d.in',
        output: 'tmpfiles', 
        configuration: conf_data,
	),
	rename: meson.project_name(),
	install_dir:'/etc/tmpfiles.d' 
)

install_data(
	[
		'templates/grub.cfg',
		'templates/grub2',
		'templates/isolinux.cfg',
	],
	install_dir:'/usr/share/@0@/templates'.format(meson.project_name())
)

