#
# Copyright (c) 2019, Breakaway Consulting Pty. Ltd.
#

from prj import SystemBuildError, Module
import ply.cpp


class EntryModule(Module):
    xml_schema = """
<schema>
    <entry name="code_addr" type="int" default="0x80000" />
    <entry name="stack_size" type="int" default="0x1000" />
    <entry name="platform_init" type="c_ident" default="" />
</schema>"""

    files = [
        {'input': 'startup.s', 'render': True, 'type': 'asm'},
        {'input': 'default.ld', 'render': True, 'type': 'linker_script', 'stage': 'post_prepare'},
    ]

    def post_prepare(self, system, config):

        pre_processor = ply.cpp.Preprocessor(include_paths=system.include_paths)

        for c_file in system.c_files:
            with open(c_file) as file_obj:
                try:
                    pre_processor.parse(file_obj.read(), c_file)
                except ply.cpp.CppError as exc:
                    raise SystemBuildError(str(exc))

        super().post_prepare(system, config)


module = EntryModule()  # pylint: disable=invalid-name
