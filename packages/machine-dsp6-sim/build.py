#
# Copyright (c) 2020, Breakaway Consulting Pty. Ltd.
#

import os
from shutil import which
from prj import execute, SystemBuildError
from pathlib import Path


def run(system, _=None):
    return system_build(system)


def system_build(system):
    output_dir = Path(system.output)
    chessde_prx = output_dir / "chessde.prx"

    if which("chessmk") is None:
        raise SystemBuildError("Unable to find chessmk in PATH.")

    file_els = []
    for c_file in system.c_files:
        reldir, filename = os.path.split(os.path.relpath(c_file, output_dir))
        file_els.append('    <file type="c" name="{}" path="{}" />'.format(filename, reldir))

    # FIXME: Add support for custom include paths.
    # inc_path_args = ['-I%s' % i for i in system.include_paths]

    file_str = "\n".join(file_els)
    with chessde_prx.open("w") as f:
        f.write("""
<project name="Project" processor="dsp6_i32">
    <option id="project.name" value="hello"/>
    <option id="project.type" value="exe"/>
    <option id="cpp.wundef" value="on"/>
    {}
</project>
""".format(file_str))

    execute(["chessmk", str(chessde_prx)])
