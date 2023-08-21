# libRad

libRad is a "supplement" for the C++ standard library; that is, a library which adds many helpful
utilities, and improves upon several of the standard utilities.

It does NOT aim to be a replacement for the C++ standard library.

## Features

### Memory allocation/management

libRad adds it own memory allocation functions in `rad_memory.h`, such as
`RAD_ALLOC`, `RAD_REALLOC`, `RAD_FREE`, `RAD_ALLOC_ALIGNED`, `RAD_REALLOC_ALIGNED`,
and `RAD_FREE_ALIGNED`, which allocate/reallocate/free memory with/without a
specified alignment, in a cross-platform way, automatically assigning debugging
information (the source file path and line number of the call) to each allocation
(unless `RAD_USE_DEBUG_MEMORY` is defined to 0 - by default it is defined to 0
if NDEBUG is defined, or 1 otherwise).

If `RAD_USE_OPERATOR_NEW_DELETE_REPLACEMENTS` is defined to 1, it will also automatically
replace all C++ global operator new/new[]/delete/delete[] with variants that call
the libRad memory allocation functions.

If you use `RAD_NEW` instead of new/new[], it will also automatically assign debugging
information to each allocation. There is no `RAD_DELETE`; just use normal delete/delete[]
to delete allocations created with `RAD_NEW`.

It also provides `rad::is_aligned`, which allows you to conviniently check if a pointer
or address is aligned to a given alignment.

#### Custom allocators

All libRad containers which allocate memory also support (or will soon support) custom
memory allocators, just like the C++ standard library types (e.g. std::vector). However,
libRad containers also optionally support calling an additional function on your custom
allocator if present: `reallocate`. This way, you can create custom allocators that, when
used with libRad containers, can reallocate memory more efficiently than the C++ standard
containers, while still being fully-compatible with the actual C++ standard containers.

## Object management

libRad adds several (very) helpful object management utilities in `rad_object_utils.h`, such as:

- `rad::destruct`: Destructs a given object, or a given range of objects.
- `rad::uninitialized_direct_construct`: Constructs objects into a given range of uninitialized memory using placement new and the given variadic arguments.
- `rad::uninitialized_move_strong`: Like `std::uninitialized_move`, except with a strong exception guarantee!
- `rad::move_strong`: Like `std::move` (the one from `<algorithm>`, not the one from `<utility>`), except with a strong exception guarantee!

## Vectors

libRad adds `rad::vector`, which is a custom drop-in replacement for `std::vector`
that is more efficient and adds extra functionality.

Unlike `std::vector`, `rad::vector` internally calls `reallocate` on its allocator,
which allows it to grow more efficiently than `std::vector`.

It also adds new functionality, such as the `release` function, which releases
ownership of the vector's data buffer to the caller (similar to `std::unique_ptr`'s
`release` function). It's up to the user to destruct the elements and free the
memory using the allocator's deallocate function (or equivalent).

## Stack or heap memory

libRad adds `rad::stack_or_heap_memory`, which is a template container that consists
of a data buffer of a given size/alignment, and a pointer. You simply construct the object
with a given size (or call `.reallocate` on it). If that size fits within the container's
data buffer, the container will set its pointer to just point to that data buffer, and no
memory allocation will be done. Otherwise, the container will heap-allocate memory, and
set its pointer to that heap allocation. When the container is destructed, it will automatically
free the memory if it was heap-allocated, or do nothing if it was not heap-allocated.

Example usage:

```cpp
// This container will have a fixed-size stack buffer of chars with sizeof(32) and alignof(int).

// If requiredSize fits within that stack buffer, the stack buffer
// will be used directly, and no heap allocation will be done!

// Otherwise, the container will instead heap-allocate a char array
// of requiredSize with alignof(int).

const auto requiredSize = (sizeof(int) * dynamicNumberOfInts);
rad::stack_or_heap_memory<32, alignof(int)> memory(requiredSize);

// memory.data just returns the pointer to the memory; that is,
// either the stack buffer, or the heap-allocated memory.
std::fill(memory.data<int>(), memory.data<int>() + dynamicNumberOfInts, 3);

// There's plenty of other helpful things you can do with it:
// - Use memory.is_heap to check if the memory was heap-allocated or not.
// - Use memory.reallocate to efficiently reallocate the memory to a new size.
// - Use memory.deallocate to free any heap-allocations and switch to using the stack buffer.
// - Use memory.release_heap to release ownership of any heap allocations
//   (it will do nothing and return nullptr if the stack buffer is being used).
```

It is extremely efficient both in terms of memory usage, and runtime cost.
It does one single check in its constructor to see if it can use the stack buffer.
If it can, it will. This way, performance is extremely similar to if you had
just used a fixed-sized array (that is, it's extremely fast), but with the added
safety-net of being able to keep working even if the requested size exceeds
the fixed-sized array.

## Stack or heap array

libRad also adds `rad::stack_or_heap_array`, which is a convenient container which
wraps around `rad::stack_or_heap_memory`, but which also stores a count of elements,
and thus can be used like an `std::array` (with some extra stuff).

Example usage:

```cpp
// This container will have a fixed-size stack buffer of 8 ints.

// If dynamicNumberOfInts is <= 8, the stack buffer will be used directly,
// and no heap allocation will be done!

// Otherwise, the container will instead heap-allocate an int array
// of dynamicNumberOfInts.

// Its constructor takes a count, followed by an optional variadic number
// of arguments, which will be used to construct the elements in the array.

// In this case, we're passing the number 3 in as the only argument, so
// all of the ints in the array will be set to 3 upon construction.
rad::stack_or_heap_array<int, 8> arrayOfInts(dynamicNumberOfInts, 3);

// Unlike rad::stack_or_heap_memory, rad::stack_or_heap_array also has .begin() and .end()
std::fill(arrayOfInts.begin(), arrayOfInts.end(), 2);

// You can safely use it for any type; it will automatically handle construction/destruction!
rad::stack_or_heap_array<std::string, 16> arrayOfStrings(dynamicNumberOfStrings);

for (std::size_t i = 0; i < arrayOfStrings.size(); ++i)
{
    arrayOfStrings[i] = "string #";
    arrayOfStrings[i] += std::to_string(i);
}
```

## Defer

libRad adds defer functionality, similar to that found in Go, in `rad_defer.h`.

Example usage:

```cpp
int main(int argc, char* argv[])
{
    int* arrayOfInts = RAD_NEW(int)[4]; // new int[4], but with debugging info

    RAD_DEFER
    {
        // This is the body of a lambda function; go nuts!
        // This lambda will not be run until the end of the current scope is reached,
        // at which point, it will be run automatically.
        delete[] arrayOfInts;
        std::cout << "print #3\n";
    }

    RAD_DEFER
    {
        // You can have as many defer statements as you want; they will not collide with each other.
        // The defer statements are guaranteed to be run in the reverse order they are listed in,
        // similar to how objects in C++ are guaranteed to be destructed in the reverse order they
        // were created in.
        std::cout << "print #2\n";
    }

    std::cout << "print #1\n";
    
    // Even if an exception is thrown, the defer statements will be called!
    throw std::runtime_error("uh oh!");

    return EXIT_SUCCESS;
}
```

The above code will print the following:

```
print #1
print #2
print #3
```

This functionality can be very useful in certain cases, such as to easily handle cleaning up
objects created via a C-style API in an exception-safe way, without having to create
a custom RAII wrapper or std::unique_ptr deleter.

It's implemented via a lambda function that is passed into an instance of a simple class
which, on destruction, just calls the lambda function. So, it can be optimized very well!
No heap memory allocation or anything is happening here.

## Pairs

libRad adds `rad::pair`, which is a custom drop-in replacement for `std::pair`
which aims to be more memory efficient.

The main difference is that `rad::pair` is a "compressed pair"; that is,
if one of its members is empty, it will occupy no space within the pair,
unlike `std::pair`, in which the empty members will still occupy space.

The libRad containers use this heavily, mainly to store allocators in a
way that occupies no space unless the allocator class is not empty.

## Scoped enum helpers

libRad adds utilities to assist with the usage of "scoped enums" (enum classes) in `rad_scoped_enum_helpers.h`,
such as the `RAD_ENABLE_SCOPED_ENUM_BITWISE_OPS` macro, which defines bitwise operators for a given
scoped enum type.

Example usage:

```cpp
enum class animal_properties
{
    has_wings = 1,
    can_fly = 2,
    can_swim = 4,
};

// This one line will enable you to use all of the following operators on animal_properties:
// - operator&
// - operator|
// - operator^
// - operator~
// - operator<<
// - operator>>
// - operator&=
// - operator|=
// - operator^=
// - operator<<=
// - operator>>=
RAD_ENABLE_SCOPED_ENUM_BITWISE_OPS(animal_properties)

enum class animal_type
{
    unknown,
    bird,
    duck,
    penguin,
};

animal_type guess_animal_type(animal_properties props)
{
    // Note how you can use the bitwise AND operator (&)
    // and that the result can be cast to bool.
    if (props & animal_properties::has_wings)
    {
        if (props & animal_properties::can_fly)
        {
            if (props & animal_properties::can_swim)
            {
                return animal_type::duck;
            }
            else
            {
                return animal_type::bird;
            }
        }
        else if (props & animal_properties::can_swim)
        {
            return animal_type::penguin;
        }
    }

    return animal_type::unknown;
}
```
