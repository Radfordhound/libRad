/// @file rad_ref_count_ptr.h
/// @author Graham Scott
/// @brief Header file providing rad::ref_count_ptr; a template class
/// which represents a simple pointer to an object which inherits from
/// rad::ref_count_object, and which automatically increments and decrements
/// the reference count as necessary, similar to std::unique_ptr.
/// @date 2024-06-14
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_REF_COUNT_PTR_H_INCLUDED
#define RAD_REF_COUNT_PTR_H_INCLUDED

#include <utility>
#include <type_traits>
#include <cstddef>

namespace rad
{
template<class T>
class ref_count_ptr
{
    T* ptr_ = nullptr;

    void release_ref_() noexcept
    {
        if (ptr_ && ptr_->release_ref())
        {
            delete ptr_;
        }
    }

    void reset_no_add_ref_(T* refCountObjPtr) noexcept
    {
        release_ref_();
        
        ptr_ = refCountObjPtr;
    }

public:
    using pointer = T*;
    using element_type = T;

    inline pointer get() const noexcept
    {
        return ptr_;
    }

    inline void reset() noexcept
    {
        reset_no_add_ref_(nullptr);
    }

    void reset(pointer refCountObjPtr) noexcept
    {
        if (refCountObjPtr)
        {
            refCountObjPtr->add_ref();
        }

        reset_no_add_ref_(refCountObjPtr);
    }

    [[nodiscard]] pointer detach() noexcept
    {
        const auto oldPtr = ptr_;

        ptr_ = nullptr;

        return oldPtr;
    }

    void swap(ref_count_ptr& other) noexcept
    {
        std::swap(ptr_, other.ptr_);
    }

    ref_count_ptr& operator=(const ref_count_ptr& other) noexcept
    {
        if (&other != this)
        {
            reset(other.ptr_);
        }

        return *this;
    }

    ref_count_ptr& operator=(ref_count_ptr&& other) noexcept
    {
        reset_no_add_ref_(other.detach());
        return *this;
    }

    template<class U>
    ref_count_ptr& operator=(ref_count_ptr<U>&& other) noexcept
    {
        reset_no_add_ref_(other.detach());
        return *this;
    }

    inline ref_count_ptr& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    inline ref_count_ptr& operator=(pointer refCountObjPtr) noexcept
    {
        reset(refCountObjPtr);
        return *this;
    }

    inline explicit operator bool() const noexcept
    {
        return (ptr_ != nullptr);
    }

    inline std::add_lvalue_reference_t<T> operator*() const
        noexcept(noexcept(*std::declval<pointer>()))
    {
        return *ptr_;
    }

    inline pointer operator->() const noexcept
    {
        return ptr_;
    }

    constexpr ref_count_ptr() noexcept = default;

    constexpr ref_count_ptr(std::nullptr_t) noexcept
    {
    }

    ref_count_ptr(pointer refCountObjPtr) noexcept
        : ptr_(refCountObjPtr)
    {
        if (ptr_)
        {
            ptr_->add_ref();
        }
    }

    ref_count_ptr(const ref_count_ptr& other) noexcept
        : ref_count_ptr(other.ptr_)
    {
    }

    template<class U>
    ref_count_ptr(const ref_count_ptr<U>& other) noexcept
        : ref_count_ptr(other.ptr_)
    {
    }

    ref_count_ptr(ref_count_ptr&& other) noexcept
        : ptr_(other.detach())
    {
    }

    template<class U>
    ref_count_ptr(ref_count_ptr<U>&& other) noexcept
        : ptr_(other.detach())
    {
    }

    inline ~ref_count_ptr()
    {
        release_ref_();
    }
};

template<class T1, class T2>
inline bool operator==(const ref_count_ptr<T1>& x, const ref_count_ptr<T2>& y) noexcept
{
    return (x.get() == y.get());
}

template<class T>
inline bool operator==(const ref_count_ptr<T>& x, std::nullptr_t) noexcept
{
    return !x;
}

template<class T>
inline bool operator==(std::nullptr_t, const ref_count_ptr<T>& x) noexcept
{
    return !x;
}

template<class T1, class T2>
inline bool operator!=(const ref_count_ptr<T1>& x, const ref_count_ptr<T2>& y) noexcept
{
    return (x.get() != y.get());
}

template<class T>
inline bool operator!=(const ref_count_ptr<T>& x, std::nullptr_t) noexcept
{
    return static_cast<bool>(x);
}

template<class T>
inline bool operator!=(std::nullptr_t, const ref_count_ptr<T>& x) noexcept
{
    return static_cast<bool>(x);
}
}

#endif

