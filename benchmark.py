import subprocess

subprocess.run(['build/Benchmark.exe'], cwd='build')
subprocess.run(['build/Benchmark.exe', 'NOCULL'], cwd='build')