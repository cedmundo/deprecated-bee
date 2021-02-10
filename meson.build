project('bee', 'c')

jit_dep = dependency('libjit')
bee_exe = executable('bee', 'bee.c', install : true, dependencies : [ jit_dep ])

test('Bee runs all programs', find_program('test.sh', required : true))
