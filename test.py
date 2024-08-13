
i = 12
positions = []
for x in range(i):
    for y in range(i):
        positions.append(f"float2({x}.0f / 11, {y}.0f / 11)")
indices = []
for y in range(i - 1):
    for x in range(i - 1):
        indices.append(f'uint3({(x) + i * (y)}, {(x + 1) + i * (y)}, {(x) + i * (y + 1)})')
        indices.append(f'uint3({(x) + i * (y + 1)}, {(x + 1) + i * (y)}, {(x + 1) + i * (y + 1)})')
print(f"Dim {i}")
print(len(positions))
print(len(indices))
print(positions)
print(indices)
    

