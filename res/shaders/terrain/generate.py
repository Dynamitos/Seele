import subprocess

e = ['Reset', 'Classify', 'Split', 'PrepareIndirect', 'Allocate', 'Bisect', 'PropagateBisect', 'PrepareSimplify', 'Simplify', 'PropagateSimplify', 'ReducePrePass', 'ReduceFirstPass', 'ReduceSecondPass', 'BisectorIndexation', 'PrepareBisectorIndirect', 'Validate']

def generate(entry: list[str], file: str):
    for point in entry:
        subprocess.run(['C:/Users/Dynamitos/slang/build/Release/bin/slangc', '-I', '../lib', f'{file}.slang', '-profile', 'spirv_1_6', '-stage', 'compute', '-target','spirv', '-entry', f'{point}', '-o', f'{point}.spv', '-emit-spirv-via-glsl'])

generate(e, 'CBTCompute')

e = ['ClearBuffer', 'EvaluateLEB']

generate(e, 'LEB')