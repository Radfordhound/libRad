/**
 * @file rad_scoped_enum_helpers.h
 * @author Graham Scott
 * @brief Header file providing helpers for scoped enums.
 * @version 0.1
 * @date 2023-07-29
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 * 
 */
#ifndef RAD_SCOPED_ENUM_HELPERS
#define RAD_SCOPED_ENUM_HELPERS

#include <type_traits>

namespace rad
{
namespace detail_
{
    template<typename ScopedEnumType>
    class scoped_enum_bitwise_val
    {
        const ScopedEnumType    value_{};

    public:
        constexpr operator ScopedEnumType() const noexcept
        {
            return value_;
        }

        constexpr explicit operator bool() const noexcept
        {
            using underlying_type_ = std::underlying_type_t<ScopedEnumType>;

            return (static_cast<underlying_type_>(value_) != underlying_type_(0));
        }

        constexpr scoped_enum_bitwise_val(ScopedEnumType value = ScopedEnumType{}) noexcept
            : value_(value)
        {
        }
    };
}

#define RAD_ENABLE_SCOPED_ENUM_BITWISE_OPS(scopedEnumType)\
    constexpr ::rad::detail_::scoped_enum_bitwise_val<scopedEnumType> operator&(\
        scopedEnumType a, scopedEnumType b) noexcept\
    {\
        using int_t = std::underlying_type_t<scopedEnumType>;\
        return static_cast<scopedEnumType>(static_cast<int_t>(a) & static_cast<int_t>(b));\
    }\
\
    constexpr scopedEnumType operator|(scopedEnumType a, scopedEnumType b) noexcept\
    {\
        using int_t = std::underlying_type_t<scopedEnumType>;\
        return static_cast<scopedEnumType>(static_cast<int_t>(a) | static_cast<int_t>(b));\
    }\
\
    constexpr scopedEnumType operator^(scopedEnumType a, scopedEnumType b) noexcept\
    {\
        using int_t = std::underlying_type_t<scopedEnumType>;\
        return static_cast<scopedEnumType>(static_cast<int_t>(a) ^ static_cast<int_t>(b));\
    }\
\
    constexpr scopedEnumType operator~(scopedEnumType a) noexcept\
    {\
        using int_t = std::underlying_type_t<scopedEnumType>;\
        return static_cast<scopedEnumType>(~static_cast<int_t>(a));\
    }\
\
    template<typename ShiftType>\
    constexpr scopedEnumType operator<<(scopedEnumType a, ShiftType b) noexcept\
    {\
        using int_t = std::underlying_type_t<scopedEnumType>;\
        return static_cast<scopedEnumType>(static_cast<int_t>(a) << b);\
    }\
\
    template<typename ShiftType>\
    constexpr scopedEnumType operator>>(scopedEnumType a, ShiftType b) noexcept\
    {\
        using int_t = std::underlying_type_t<scopedEnumType>;\
        return static_cast<scopedEnumType>(static_cast<int_t>(a) >> b);\
    }\
\
    constexpr scopedEnumType& operator&=(scopedEnumType& a, scopedEnumType b) noexcept\
    {\
        return ((a = (a & b)));\
    }\
\
    constexpr scopedEnumType& operator|=(scopedEnumType& a, scopedEnumType b) noexcept\
    {\
        return ((a = (a | b)));\
    }\
\
    constexpr scopedEnumType& operator^=(scopedEnumType& a, scopedEnumType b) noexcept\
    {\
        return ((a = (a ^ b)));\
    }\
\
    template<typename ShiftType>\
    constexpr scopedEnumType& operator>>=(scopedEnumType a, ShiftType b) noexcept\
    {\
        return ((a = (a >> b)));\
    }\
\
    template<typename ShiftType>\
    constexpr scopedEnumType& operator<<=(scopedEnumType a, ShiftType b) noexcept\
    {\
        return ((a = (a << b)));\
    }
}

#endif
