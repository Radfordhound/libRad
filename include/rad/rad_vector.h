/// @file rad_vector.h
/// @author Graham Scott
/// @brief Header file providing rad::vector.
/// @date 2023-06-14
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_VECTOR_H_INCLUDED
#define RAD_VECTOR_H_INCLUDED

#include "rad_base.h"
#include "rad_default_allocator.h"
#include "rad_memory.h"
#include <cstddef>
#include <cassert>
#include <type_traits>
#include <iterator>
#include <initializer_list>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <cstring>

namespace rad
{
/// @brief A resizable list of elements, like `std::vector`, but
/// with several additions/improvements.
///
/// @details Some of the changes include:
///
/// - Uses a `rad::allocator` for memory allocation instead of the STL allocator approach.
///
/// - Does reallocation through the `rad::allocator`, allowing for realloc optimizations
///   where applicable (e.g. growing/shrinking the memory block without a new allocation).
///
/// - Has a `release` function, which allows you to take ownership of the vector's buffer.
///
/// - Has a `no_value_init` variant of the default constructor, which allows for
///   default-construction of elements (instead of value-construction).
///
/// - Uses strict bounds checking in all indexing operations if
///   `RAD_USE_STRICT_BOUNDS_CHECKING` is defined to 1.
///
/// - Has a `no_value_init` variant of the push_back function, which allows for a
///   default-constructed element to be pushed back (instead of a value-constructed element).
///
/// - Has a `back_index` function, which returns the index of the last element in the vector.
///
/// - Has a `clear_and_free` function, which both empties the vector and frees its underlying memory.
///
/// @tparam T The value type of the vector.
template<typename T>
class vector
{
    using size_type_ = std::size_t;

    rad::allocator* allocator_ = &default_allocator;
    T*              dataBegin_ = nullptr;
    T*              dataEnd_ = nullptr;
    T*              bufEnd_ = nullptr;

    void destroy_data_() noexcept
    {
        // NOTE: This is safe even if dataBegin and dataEnd are both nullptr.
        rad::destruct(dataBegin_, dataEnd_);
        allocator_->free(dataBegin_);
    }

    void reset_() noexcept
    {
        dataBegin_ = dataEnd_ = bufEnd_ = nullptr;
    }

    void validate_range_(size_type_ index) const
    {
        if (index >= size())
        {
            throw std::out_of_range(
                "The given index was outside of the vector's range"
            );
        }
    }

    void assert_iterator_validity_(const T* it) const noexcept
    {
        assert(it >= dataBegin_ && it <= dataEnd_ &&
            "The given iterator was invalid; this is undefined behavior!"
        );
    }

    size_type_ compute_new_capacity_(size_type_ newDataCount) const noexcept
    {
        // If geometric growth would exceed max_size() and potentially
        // overflow, we just return max_size() instead.
        const auto bufCount = capacity();
        const auto maxCount = max_size();

        if (bufCount > (maxCount - (bufCount / 2)))
        {
            return maxCount;
        }

        // Attempt to geometrically grow from current capacity,
        // falling back to newDataCount if the computed value is
        // not sufficient.
        return std::max<size_type_>(
            bufCount + (bufCount / 2),
            newDataCount
        );
    }

    void reallocate_(
        size_type_ oldCount,
        size_type_ newCapacity)
    {
        dataBegin_ = allocator_->resize(
            no_default_init,
            dataBegin_,
            oldCount,
            newCapacity
        );

        dataEnd_ = dataBegin_ + oldCount;
        bufEnd_ = dataBegin_ + newCapacity;
    }

    void reserve_for_one_()
    {
        if (dataEnd_ == bufEnd_)
        {
            const auto oldCount = size();
            const auto newCapacity = compute_new_capacity_(oldCount + 1);

            reallocate_(oldCount, newCapacity);
        }
    }

    /*
    template<typename InputIt>
    T* insert_unchecked_(
        const T* pos,
        InputIt begin,
        InputIt end,
        size_type_ insertCount)
    {
        // TODO: Rewrite and optimize this
        const auto it = const_cast<T*>(pos);

        // Insert elements.
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            std::memmove(it + insertCount, it, dataEnd_ - it);
            dataEnd_ += insertCount;

            std::uninitialized_copy(begin, end, it);
        }
        else
        {
            //// 12345678
            //// 12ABC345678
            ////   ^ ^
            //std::move_backward(it, dataEnd_, it + insertCount);
            // TODO: Test this!!!
            iterator dst = it;

            while (begin != end)
            {
                dst = emplace_unchecked(dst, *begin);
                ++dst;
                ++begin;
            }
        }

        return it;
    }
    */

public:
    using value_type            = T;
    using allocator_type        = rad::allocator;
    using size_type             = size_type_;
    using difference_type       = std::ptrdiff_t;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using pointer               = value_type*;
    using const_pointer         = const value_type*;
    using iterator              = pointer;
    using const_iterator        = const_pointer;
    // TODO: reverse_iterator
    // TODO: const_reverse_iterator

    static constexpr bool has_trivially_destructible_elements =
        std::is_trivially_destructible_v<T>;

    inline allocator_type& allocator() const noexcept
    {
        return *allocator_;
    }

    inline const T* data() const noexcept
    {
        return dataBegin_;
    }

    inline T* data() noexcept
    {
        return dataBegin_;
    }

    inline size_type size() const noexcept
    {
        return dataEnd_ - dataBegin_;
    }

    inline size_type capacity() const noexcept
    {
        return bufEnd_ - dataBegin_;
    }

    static constexpr size_type max_size() noexcept
    {
        return std::min<size_type>(
            (std::numeric_limits<difference_type>::max)(),
            (std::numeric_limits<size_type>::max)()
        );
    }

    [[nodiscard]] inline bool empty() const noexcept
    {
        return dataBegin_ == dataEnd_;
    }

    inline const_iterator cbegin() const noexcept
    {
        return dataBegin_;
    }

    inline const_iterator begin() const noexcept
    {
        return dataBegin_;
    }

    inline iterator begin() noexcept
    {
        return dataBegin_;
    }

    inline const_iterator cend() const noexcept
    {
        return dataEnd_;
    }

    inline const_iterator end() const noexcept
    {
        return dataEnd_;
    }

    inline iterator end() noexcept
    {
        return dataEnd_;
    }

    inline const_reference front() const noexcept
    {
        assert(!empty() &&
            "Cannot call front() on an empty vector!"
        );

        return *dataBegin_;
    }

    inline reference front() noexcept
    {
        assert(!empty() &&
            "Cannot call front() on an empty vector!"
        );

        return *dataBegin_;
    }

    inline const_reference back() const noexcept
    {
        assert(!empty() &&
            "Cannot call back() on an empty vector!"
        );

        return *(dataEnd_ - 1);
    }

    inline reference back() noexcept
    {
        assert(!empty() &&
            "Cannot call back() on an empty vector!"
        );

        return *(dataEnd_ - 1);
    }

    inline size_type back_index() const noexcept
    {
        assert(!empty() &&
            "Cannot call back_index() on an empty vector!"
        );

        return size() - 1;
    }

    void reserve(size_type newCapacity)
    {
        const auto oldCapacity = capacity();

        if (newCapacity > oldCapacity)
        {
            reallocate_(size(), newCapacity);
        }
    }

    void pop_back() noexcept
    {
        assert(dataEnd_ != dataBegin_ &&
            "Cannot call pop_back() on an empty vector!"
        );

        rad::destruct(--dataEnd_);
    }

    void pop_back(size_type amount) noexcept
    {
        assert(size() >= amount &&
            "Cannot pop_back() more elements than exist in the vector!"
        );

        const auto newEnd = (dataEnd_ - amount);
        rad::destruct(newEnd, dataEnd_);
        dataEnd_ = newEnd;
    }

    bool resize(no_value_init_t, size_type newCount)
    {
        const auto oldCount = size();

        if (newCount > oldCount)
        {
            reallocate_(oldCount, newCount);

            const auto newEnd = dataBegin_ + newCount;
            std::uninitialized_default_construct(dataEnd_, newEnd);
            dataEnd_ = newEnd;

            return true;
        }
        else
        {
            pop_back(oldCount - newCount);
            return false;
        }
    }

    bool resize(size_type newCount)
    {
        const auto oldCount = size();

        if (newCount > oldCount)
        {
            reallocate_(oldCount, newCount);

            const auto newEnd = dataBegin_ + newCount;
            std::uninitialized_value_construct(dataEnd_, newEnd);
            dataEnd_ = newEnd;

            return true;
        }
        else
        {
            pop_back(oldCount - newCount);
            return false;
        }
    }

    template<typename... Args>
    bool resize(size_type newCount, const Args&... args)
    {
        const auto oldCount = size();

        if (newCount > oldCount)
        {
            reallocate_(oldCount, newCount);

            const auto newEnd = dataBegin_ + newCount;
            uninitialized_direct_construct(dataEnd_, newEnd, args...);
            dataEnd_ = newEnd;

            return true;
        }
        else
        {
            pop_back(oldCount - newCount);
            return false;
        }
    }

    template<typename... Args>
    inline reference emplace_back_unchecked(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        // Construct new element.
        ::new (dataEnd_) T(std::forward<Args>(args)...);
        return *(dataEnd_++);
    }

    template<typename... Args>
    reference emplace_back(Args&&... args)
    {
        // Reallocate if necessary.
        reserve_for_one_();

        // Append new element.
        return emplace_back_unchecked(std::forward<Args>(args)...);
    }

    inline reference push_back_unchecked(no_value_init_t)
        noexcept(std::is_nothrow_default_constructible_v<T>)
    {
        // Default-construct new element if necessary.
        if constexpr (!std::is_trivially_default_constructible_v<T>)
        {
            ::new (dataEnd_) T;
        }

        return *(dataEnd_++);
    }

    inline reference push_back_unchecked(const T& val)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        return emplace_back_unchecked(val);
    }

    inline reference push_back_unchecked(T&& val)
        noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        return emplace_back_unchecked(std::move(val));
    }

    reference push_back(no_value_init_t)
    {
        // Reallocate if necessary.
        reserve_for_one_();

        // Append new element.
        return push_back_unchecked(no_value_init);
    }

    inline reference push_back(const T& val)
    {
        return emplace_back(val);
    }

    inline reference push_back(T&& val)
    {
        return emplace_back(std::move(val));
    }

    template<typename InputIt>
    void assign(InputIt begin, InputIt end)
    {
        const auto oldBegin = dataBegin_;
        const auto oldEnd = dataEnd_;

        reset_();

        // Destruct the old elements.
        rad::destruct(oldBegin, oldEnd);

        // Reallocate memory.
        const std::size_t newCount = std::distance(begin, end);

        const auto newPtr = static_cast<T*>(reallocate(
            oldBegin,
            sizeof(T) * newCount,
            alignof(T)
        ));

        // Copy new elements to reallocated memory.
        try
        {
            std::uninitialized_copy(begin, end, newPtr);
        }
        catch (...)
        {
            free(newPtr);
            throw;
        }

        dataBegin_ = newPtr;
        bufEnd_ = dataEnd_ = newPtr + newCount;
    }

    iterator emplace_unchecked(no_value_init_t, const_iterator pos)
    {
        // Emplace the element.
        const auto it = const_cast<iterator>(pos);
        const auto oldEnd = dataEnd_;

        if (it != oldEnd)
        {
            // Move-construct last element at the new uninitialized end.
            ::new (oldEnd) T(std::move(*(oldEnd - 1)));
            ++dataEnd_;

            // Shift remaining elements as necessary.
            std::move_backward(it, oldEnd - 1, oldEnd);

            // NOTE: The new element is an old element which is already
            // constructed, so we don't have to do anything else here.
        }
        else
        {
            push_back_unchecked(no_value_init);
        }

        return it;
    }

    iterator emplace(no_value_init_t, const_iterator pos)
    {
        assert_iterator_validity_(pos);

        // Reallocate if necessary.
        const auto index = (pos - dataBegin_);
        reserve_for_one_();

        // Compute new iterator.

        // NOTE: This is necessary because the above
        // reallocation might invalidate the iterator!

        pos = (dataBegin_ + index);

        // Emplace new element.
        return emplace_unchecked(no_value_init, pos);
    }

    template<typename... Args>
    iterator emplace_unchecked(const_iterator pos, Args&&... args)
    {
        // Emplace the element.
        const auto it = const_cast<iterator>(pos);
        const auto oldEnd = dataEnd_;

        if (it != oldEnd)
        {
            // Move-construct last element at the new uninitialized end.
            ::new (oldEnd) T(std::move(*(oldEnd - 1)));
            ++dataEnd_;

            // Shift remaining elements as necessary.
            std::move_backward(it, oldEnd - 1, oldEnd);

            // Move-assign the new element.
            *it = T(std::forward<Args>(args)...);
        }
        else
        {
            emplace_back_unchecked(std::forward<Args>(args)...);
        }

        return it;
    }

    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        assert_iterator_validity_(pos);

        // Reallocate if necessary.
        const auto index = (pos - dataBegin_);
        reserve_for_one_();

        // Compute new iterator.

        // NOTE: This is necessary because the above
        // reallocation might invalidate the iterator!

        pos = (dataBegin_ + index);

        // Emplace new element.
        return emplace_unchecked(pos, std::forward<Args>(args)...);
    }

    inline iterator insert_unchecked(const_iterator pos, const T& val)
    {
        return emplace_unchecked(pos, val);
    }

    inline iterator insert_unchecked(const_iterator pos, T&& val)
    {
        return emplace_unchecked(pos, std::move(val));
    }

    iterator insert_unchecked(no_value_init_t, const_iterator pos, size_type count)
    {
        // Insert elements.
        const auto it = const_cast<iterator>(pos);

        if constexpr (std::is_trivially_copyable_v<T>)
        {
            // TODO: Test this!!!
            const auto insertRangeEnd = (it + count);
            std::memmove(insertRangeEnd, it, dataEnd_ - it);

            std::uninitialized_default_construct(it, insertRangeEnd);
        }
        else
        {
            // TODO: Replace this with an optimized algorithm!
            // TODO: Test this!!!
            while (count)
            {
                emplace_unchecked(no_value_init, it);
                --count;
            }
        }

        return it;
    }

    inline iterator insert(const_iterator pos, const T& val)
    {
        return emplace(pos, val);
    }

    inline iterator insert(const_iterator pos, T&& val)
    {
        return emplace(pos, std::move(val));
    }

    iterator insert(no_value_init_t, const_iterator pos, size_type count)
    {
        assert_iterator_validity_(pos);

        // Reallocate if necessary.
        const auto index = (pos - dataBegin_);
        reserve(size() + count);

        // Compute new iterator.

        // NOTE: This is necessary because the above
        // reallocation might invalidate the iterator!

        pos = (dataBegin_ + index);

        // Insert new elements.
        return insert_unchecked(no_value_init, pos, count);
    }

    iterator insert(const_iterator pos, size_type count, const T& val)
    {
        // TODO: Create an unchecked variant of this function.

        // Reallocate if necessary.
        const auto index = (pos - dataBegin_);
        reserve(size() + count);

        // Compute new insertion iterator.

        // NOTE: This is necessary because the above reallocation
        // might invalidate the iterator! Also, we need to cast
        // from a const_iterator to an iterator anyway.

        iterator it = (dataBegin_ + index);

        // Insert elements.
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            // TODO: Test this!!!
            std::memmove(it + count, it, dataEnd_ - it);

            while (count)
            {
                *it = val;
                ++it;
                --count;
            }

            dataEnd_ = it;
        }
        else
        {
            // TODO: Test this!!!
            while (count)
            {
                it = emplace(it, val);
                --count;
            }
        }

        return it;
    }

    template<typename InputIt>
    iterator insert(const_iterator pos, InputIt begin, InputIt end)
    {
        // TODO: Create an unchecked variant of this function.
        // TODO: Rewrite and optimize this function.

        // Compute insert count and reallocate if necessary.
        const auto insertCount = std::distance(begin, end);
        const auto index = (pos - dataBegin_);

        reserve(size() + insertCount);

        // Compute new insertion iterator.

        // NOTE: This is necessary because the above reallocation
        // might invalidate the iterator! Also, we need to cast
        // from a const_iterator to an iterator anyway.

        const iterator it = (dataBegin_ + index);

        // Insert elements.
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            std::memmove(it + insertCount, it, dataEnd_ - it);
            dataEnd_ += insertCount;

            std::uninitialized_copy(begin, end, it);
        }
        else
        {
            // TODO: Test this!!!
            iterator dst = it;

            while (begin != end)
            {
                dst = emplace_unchecked(dst, *begin);
                ++dst;
                ++begin;
            }
        }

        return it;
    }

    inline iterator insert(const_iterator pos, std::initializer_list<T> ilist)
    {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template<typename InputIt>
    inline iterator append(InputIt begin, InputIt end)
    {
        return insert(this->end(), begin, end);
    }

    inline iterator append(no_value_init_t, size_type count)
    {
        return insert(no_value_init, end(), count);
    }

    iterator erase(const_iterator pos)
    {
        const auto it = move_strong(
            const_cast<iterator>(pos + 1),
            dataEnd_,
            const_cast<iterator>(pos)
        );

        rad::destruct(*it);
        dataEnd_ = it;
        
        return const_cast<iterator>(pos);
    }

    // TODO: Range erase

    void clear() noexcept
    {
        rad::destruct(dataBegin_, dataEnd_);
        dataEnd_ = dataBegin_;
    }

    void clear_and_free() noexcept
    {
        destroy_data_();
        reset_();
    }

    /// @brief Releases ownership of the data buffer
    /// to the caller and clears the vector.
    /// 
    /// @details After calling this function, it is the caller's
    /// responsibility to destruct the elements within the data buffer
    /// (unless `has_trivially_destructible_elements` is true), and to
    /// deallocate the memory using the allocator's free function
    /// (or equivalent).
    ///
    /// All of this can be done using the allocator's `destroy`
    /// helper function.
    /// 
    /// @return pointer A pointer to the data buffer, no longer owned by the vector.
    [[nodiscard]] pointer release() noexcept
    {
        const auto dataBuf = dataBegin_;
        reset_();
        return dataBuf;
    }

    const_reference at(size_type pos) const
    {
        validate_range_(pos);
        return dataBegin_[pos];
    }

    reference at(size_type pos)
    {
        validate_range_(pos);
        return dataBegin_[pos];
    }

    inline const_reference operator[](size_type pos) const
        noexcept(!RAD_USE_STRICT_BOUNDS_CHECKING)
    {
    #if RAD_USE_STRICT_BOUNDS_CHECKING
        validate_range_(pos);
    #endif

        return dataBegin_[pos];
    }

    inline reference operator[](size_type pos)
        noexcept(!RAD_USE_STRICT_BOUNDS_CHECKING)
    {
    #if RAD_USE_STRICT_BOUNDS_CHECKING
        validate_range_(pos);
    #endif

        return dataBegin_[pos];
    }

    // TODO: Handle propagate_on_container_copy_assignment properly!!!
    vector& operator=(const vector& other)
    {
        if (&other != this)
        {
            const auto otherCount = other.size();
            const auto newPtr = other.allocator_->create_copy<T>(
                other.dataBegin_,
                otherCount
            );

            destroy_data_();

            allocator_ = other.allocator_;
            dataBegin_ = newPtr;
            bufEnd_ = dataEnd_ = newPtr + otherCount;
        }
        
        return *this;
    }

    // TODO: Handle propagate_on_container_move_assignment properly!!!
    vector& operator=(vector&& other) noexcept
    {
        if (&other != this)
        {
            destroy_data_();

            allocator_ = other.allocator_;
            dataBegin_ = other.dataBegin_;
            dataEnd_ = other.dataEnd_;
            bufEnd_ = other.bufEnd_;

            other.reset_();
        }

        return *this;
    }

    constexpr vector() noexcept = default;

    constexpr explicit vector(rad::allocator& allocator) noexcept
        : allocator_(&allocator)
    {
    }

    explicit vector(no_value_init_t, size_type count)
        : dataBegin_(allocator_->create<T>(no_value_init, count))
        , dataEnd_(dataBegin_ + count)
        , bufEnd_(dataEnd_)
    {
    }

    explicit vector(no_value_init_t, rad::allocator& allocator, size_type count)
        : allocator_(&allocator)
        , dataBegin_(allocator_->create<T>(no_value_init, count))
        , dataEnd_(dataBegin_ + count)
        , bufEnd_(dataEnd_)
    {
    }

    template<typename... Args>
    explicit vector(size_type count, const Args&... args)
        : dataBegin_(allocator_->create<T>(count, args...))
        , dataEnd_(dataBegin_ + count)
        , bufEnd_(dataEnd_)
    {
    }

    template<typename... Args>
    explicit vector(rad::allocator& allocator, size_type count, const Args&... args)
        : allocator_(&allocator)
        , dataBegin_(allocator_->create<T>(count, args...))
        , dataEnd_(dataBegin_ + count)
        , bufEnd_(dataEnd_)
    {
    }

    template<typename InputIt>
    explicit vector(rad::allocator& allocator, InputIt begin, InputIt end)
        : allocator_(&allocator)
    {
        const std::size_t count = std::distance(begin, end);

        dataBegin_ = static_cast<T*>(allocator_->allocate(
            sizeof(T) * count,
            alignof(T)
        ));

        bufEnd_ = dataEnd_ = dataBegin_ + count;

        try
        {
            std::uninitialized_copy(begin, end, dataBegin_);
        }
        catch (...)
        {
            free(dataBegin_);
            throw;
        }
    }

    template<typename InputIt>
    explicit vector(InputIt begin, InputIt end)
        : vector(default_allocator, begin, end)
    {
    }

    vector(const vector& other)
        : allocator_(other.allocator_)
        , dataBegin_(allocator_->create_copy<T>(other.dataBegin_, other.size()))
        , dataEnd_(dataBegin_ + other.size())
        , bufEnd_(dataEnd_)
    {
    }

    constexpr vector(vector&& other) noexcept
        : allocator_(other.allocator_)
        , dataBegin_(other.dataBegin_)
        , dataEnd_(other.dataEnd_)
        , bufEnd_(other.bufEnd_)
    {
        other.reset_();
    }

    inline ~vector()
    {
        destroy_data_();
    }
};
}

#endif
