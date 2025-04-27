import subprocess
import os
import fnmatch
from pathlib import Path

repo_path = Path(__file__).parent.parent
tests_dir_path = repo_path / "tests"
tech_path = tests_dir_path / "tech1.json"
results_dir_path = repo_path / "results"
prog_path = repo_path / "build" / "BufferInserter"


def run_command(cmd: list[str]) -> str:
    result = subprocess.run(
        " ".join(cmd),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=results_dir_path,
        text=True,
        shell=True,
    )
    return result.stdout


def update_results() -> None:
    test_paths: list[Path] = []
    for filename in os.listdir(tests_dir_path):
        if fnmatch.fnmatch(filename, "test*.json"):
            test_paths.append(tests_dir_path / filename)
    results_dir_path.mkdir(exist_ok=True)
    for test_path in test_paths:
        cmd = [
            f"{prog_path}",
            f"{tech_path}",
            f"{test_path}",
        ]
        run_command(cmd)


def main():
    update_results()
    return 0


if __name__ == "__main__":
    main()
