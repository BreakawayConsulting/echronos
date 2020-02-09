#
# Copyright (c) 2020, Breakaway Consulting Pty. Ltd.
#

from shutil import which
from prj import execute, SystemBuildError
from pathlib import Path


def run(system, _=None):
    return system_run(system)


def system_run(system):
    output_dir = Path(system.output)
    chessde_prx = output_dir / "chessde.prx"

    if which("chessmk") is None:
        raise SystemBuildError("Unable to find chessmk in PATH.")

    execute(["chessmk", "-S", "-s", str(chessde_prx)])
