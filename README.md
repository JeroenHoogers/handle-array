# Handle Array
Simple header only library with 2 handle based containers. Minimal cpp standard: C++11 
- `HandlePool`
- `HandleArray`

Handle-based containers are commonly used in the context of Data Oriented Design as an alternative to arrays of pointers, providing improved cache locality and memory allocation patterns. 
A `Handle<T>` is essentially a weak reference consisting of an index and generation. It behaves similar to a `T*` pointer and is returned when inserting a value using `add(value)`. However, unlike a `T*` the value it refers to can only be accessed through the container's `get(handle)` method. If the value associated with the handle has been removed in the meantime, `get()` will return `nullptr`.

The implementation is primarily based on Sebastian Aaltonen's "Generational Handles" idea (source: https://advances.realtimerendering.com/s2023/AaltonenHypeHypeAdvances2023.pdf)

## Usage:

### Handle pool
A Handle pool is a non-iterable container optimized for handle-based retrieval.

```cpp
#include <handle_pool>
...

hndl::HandlePool<int> pool;

// add elements
hndl::Handle<int> id0 = pool.add(3);
hndl::Handle<int> id1 = pool.add(5);
hndl::Handle<int> id2 = pool.add(6);

// delete elements by handle
pool.remove(id0);

// get element by handle
int* i0 = pool.get(id0); // (nullptr)
int* i1 = pool.get(id1); // (5)
int* i2 = pool.get(id2); // (6)
```

### Handle array
A Handle array is optimized for fast iteration, although its `add()` and `remove()` operations are slightly slower compared to `HandlePool`.

**Note:** Do not use this container if you only want to retrieve values by handle (using `get()`), use `HandlePool` instead.

```cpp
#include <handle_array>
...

hndl::HandleArray<int> arr;

// add elements
hndl::Handle<int> id0 = arr.add(3);
hndl::Handle<int> id1 = arr.add(5);
hndl::Handle<int> id2 = arr.add(6);

// delete elements by handle
arr.remove(id0);

// get element by handle
int* i0 = arr.get(id0); // (nullptr)
int* i1 = arr.get(id1); // (5)
int* i2 = arr.get(id2); // (6)

// iterate over (packed) data vector
for (int v : arr.data()) {
		printf("%d ", v); // 5 6
}
```
