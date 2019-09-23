#
# Copyright (c) 2019, Breakaway Consulting Pty. Ltd.
#

import os
from shutil import which
from prj import execute, commonpath, SystemBuildError


def run(system, _=None):
    return system_build(system)


def system_build(system):
    prefixes = ['aarch64-linux-gnu-', 'aarch64-elf-']
    for prefix in prefixes:
        if which(prefix + 'gcc') is not None:
            break
    else:
        raise SystemBuildError("Unable to find toolchain. Tried: {}".format(' '.join(prefixes)))
    
    inc_path_args = ['-I%s' % i for i in system.include_paths]

    common_flags = ['-g3', '-mlittle-endian', '-mcpu=cortex-a53']

    a_flags = common_flags
    c_flags = common_flags  + ['-Os', '-fno-pic',  '-mstrict-align']

    all_input_files = system.c_files + system.asm_files
    all_input_files = [os.path.normpath(os.path.abspath(path)) for path in all_input_files]
    common_parent_path = commonpath(all_input_files)

    # Compile all C files.
    c_obj_files = [os.path.join(system.output, os.path.relpath(os.path.abspath(c_file_path.replace('.c', '.o')),
                                                               common_parent_path)) for c_file_path in system.c_files]

    for c_path, obj_path in zip(system.c_files, c_obj_files):
        os.makedirs(os.path.dirname(obj_path), exist_ok=True)
        execute([prefix + 'gcc', '-ffreestanding', '-c', c_path, '-o', obj_path, '-Wall', '-Werror', '-Wno-unused-function'] +
                c_flags + inc_path_args)

    # Assemble all asm files.
    asm_obj_files = [os.path.join(system.output, os.path.relpath(os.path.abspath(asm_file_path.replace('.s', '.o')),
                                                                 common_parent_path))
                     for asm_file_path in system.asm_files]
    for asm_file_path, obj_file_path in zip(system.asm_files, asm_obj_files):
        os.makedirs(os.path.dirname(obj_file_path), exist_ok=True)
        execute([prefix + 'as', '-o', obj_file_path, asm_file_path] + a_flags + inc_path_args)

    # Perform final link
    obj_files = asm_obj_files + c_obj_files
    system_elf_file = "{}.elf".format(system.output_file)
    execute([prefix + 'ld', '-T', system.linker_script, '-o', system_elf_file] + obj_files)

    # Raspberry PI wants to boot a raw binary file
    execute([prefix + 'objcopy', system_elf_file, '-O', 'binary', system.output_file])
