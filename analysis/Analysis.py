import subprocess
import concurrent.futures
import matplotlib.pyplot as plt
import re
import json
from pathlib import Path

average = 5
max_len = 500

repo_path = Path(__file__).parent.parent
tech_path = repo_path / "tests" / "tech1.json"
json_dir_path = repo_path / "build"
runtime_plot_path = repo_path / "res" / "runtime.png"
delay_plot_path = repo_path / "res" / "delay.png"
table_path = repo_path / "res" / "table.txt"
prog_path = repo_path / "build" / "BufferInserter"


class TestResult:
    def __init__(self, len: int, time: int, rat: float):
        self.len = len
        self.time = time
        self.rat = rat


def prepare_data(len: int) -> Path:
    data = {
        "edge": [
            {
                "id": 0,
                "segments": [
                    [0, 0],
                    [len, 0],
                ],
                "vertices": [0, 1],
            }
        ],
        "node": [
            {"id": 0, "name": "buf1x", "type": "b", "x": 0, "y": 0},
            {
                "capacitance": 0.5,
                "id": 1,
                "name": "z0",
                "rat": 200.0,
                "type": "t",
                "x": len,
                "y": 0,
            },
        ],
    }
    file_path = json_dir_path / f"test{len}.json"
    string = json.dumps(data, indent="  ")
    file_path.write_text(string, encoding="UTF-8")
    return file_path


def run_command(cmd: list[str]) -> str:
    result = subprocess.run(
        " ".join(cmd),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=repo_path,
        text=True,
        shell=True,
    )
    return result.stdout


def get_result(len: int) -> TestResult:
    file_path = prepare_data(len)
    cmd = [f"{prog_path}", f"{tech_path}", f"{file_path}"]
    res_time = 0.0
    for _ in range(average):
        output = run_command(cmd)
        match = re.findall(r"Resulting RAT = (.*)\nResulting AlgoTime = (.*)\n", output)
        if not match:
            print(cmd)
            raise RuntimeError(f"Can not get match '{output}'")
        if not match[0]:
            raise RuntimeError(f"fail to get output {cmd}\n")
        rat, time = match[0]
        res_time += int(time)
    time = res_time / average
    return TestResult(len, time, float(rat))


def get_results(n: int = 12) -> list[TestResult]:
    test_results = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=n) as executor:
        futures = {
            executor.submit(get_result, length): length for length in range(1, max_len)
        }
        for future in concurrent.futures.as_completed(futures):
            try:
                result = future.result()
                test_results.append(result)
            except Exception as e:
                print(f"Error occurred for length {futures[future]}: {e}")
    return test_results


def write_results(test_results: list[TestResult]) -> None:
    with open(table_path, "w", encoding="UTF-8") as f:
        for result in test_results:
            f.write(f"{result.len} {result.time} {result.rat}\n")


def read_results() -> list[TestResult]:
    test_results: list[TestResult] = []
    with open(table_path, "r", encoding="UTF-8") as f:
        for line in f:
            len_str, time_str, rat_str = line.split()
            test_results.append(
                TestResult(int(len_str), float(time_str), float(rat_str))
            )
    return test_results


def plot_results(test_results: list[TestResult]) -> None:
    lens = [result.len for result in test_results]
    times = [result.time for result in test_results]
    rats = [result.rat for result in test_results]
    plt.figure(figsize=(5, 5))

    plt.plot(lens, rats, marker="o", color="blue")
    plt.title("RAT(len)")
    plt.xlabel("Length")
    plt.ylabel("RAT")
    plt.grid()
    plt.savefig(delay_plot_path)
    plt.close()

    plt.plot(lens, times, marker="o", color="orange")
    plt.title("Time(len)")
    plt.xlabel("Length")
    plt.ylabel("Time")
    plt.grid()
    plt.savefig(runtime_plot_path)
    plt.close()


def main():
    test_results = get_results()
    write_results(test_results)
    test_results = read_results()
    plot_results(test_results)
    return 0


if __name__ == "__main__":
    main()
