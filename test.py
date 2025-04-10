
i = 4
positions = []
for x in range(i):
    for y in range(i):
        positions.append(f"Vector({x}.0f / {i-1}, 0, {y}.0f / {i-1})")
indices = []
for y in range(i - 1):
    for x in range(i - 1):
        indices.append(f'UVector3({(x) + i * (y)}, {(x + 1) + i * (y)}, {(x) + i * (y + 1)})')
        indices.append(f'UVector3({(x) + i * (y + 1)}, {(x + 1) + i * (y)}, {(x + 1) + i * (y + 1)})')
print(f"Dim {i}")
print(len(positions))
print(len(indices))
print(positions)
print(indices)
    

# Create a tensor with attached grads from a numpy array
# Note: We pass zero=True to initialize the grads to zero on allocation
x = spy.Tensor.numpy(device, np.array([1, 2, 3, 4], dtype=np.float32)).with_grads(zero=True)

# Evaluate the polynomial and ask for a tensor back
# Expecting result = 2x^2 + 8x - 1
result: spy.Tensor = module.polynomial(a=2, b=8, c=-1, x=x, _result='tensor')
print(result.to_numpy())