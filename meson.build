project('zenwm', 'cpp', default_options: ['cpp_std=c++17'])

include_dirs = include_directories('include')
sources = ['src/main.cpp', 'src/core/input.cpp', 'src/core/events.cpp']

executable(
  'zenwm',
  sources,
  dependencies: [dependency('x11')],
  include_directories: include_dirs,
)
