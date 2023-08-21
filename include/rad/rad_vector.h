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

    struct values_
    {
        T*  dataBegin = nullptr;
        T*  dataEnd = nullptr;
        T*  bufEnd = nullptr;

        void reset() noexcept
        {
            dataBegin = nullptr;
            dataEnd = nullptr;
            bufEnd = nullptr;
        }
    };

    pair<Allocator, values_>    data_;

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

    void destroy_data_()
    {
        // Destruct elements if necessary.

        // NOTE: This is safe even if dataBegin and dataEnd are both nullptr.

        Allocator& allocator = data_.first();

        if constexpr (must_call_allocator_destroy_on_elements_())
        {
            const values_& v = data_.second();

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

    inline const T* data() const noexcept
    {
        return data_.second().dataBegin;
    }

    inline T* data() noexcept
    {
        return data_.second().dataBegin;
    }

    inline size_type size() const noexcept
    {
        return (end() - begin());
    }

    inline size_type capacity() const noexcept
    {
        return (data_.second().bufEnd - begin());
    }

    constexpr size_type max_size() const noexcept
    {
        // The smallest of the following:
        // - The maximum possible value of difference_type
        // - The maximum count of elements of type T that can be allocated by allocator_traits_

        return std::min<size_type>(
            (std::numeric_limits<difference_type>::max)(),
            allocator_traits_::max_size(data_.first())
        );
    }

    inline const_iterator cbegin() const noexcept
    {
        return data_.second().dataBegin;
    }

    inline const_iterator begin() const noexcept
    {
        return data_.second().dataBegin;
    }

    inline iterator begin() noexcept
    {
        return data_.second().dataBegin;
    }

    inline const_iterator cend() const noexcept
    {
        return data_.second().dataEnd;
    }

    inline const_iterator end() const noexcept
    {
        return data_.second().dataEnd;
    }

    inline iterator end() noexcept
    {
        return data_.second().dataEnd;
    }

    template<typename... Args>
    reference emplace_back(Args&&... args)
    {
        // Reallocate if necessary.
        values_& v = data_.second();
        if (v.dataEnd == v.bufEnd)
        {
            const auto oldDataCount = size();
            const auto newBufCount = compute_new_buf_count_(oldDataCount + 1);
            
            v.dataBegin = allocator_traits_::reallocate(
                data_.first(), data(), oldDataCount, capacity(),
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
            allocator_traits_::destroy(data_.first(), it);
        }

        data_.second().dataEnd = it;
        
        return const_cast<iterator>(pos);
    }

    // TODO: Range erase

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
        data_.second().reset();
        return dataBuf;
    }

    vector& operator=(const vector& other) = delete;

    // TODO: Handle propagate_on_container_move_assignment properly!!!
    vector& operator=(vector&& other) noexcept
    {
        if (&other != this)
        {
            destroy_data_();

            data_ = std::move(data_);
            other.data_.second().reset();
        }

        return *this;
    }

    vector() noexcept = default;

    vector(const vector& other) = delete; // TODO

    vector(vector&& other) noexcept :
        data_(std::move(other.data_))
    {
        other.data_.second().reset();
    }

    inline ~vector()
    {
        destroy_data_();
    }
};
} // rad

#endif
