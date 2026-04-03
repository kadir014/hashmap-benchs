project(
    'hashmaps_bench',
    ['c', 'cpp'],
    license: 'MIT',
    default_options: [
        'c_std=c99',
        'cpp_std=c++14',
        'warning_level=1',
        'default_library=static'
    ]
)


compiler = meson.get_compiler('cpp')

cpp_args = []

if compiler.get_id() == 'msvc'
    c_args = ['/arch:AVX2', '/D_CRT_SECURE_NO_WARNINGS']
    link_args = []

    c_args += ['/wd4305']

else
    # -march=mavx2 ?
    c_args = []
    link_args = ['-lm']

endif


examples_src = [
    'src/main.cpp',
    'src/astropath.c',
    'src/nova.c'
]
examples_includes = ['src']

executable(
    'main',
    sources: examples_src,
    include_directories: examples_includes,
    c_args: c_args,
    cpp_args: c_args,
    link_args: link_args
)