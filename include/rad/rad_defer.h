/**
 * @file rad_defer.h
 * @author Graham Scott
 * @brief Header file providing defer functionality, similar to defer in Go and Zig.
 * @version 0.1
 * @date 2023-04-07
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 */
#ifndef RAD_DEFER_H_INCLUDED
#define RAD_DEFER_H_INCLUDED

namespace rad
{
namespace detail_
{
    template<class Func>
    class deferrer_
    {
        Func deferredFunc_;

    public:
        inline deferrer_(Func deferredFunc) noexcept
            : deferredFunc_(deferredFunc)
        {
        }

        inline ~deferrer_()
        {
            deferredFunc_();
        }
    };

    #define RAD_DEFER_VAR_NAME2_(lineNumber) ZZZZ_deferrer_from_line_##lineNumber##_
    #define RAD_DEFER_VAR_NAME1_(lineNumber) RAD_DEFER_VAR_NAME2_(lineNumber)

    #define RAD_DEFER const ::rad::detail_::deferrer_ RAD_DEFER_VAR_NAME1_(__LINE__) = [&]()
}
}

#endif
