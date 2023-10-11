/**
 * @file rad_vector.h
 * @author Graham Scott
 * @brief Header file providing rad::vector.
 * @version 0.1
 * @date 2023-06-14
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 * 
 */
#ifndef RAD_VECTOR_H_INCLUDED
#define RAD_VECTOR_H_INCLUDED

#include "rad_pair.h"
#include "rad_default_allocator.h"
#include "rad_allocator_traits.h"
#include <type_traits>
#include <limits>
#include <algorithm>
#include <cstddef>

namespace rad
{
template<typename T, class Allocator = default_allocator<T>>
class vector
{
    using allocator_traits_ = allocator_traits<Allocator>;
    using size_type_        = typename allocator_traits_::size_type;

    struct values_t_
    {
        T*  dataBegin;
        T*  dataEnd;
        T*  bufEnd;

        constexpr void reset() noexcept
        {
            dataBegin = nullptr;
            dataEnd = nullptr;
            bufEnd = nullptr;
        }
    };

    pair<Allocator, values_t_>    data_;

    constexpr const Allocator& allocator_() const noexcept
    {
        return data_.first();
    }

    constexpr Allocator& allocator_() noexcept
    {
        return data_.first();
    }

    constexpr const values_t_& values_() const noexcept
    {
        return data_.second();
    }

    constexpr values_t_& values_() noexcept
    {
        return data_.second();
    }

    static constexpr bool must_call_allocator_destroy_on_elements_() noexcept
    {
        return (
            !std::is_trivially_destructible_v<T> ||
            allocator_traits_::has_destroy
        );
    }

    size_type_ compute_new_buf_count_(size_type_ newDataCount) const noexcept
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
            newDataCount);
    }

    template<typename... Args>
    void init_with_direct_construct_(size_type_ count, const Args&... args)
    {
        // Allocate memory.
        auto& v = values_();

        v.dataBegin = allocator_traits_::allocate(allocator_(), count);
        v.bufEnd = v.dataEnd = (v.dataBegin + count);

        // Direct-construct elements.
        try
        {
            uninitialized_direct_construct(v.dataBegin, v.dataEnd, args...);
        }
        catch (...)
        {
            allocator_traits_::deallocate(allocator_(), v.dataBegin, count);
            throw;
        }
    }

    void destroy_data_()
    {
        // Destruct elements if necessary.

        // NOTE: This is safe even if dataBegin and dataEnd are both nullptr.

        auto& allocator = allocator_();

        if constexpr (must_call_allocator_destroy_on_elements_())
        {
            const auto& v = values_();

            for (auto it = v.dataBegin; it != v.dataEnd; ++it)
            {
                allocator_traits_::destroy(allocator, it);
            }
        }

        // Deallocate memory.
        allocator_traits_::deallocate(allocator, data(), size());
    }

public:
    using value_type            = T;
    using allocator_type        = Allocator;
    using size_type             = size_type_;
    using difference_type       = typename allocator_traits_::difference_type;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using pointer               = typename allocator_traits_::pointer;
    using const_pointer         = typename allocator_traits_::const_pointer;
    using iterator              = value_type*; // TODO: Should we use pointer instead?
    using const_iterator        = const value_type*; // TODO: Should we use const_pointer instead?
    // TODO: reverse_iterator
    // TODO: const_reverse_iterator

    constexpr allocator_type get_allocator() const noexcept
    {
        return allocator_();
    }

    constexpr const allocator_type& allocator() const noexcept
    {
        return allocator_();
    }

    inline const T* data() const noexcept
    {
        return values_().dataBegin;
    }

    inline T* data() noexcept
    {
        return values_().dataBegin;
    }

    inline size_type size() const noexcept
    {
        return (end() - begin());
    }

    inline size_type capacity() const noexcept
    {
        return (values_().bufEnd - begin());
    }

    constexpr size_type max_size() const noexcept
    {
        // The smallest of the following:
        // - The maximum possible value of difference_type
        // - The maximum count of elements of type T that can be allocated by allocator_traits_

        return std::min<size_type>(
            (std::numeric_limits<difference_type>::max)(),
            allocator_traits_::max_size(allocator_())
        );
    }

    [[nodiscard]] inline bool empty() const noexcept
    {
        return (begin() == end());
    }

    inline const_iterator cbegin() const noexcept
    {
        return values_().dataBegin;
    }

    inline const_iterator begin() const noexcept
    {
        return values_().dataBegin;
    }

    inline iterator begin() noexcept
    {
        return values_().dataBegin;
    }

    inline const_iterator cend() const noexcept
    {
        return values_().dataEnd;
    }

    inline const_iterator end() const noexcept
    {
        return values_().dataEnd;
    }

    inline iterator end() noexcept
    {
        return values_().dataEnd;
    }

    template<typename... Args>
    reference emplace_back(Args&&... args)
    {
        // Reallocate if necessary.
        auto& v = values_();
        if (v.dataEnd == v.bufEnd)
        {
            const auto oldDataCount = size();
            const auto newBufCount = compute_new_buf_count_(oldDataCount + 1);
            
            v.dataBegin = allocator_traits_::reallocate(
                allocator_(), v.dataBegin, oldDataCount, capacity(),
                newBufCount);

            v.dataEnd = (v.dataBegin + oldDataCount);
            v.bufEnd = (v.dataBegin + newBufCount);
        }

        // Construct new element.
        ::new (v.dataEnd) T(std::forward<Args>(args)...);
        return *(v.dataEnd++);
    }

    inline reference push_back(const T& val)
    {
        return emplace_back(val);
    }

    inline reference push_back(T&& val)
    {
        return emplace_back(std::move(val));
    }

    iterator erase(const_iterator pos)
    {
        // 1 2 3 4
        //   ^
        // ^      ^
        //        ^

        // 1 3 4 $
        //       ^

        // 1 3 4 $
        //   ^
        // ^     ^ ^

        // 1 2 3 4
        //       ^

        const auto it = move_strong(
            const_cast<iterator>(pos + 1),
            end(),
            const_cast<iterator>(pos));

        if constexpr (must_call_allocator_destroy_on_elements_())
        {
            allocator_traits_::destroy(allocator_(), it);
        }

        values_().dataEnd = it;
        
        return const_cast<iterator>(pos);
    }

    // TODO: Range erase

    void clear() noexcept
    {
        destroy_data_();
        values_().reset();
    }

    /**
     * @brief Releases ownership of the data buffer
     * to the caller and resets the vector.
     * 
     * After calling this function, it is the caller's
     * responsibility to destruct the elements within the
     * data buffer if necessary and to deallocate the memory
     * using the allocator's deallocate function or equivalent.
     * 
     * @return pointer A pointer to the data buffer, no longer owned by the vector.
     */
    pointer release() noexcept
    {
        const auto dataBuf = data();
        values_().reset();
        return dataBuf;
    }

    inline const_reference operator[](size_type pos) const noexcept
    {
        return data()[pos];
    }

    inline reference operator[](size_type pos) noexcept
    {
        return data()[pos];
    }

    vector& operator=(const vector& other) = delete;

    // TODO: Handle propagate_on_container_move_assignment properly!!!
    vector& operator=(vector&& other) noexcept
    {
        if (&other != this)
        {
            destroy_data_();

            data_ = std::move(data_);
            other.values_().reset();
        }

        return *this;
    }

    constexpr vector()
        noexcept(std::is_nothrow_default_constructible_v<Allocator>)
    {
        values_().reset();
    }

    constexpr explicit vector(const Allocator& allocator)
        noexcept(std::is_nothrow_copy_constructible_v<Allocator>)
        : data_(allocator, {})
    {
    }

    template<typename... Args>
    explicit vector(size_type count, const Args&... args)
    {
        init_with_direct_construct_(count, args...);
    }

    template<typename... Args>
    explicit vector(const Allocator& allocator,
        size_type count, const Args&... args)
        : data_(allocator, {})
    {
        init_with_direct_construct_(count, args...);
    }

    vector(size_type count, const T& val)
    {
        init_with_direct_construct_(count, val);
    }

    vector(const Allocator& allocator, size_type count, const T& val)
        : data_(allocator, {})
    {
        init_with_direct_construct_(count, val);
    }

    vector(const vector& other) = delete; // TODO

    constexpr vector(vector&& other) noexcept
        : data_(std::move(other.data_))
    {
        other.values_().reset();
    }

    inline ~vector()
    {
        destroy_data_();
    }
};
}

#endif
