project('zenwm', 'cpp', default_options : ['cpp_std=c++17'])

sources = ['src/main.cpp']
include_dirs = include_directories('include')

executable('zenwm', sources,
  dependencies: [dependency('x11')],
  include_directories: include_dirs
)
