project('json-c-embedded', 'c')

srcs = [
  'src/json_serializer.c'
]

test_srcs = [
  'test/test_json_serializer.c'
]

cmocka = dependency('cmocka')

test_exe = executable('test_json_serializer', srcs + test_srcs, dependencies: [ cmocka ])
test('test_json_serializer', test_exe)
