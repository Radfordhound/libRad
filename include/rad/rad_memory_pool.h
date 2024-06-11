/// @file rad_memory_pool.h
/// @author Graham Scott
/// @brief Header file providing memory pools.
/// @date 2023-10-10
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_MEMORY_POOL_H_INCLUDED
#define RAD_MEMORY_POOL_H_INCLUDED

#include "rad_memory.h"
#include "rad_object_utils.h"
#include "rad_vector.h"
#include <memory>
#include <utility>
#include <cstddef>
#include <cassert>

namespace rad
{
namespace detail_
{
    template<typename T>
    union memory_pool_element
    {
        memory_pool_element*    next;
        T                       data;
    };

    template<typename T>
    class memory_pool_block
    {
        std::unique_ptr<unsigned char[]>    elements_;

    public:
        inline memory_pool_element<T>* data() const noexcept
        {
            return reinterpret_cast<memory_pool_element<T>*>(elements_.get());
        }

        memory_pool_block() noexcept = default;

        memory_pool_block(std::size_t elementCount)
            : elements_(RAD_NEW(unsigned char)[sizeof(memory_pool_element<T>) * elementCount])
        {
            // Validate arguments.
            assert((elementCount > 0) &&
                "elementCount must be non-zero");

            // Initialize all elements.
            auto element = data();
            const auto lastElement = (element + (elementCount - 1));

            while (element != lastElement)
            {
                element->next = (element + 1);
                ++element;
            }

            element->next = nullptr;
        }
    };
}

template<typename T>
class fixed_memory_pool
{
    detail_::memory_pool_block<T>       block_;
    detail_::memory_pool_element<T>*    firstFreeElement_ = nullptr;

public:
    [[nodiscard]] T* allocate() noexcept
    {
        // If no elements are free, the allocation failed; return nullptr.
        if (!firstFreeElement_)
        {
            return nullptr;
        }

        // Return pointer to the first free element's data, and update our pointer.
        const auto element = firstFreeElement_;
        firstFreeElement_ = element->next;

        return &element->data;
    }

    void deallocate(T* ptr) noexcept
    {
        const auto element = reinterpret_cast<detail_::memory_pool_element<T>*>(ptr);

        // Reclaim the given element, and update our pointer.
        element->next = firstFreeElement_;
        firstFreeElement_ = element;
    }

    fixed_memory_pool& operator=(const fixed_memory_pool& other) = delete;

    fixed_memory_pool& operator=(fixed_memory_pool&& other) noexcept
    {
        if (&other != this)
        {
            block_ = std::move(other.block_);
            firstFreeElement_ = other.firstFreeElement_;

            other.firstFreeElement_ = nullptr;
        }

        return *this;
    }

    fixed_memory_pool() noexcept = default;

    fixed_memory_pool(std::size_t elementCount)
        : block_(elementCount)
        , firstFreeElement_(block_.data())
    {
    }

    fixed_memory_pool(const fixed_memory_pool& other) = delete;

    fixed_memory_pool(fixed_memory_pool&& other) noexcept
        : block_(std::move(other.block_))
        , firstFreeElement_(other.firstFreeElement_)
    {
        other.firstFreeElement_ = nullptr;
    }
};

template<typename T>
class dynamic_memory_pool
{
    vector<detail_::memory_pool_block<T>>   blocks_;
    detail_::memory_pool_element<T>*        firstFreeElement_ = nullptr;
    std::size_t                             elementsPerBlock_;

public:
    [[nodiscard]] T* allocate()
    {
        // If no elements are free, allocate a new block.
        detail_::memory_pool_element<T>* element;

        if (!firstFreeElement_)
        {
            const auto& newestBlock = blocks_.emplace_back(elementsPerBlock_);
            element = newestBlock.data();
        }
        else
        {
            element = firstFreeElement_;
        }

        // Return pointer to the first free element's data, and update our pointer.
        firstFreeElement_ = element->next;

        return &element->data;
    }

    void deallocate(T* ptr) noexcept
    {
        const auto element = reinterpret_cast<detail_::memory_pool_element<T>*>(ptr);

        // Reclaim the given element, and update our pointer.
        element->next = firstFreeElement_;
        firstFreeElement_ = element;
    }

    dynamic_memory_pool& operator=(const dynamic_memory_pool& other) = delete;

    dynamic_memory_pool& operator=(dynamic_memory_pool&& other) noexcept
    {
        if (&other != this)
        {
            blocks_ = std::move(other.blocks_);
            firstFreeElement_ = other.firstFreeElement_;
            elementsPerBlock_ = other.elementsPerBlock_;

            other.firstFreeElement_ = nullptr;
        }

        return *this;
    }

    dynamic_memory_pool() noexcept = default;

    dynamic_memory_pool(std::size_t elementsPerBlock)
        : blocks_(1, elementsPerBlock)
        , firstFreeElement_(blocks_[0].data())
        , elementsPerBlock_(elementsPerBlock)
    {
    }

    dynamic_memory_pool(const dynamic_memory_pool& other) = delete;

    dynamic_memory_pool(dynamic_memory_pool&& other) noexcept
        : blocks_(std::move(other.blocks_))
        , firstFreeElement_(other.firstFreeElement_)
        , elementsPerBlock_(other.elementsPerBlock_)
    {
        other.firstFreeElement_ = nullptr;
    }
};
}

#endif
