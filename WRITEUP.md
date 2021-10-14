# Seele Engine

Seele is a Proof-of-Concept 3D render engine using Vulkan as it's primary graphics API, but other APIs could also be implemented.
The focus of the project is to make the renderer use as many of the system resources as efficiently as possible to maximize the framerate 
while minimizing resource usage.
It does this by being designed to support multithreading for more recent APIs, like DX12 or Vulkan, while still providing an interface that could be implemented by an
API that doesn't support it, like OpenGL. These APIs would suffer a significant performance loss, but would still work correctly.

## Custom containers and other utilities

Some of the basic containers are reimplemented, mostly not for any particular feature, but because that makes them more customizable and extendable for future uses and
performance optimizations, like serialization for example. Due to them occuring in the code samples later, they will be quickly mentioned here.

- `Array`: A heap-array backed dynamically resizable container, like `std::vector`
- `StaticArray`: A static-array backed non-resizable container, like `std::array`
- `Map`: A key-value splay-tree associative container, like `std::map`
- `UniquePtr`: A smart pointer claiming ownership of an object, like `std::unique_pointer`
- `RefPtr`: A reference counting smart pointer, similar to `std::shared_pointer`, with a few differences:
  - global pointer-based reference counting
  - this means a `RefPtr` can be constructed from a raw pointer and will retain its reference count correctly
  - for example if you construct `RefPtr<HelloWorld> ptr = new HelloWorld();`, and in the constructor of HelloWorld create `RefPtr<HelloWorld ptr = this;`, both would have a reference count of 2.
- `DEFINE_REF`/`DECLARE_REF`: Macros to automatically typedef different types of smart pointer for convenience, for example DEFINE_REF(HelloWorld) would typedef:
  - `PHelloWorld`, the same as `RefPtr<HelloWorld>`
  - `UPHelloWorld`, the same as `UniquePtr<HelloWorld>`

