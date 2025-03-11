project('zenwm', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])

include_dirs = include_directories('include')

sources = [ 'src/main.cpp' ]

executable('zenwm', sources,
  dependencies: [dependency('x11')],
  include_directories: include_dirs
)
