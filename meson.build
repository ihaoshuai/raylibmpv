project('cvp', 'cpp', 'c')

cppc = meson.get_compiler('cpp')

current_system = host_machine.system()

mpv = cppc.find_library('mpv', dirs: meson.current_source_dir()/'dependencies/mpv/lib')
mpv_incl = include_directories('dependencies/mpv/include')

raylib = cppc.find_library('raylib', dirs: meson.current_source_dir()/'dependencies/raylib/lib64'/current_system)
raylib_incl = include_directories('dependencies/raylib/include')

spdlog = dependency('spdlog')


avformat = dependency('libavformat')
avcodec = dependency('libavcodec')
avswscale = dependency('libswscale')
avutil = dependency('libavutil')


incl = include_directories('include')


incls = [ incl, mpv_incl, raylib_incl ]
deps = [ mpv, raylib, spdlog, avformat, avcodec, avswscale, avutil ]

if current_system == 'linux' 
    x11 = dependency('x11')
    deps += x11
elif current_system == 'windows'
    winim = cppc.find_library('winmm')
    winopengl = cppc.find_library('opengl32')
    gdi32 = cppc.find_library('gdi32')
    deps += [winim, winopengl, gdi32]
endif

src = files(
    'src/main.cpp',
    'src/window.cpp',
    'src/video.cpp',
    'src/ui.cpp',
    'src/thumbnail.cpp',
    'src/screen_idle.cpp',
    'src/screen_video_play.cpp',
    
)

extra_args = []
link_args =  []
if get_option('buildtype') == 'release'
    extra_args += [ '-DRELEASE_BUILD' ]
    link_args += ['-Wl,-rpath,$ORIGIN/lib']
endif

executable(
    'cvp', 
    src, 
    cpp_args: extra_args,
    include_directories: incls, 
    dependencies: deps
)