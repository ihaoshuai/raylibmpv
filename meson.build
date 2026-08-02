project('cvp', 'cpp', 'c')

cppc = meson.get_compiler('cpp')

mpv = cppc.find_library('mpv', dirs: meson.current_source_dir()/'dependencies/mpv/lib')
mpv_incl = include_directories('dependencies/mpv/include')

raylib = cppc.find_library('raylib', dirs: meson.current_source_dir()/'dependencies/raylib/lib64')
raylib_incl = include_directories('dependencies/raylib/include')

spdlog = dependency('spdlog')

x11 = dependency('x11')


incl = include_directories('include')


incls = [ incl, mpv_incl, raylib_incl ]
deps = [ mpv, raylib, spdlog, x11 ]

src = files(
    'src/main.cpp',
    'src/window.cpp',
    'src/video.cpp',
    'src/ui.cpp',
)

executable('cvp', src, include_directories: incls, dependencies: deps)