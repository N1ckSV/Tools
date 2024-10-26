#ifndef _NICKSV_MEMORY
#define _NICKSV_MEMORY
#pragma once


#include "NickSV/Tools/Definitions.h"
#include "NickSV/Tools/TypeTraits.h"

#include <stdexcept>
#include <memory>

// • NICKSV_NOT_NULL_IGNORE - if defined, 
//   NotNull<T> is just an empty template (using NotNull<T> = T) 
//   in both Debug and Release modes
//
// • NICKSV_NOT_NULL_NOT_IGNORE_RELEASE - if defined, 
//   NotNull<T> won't be ignored as a class in Release mode.
//   But nullptr assertion using C assert() from <assert.h> by default,
//   so NOT_NULL_USE_EXCEPTIONS will be auto defined with it
//   
// • NICKSV_NOT_NULL_IGNORE_DEBUG - if defined, 
//   NotNull<T> is just an empty template (using NotNull<T> = T) 
//   in Debug mode

#ifndef NICKSV_NOT_NULL_IGNORE
    #ifndef NDEBUG
        #ifdef NICKSV_NOT_NULL_IGNORE_DEBUG
            #define NICKSV_NOT_NULL_IGNORE
        #endif
    #else
        #ifdef NICKSV_NOT_NULL_NOT_IGNORE_RELEASE
            #ifndef NOT_NULL_USE_EXCEPTIONS
            #define NOT_NULL_USE_EXCEPTIONS
            #endif
        #else
            #define NICKSV_NOT_NULL_IGNORE
        #endif
    #endif
#endif

#ifndef NICKSV_NOT_NULL_IGNORE


#include <string>
#include <assert.h>

#include <algorithm>    // for forward
#include <cstddef>      // for ptrdiff_t, nullptr_t, size_t
#include <system_error> // for hash
#include <utility>

#endif // NICKSV_NOT_NULL_IGNORE





namespace NickSV {
namespace Tools {


struct NotNullException : std::invalid_argument
{
    NotNullException() : std::invalid_argument("nullptr given to NotNull wrapper") {};
};

// Use exception instead of assert() when nullptr given to NotNull wrapper.
// Useful if you want use NotNull in 
#ifdef NOT_NULL_USE_EXCEPTIONS
    #define NOT_NULL_POINTER_ASSERT(ptr) do { if(!ptr) { throw NotNullException(); } } while(0); 
    #define NOT_NULL_NOEXCEPT(expr) noexcept(false)
#else
    #define NOT_NULL_POINTER_ASSERT(ptr) do { assert((ptr) && "nullptr given to NotNull wrapper"); } while(0);
    #define NOT_NULL_NOEXCEPT(expr) noexcept(expr)
#endif //NOT_NULL_USE_EXCEPTIONS


#ifndef NICKSV_NOT_NULL_IGNORE

    // for declval


// Imp of NotNull is borrowed from https://github.com/microsoft/GSL/blob/main/include/gsl/pointers




template<typename T>
using my_ptr_traits = typename std::pointer_traits<typename std::remove_reference<T>::type>;

namespace details
{
    template <typename T, typename = void>
    struct is_comparable_to_nullptr : std::false_type
    {
    };

    template <typename T>
    struct is_comparable_to_nullptr<
        T,
        typename std::enable_if<std::is_convertible<decltype(std::declval<T>() != nullptr), bool>::value>::type>
        : std::true_type
    {
    };

    // Resolves to the more efficient of `const T` or `const T&`, in the context of returning a const-qualified value
    // of type T.
    template<typename T>
    using value_or_reference_return_t = typename std::conditional<
                                            sizeof(T) < 2*sizeof(void*) && std::is_trivially_copy_constructible<T>::value,
                                            const T,
                                            const T&>::type;

} // namespace details

//   NotNull is NOT STRICT 
//   movable nullptr assert wrapper for pointer-like types.
//   Can store nullptr at default initialization and after std::move().
//   Doing nothing in Release mode by default.
//
// • NICKSV_NOT_NULL_IGNORE - if defined, 
//   NotNull<T> is just an empty template (using NotNull<T> = T) 
//   in both Debug and Release modes
//
// • NICKSV_NOT_NULL_NOT_IGNORE_RELEASE - if defined, 
//   NotNull<T> won't be ignored as a class in Release mode.
//   But nullptr assertion using C assert() from <assert.h> by default,
//   so NOT_NULL_USE_EXCEPTIONS will be auto defined with it
//   
// • NICKSV_NOT_NULL_IGNORE_DEBUG - if defined, 
//   NotNull<T> is just an empty template (using NotNull<T> = T) 
//   in Debug mode
//
template<typename PtrT, typename PtrTraits = my_ptr_traits<PtrT>>
class NotNull
{
public:
    static_assert(details::is_comparable_to_nullptr<typename PtrTraits::pointer>::value, "T cannot be compared to nullptr.");

    using pointer_type = typename PtrTraits::pointer;
    using element_type = typename PtrTraits::element_type;

    
    using GetReturnType = details::value_or_reference_return_t<pointer_type>;


    NotNull() noexcept : m_ptr(nullptr) {}


    NotNull(const NotNull& other) : m_ptr(other.m_ptr)
    { 
        NOT_NULL_POINTER_ASSERT(m_ptr);
    }

    NotNull& operator=(const NotNull& other)
    { 
        if(std::addressof(other) != this)
        {
            m_ptr = other.m_ptr;
            NOT_NULL_POINTER_ASSERT(m_ptr);
        }
        return *this;
    }

    NotNull(NotNull&& other) 
    NOT_NULL_NOEXCEPT(std::is_nothrow_move_constructible<pointer_type>::value) 
        : m_ptr(std::move(other.m_ptr))
    {
        NOT_NULL_POINTER_ASSERT(m_ptr);
    }

    NotNull& operator=(NotNull&& other)
    NOT_NULL_NOEXCEPT(std::is_nothrow_move_assignable<pointer_type>::value) 
    { 
        if(std::addressof(other) != this)
        {
            std::swap(m_ptr, other.m_ptr);
            NOT_NULL_POINTER_ASSERT(m_ptr);
        }
        return *this;
    }


    template <typename U, 
        typename std::enable_if<std::is_convertible<typename std::remove_reference<U>::type, pointer_type>::value, bool>::type = true>
    //cppcheck-suppress noExplicitConstructor
    CONSTEXPR_SINCE_CPP14 NotNull(const typename std::remove_reference<U>::type& u) : m_ptr(u)
    { 
        NOT_NULL_POINTER_ASSERT(m_ptr);
    }


    template <typename std::enable_if<!std::is_same<std::nullptr_t, pointer_type>::value, bool>::type = true>
    //cppcheck-suppress noExplicitConstructor
    CONSTEXPR_SINCE_CPP14 NotNull(pointer_type u)
    NOT_NULL_NOEXCEPT(std::is_nothrow_move_constructible<pointer_type>::value) 
        : m_ptr(std::move(u))
    {
        NOT_NULL_POINTER_ASSERT(m_ptr);
    }

    template <typename U, typename UTraits = my_ptr_traits<U>, 
        typename std::enable_if<std::is_convertible<U, pointer_type>::value, bool>::type = true>
    CONSTEXPR_SINCE_CPP14 NotNull(const NotNull<U,UTraits>& other) 
    noexcept(noexcept(NotNull(static_cast<const U&>(other)))) 
        : NotNull(static_cast<const U&>(other))
    {}

    template <typename U, typename UTraits = my_ptr_traits<U>, 
        typename std::enable_if<std::is_convertible<U, pointer_type>::value, bool>::type = true>
    //cppcheck-suppress noExplicitConstructor
    CONSTEXPR_SINCE_CPP14 NotNull(NotNull<U,UTraits>&& other) 
    NOT_NULL_NOEXCEPT(std::is_nothrow_move_constructible<pointer_type>::value) 
        : m_ptr(std::move(other.m_ptr))
    {
        NOT_NULL_POINTER_ASSERT(m_ptr);
    }


    [[nodiscard]] constexpr operator pointer_type() const & noexcept(std::is_nothrow_copy_constructible<pointer_type>::value)
    {
        static_assert(std::is_copy_constructible<pointer_type>::value, 
            "PtrT should be copy constructible");
        return m_ptr; 
    }

    [[nodiscard]] operator pointer_type() && noexcept(std::is_nothrow_move_constructible<pointer_type>::value)
    {
        static_assert(std::is_move_constructible<pointer_type>::value, 
            "PtrT should be move constructible");
        return std::move(m_ptr); 
    }
    
    [[nodiscard]] explicit operator pointer_type&() noexcept { return m_ptr; }

    [[nodiscard]] explicit operator const pointer_type&() const noexcept { return m_ptr; }

    template <class U = pointer_type>
    constexpr auto operator->() const
    -> typename std::enable_if<!std::is_same<U, void*>::value, GetReturnType>::type
    { 
        return Get(); 
    }

    template <class U = pointer_type>
    [[nodiscard]] constexpr auto operator*() const
    -> typename std::enable_if<!std::is_same<U , void*>::value, element_type>::type&
    {
        return *Get();
    }

    // Prevented compilation.
    // Use default NotNull()
    NotNull(std::nullptr_t) = delete;

    // Prevented compilation.
    NotNull& operator=(std::nullptr_t) = delete;

    // unwanted operators...pointers only point to single objects!
    NotNull& operator++() = delete;
    NotNull& operator--() = delete;
    NotNull operator++(int) = delete;
    NotNull operator--(int) = delete;
    NotNull& operator+=(typename PtrTraits::difference_type) = delete;
    NotNull& operator-=(typename PtrTraits::difference_type) = delete;
    void operator[](typename PtrTraits::difference_type) const = delete;

private:
    // Get() is private, because otherwise we can't code like "notNullPtr.Get()"
    // if NICKSV_NOT_NULL_IGNORE is defined (which is done in Release mode by default)
    // and NotNull<T> is just an alias for T (so it doesn't have Get() anyway)
    [[nodiscard]] constexpr GetReturnType Get() const &
    noexcept(std::is_reference<GetReturnType>::value || std::is_nothrow_copy_constructible<pointer_type>::value)
    {
        static_assert(std::is_reference<GetReturnType>::value || 
              std::is_copy_constructible<pointer_type>::value, "PtrT should be copy constructible");
        return m_ptr;
    }

    // Get() is private, because otherwise we can't code like "notNullPtr.Get()"
    // if NICKSV_NOT_NULL_IGNORE is defined (which is done in Release mode by default)
    // and NotNull<T> is just an alias for T (so it doesn't have Get() anyway)
    [[nodiscard]] pointer_type Get() &&
    noexcept(std::is_nothrow_move_constructible<pointer_type>::value)
    {
        static_assert(std::is_move_constructible<pointer_type>::value, "PtrT should be move constructible");
        return std::move(m_ptr);
    }


    pointer_type m_ptr;
};




template <class T, class TTraits = my_ptr_traits<T>>
auto MakeNotNull(T&& t) noexcept
-> NotNull<T, TTraits>
{
    return NotNull<T, TTraits>{std::forward<T>(t)};
}

template <class T, class TTraits = my_ptr_traits<T>>
std::ostream& operator<<(std::ostream& os, const NotNull<T, TTraits>& val)
{
    os << static_cast<const T&>(val);
    return os;
}

template <class T, class U, class TTraits = my_ptr_traits<T>, class UTraits = my_ptr_traits<U>>
auto operator==(const NotNull<T,TTraits>& lhs, const NotNull<U,UTraits>& rhs)
noexcept(noexcept(static_cast<const T&>(lhs) == static_cast<const U&>(rhs)))
    -> decltype(static_cast<const T&>(lhs) == static_cast<const U&>(rhs))
{
    return static_cast<const T&>(lhs) == static_cast<const U&>(rhs);
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator==(const NotNull<T,TTraits>& lhs, const T& rhs)
noexcept(noexcept(static_cast<const T&>(lhs) == rhs))
    -> decltype(static_cast<const T&>(lhs) == rhs)
{
    return (static_cast<const T&>(lhs) == rhs);
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator==(const T& lhs, const NotNull<T,TTraits>& rhs)
noexcept(noexcept(rhs == lhs))
    -> decltype(rhs == lhs)
{
    return (rhs == lhs);
}

template <class T, class U, class TTraits = my_ptr_traits<T>, class UTraits = my_ptr_traits<U>>
auto operator!=(const NotNull<T,TTraits>& lhs, const NotNull<U,UTraits>& rhs)
noexcept(noexcept(static_cast<const T&>(lhs) != static_cast<const U&>(rhs)))
    -> decltype(static_cast<const T&>(lhs) != static_cast<const U&>(rhs))
{
    return static_cast<const T&>(lhs) != static_cast<const U&>(rhs);
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator!=(const NotNull<T,TTraits>& lhs, const T& rhs)
noexcept(noexcept(static_cast<const T&>(lhs) != rhs))
    -> decltype(static_cast<const T&>(lhs) != rhs)
{
    return static_cast<const T&>(lhs) != rhs;
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator!=(const T& lhs, const NotNull<T,TTraits>& rhs)
noexcept(noexcept(rhs != lhs))
    -> decltype(rhs != lhs)
{
    return (rhs != lhs);
}

template <class T, class U, class TTraits = my_ptr_traits<T>, class UTraits = my_ptr_traits<U>>
auto operator<(const NotNull<T,TTraits>& lhs, const NotNull<U,UTraits>& rhs)
noexcept(noexcept(std::less<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs))))
    -> decltype(std::less<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs)))
{
    return std::less<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs));
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator<(const NotNull<T,TTraits>& lhs, const T& rhs)
noexcept(noexcept(std::less<T>{}(static_cast<const T&>(lhs), rhs)))
    -> decltype(std::less<T>{}(static_cast<const T&>(lhs), rhs))
{
    return std::less<T>{}(static_cast<const T&>(lhs), rhs);
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator<(const T& lhs, const NotNull<T,TTraits>& rhs)
noexcept(noexcept(std::less<T>{}(lhs, static_cast<const T&>(rhs))))
    -> decltype(std::less<T>{}(lhs, static_cast<const T&>(rhs)))
{
    return std::less<T>{}(lhs, static_cast<const T&>(rhs));
}


template <class T, class U, class TTraits = my_ptr_traits<T>, class UTraits = my_ptr_traits<U>>
auto operator<=(const NotNull<T,TTraits>& lhs, const NotNull<U,UTraits>& rhs) 
noexcept(noexcept(std::less_equal<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs))))
    -> decltype(std::less_equal<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs)))
{
    return std::less_equal<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs));
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator<=(const NotNull<T,TTraits>& lhs, const T& rhs)
noexcept(noexcept(std::less_equal<T>{}(static_cast<const T&>(lhs), rhs)))
    -> decltype(std::less_equal<T>{}(static_cast<const T&>(lhs), rhs))
{
    return std::less_equal<T>{}(static_cast<const T&>(lhs), rhs);
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator<=(const T& lhs, const NotNull<T,TTraits>& rhs)
noexcept(noexcept(std::less_equal<T>{}(lhs, static_cast<const T&>(rhs))))
    -> decltype(std::less_equal<T>{}(lhs, static_cast<const T&>(rhs)))
{
    return std::less_equal<T>{}(lhs, static_cast<const T&>(rhs));
}

template <class T, class U, class TTraits = my_ptr_traits<T>, class UTraits = my_ptr_traits<U>>
auto operator>(const NotNull<T,TTraits>& lhs, const NotNull<U,UTraits>& rhs)
noexcept(noexcept(std::greater<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs))))
    -> decltype(std::greater<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs)))
{
    return std::greater<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs));
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator>(const NotNull<T,TTraits>& lhs, const T& rhs)
noexcept(noexcept(std::greater<T>{}(static_cast<const T&>(lhs), rhs)))
    -> decltype(std::greater<T>{}(static_cast<const T&>(lhs), rhs))
{
    return std::greater<T>{}(static_cast<const T&>(lhs), rhs);
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator>(const T& lhs, const NotNull<T,TTraits>& rhs)
noexcept(noexcept(std::greater<T>{}(lhs, static_cast<const T&>(rhs))))
    -> decltype(std::greater<T>{}(lhs, static_cast<const T&>(rhs)))
{
    return std::greater<T>{}(lhs, static_cast<const T&>(rhs));
}

template <class T, class U, class TTraits = my_ptr_traits<T>, class UTraits = my_ptr_traits<U>>
auto operator>=(const NotNull<T,TTraits>& lhs, const NotNull<U,UTraits>& rhs)
noexcept(noexcept(std::greater_equal<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs))))
    -> decltype(std::greater_equal<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs)))
{
    return std::greater_equal<T>{}(static_cast<const T&>(lhs), static_cast<const U&>(rhs));
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator>=(const NotNull<T,TTraits>& lhs, const T& rhs)
noexcept(noexcept(std::greater_equal<T>{}(static_cast<const T&>(lhs), rhs)))
    -> decltype(std::greater_equal<T>{}(static_cast<const T&>(lhs), rhs))
{
    return std::greater_equal<T>{}(static_cast<const T&>(lhs), rhs);
}

template <class T, class TTraits = my_ptr_traits<T>>
auto operator>=(const T& lhs, const NotNull<T,TTraits>& rhs)
noexcept(noexcept(std::greater_equal<T>{}(lhs, static_cast<const T&>(rhs))))
    -> decltype(std::greater_equal<T>{}(lhs, static_cast<const T&>(rhs)))
{
    return std::greater_equal<T>{}(lhs, static_cast<const T&>(rhs));
}

// more unwanted operators
template <class T, class TTraits = my_ptr_traits<T>>
using default_diff_type = typename TTraits::difference_type;

template <class T, class U, class TTraits = my_ptr_traits<T>, class UTraits = my_ptr_traits<U>>
default_diff_type<T> operator-(const NotNull<T, TTraits>&, const NotNull<U,UTraits>&) = delete;
template <class T, class TTraits = my_ptr_traits<T>>
NotNull<T, TTraits> operator-(const NotNull<T, TTraits>&, default_diff_type<T>) = delete;
template <class T, class TTraits = my_ptr_traits<T>>
NotNull<T, TTraits> operator+(const NotNull<T, TTraits>&, default_diff_type<T>) = delete;
template <class T, class TTraits = my_ptr_traits<T>>
NotNull<T, TTraits> operator+(default_diff_type<T>, const NotNull<T, TTraits>&) = delete;



#else

//   Movable nullptr assert wrapper for pointer-like types.
//   Can store nullptr after std::move().
//   Doing nothing in Release mode by default.
//
//   USEFUL DEFINES:
// • NICKSV_NOT_NULL_IGNORE - if defined, 
//   NotNull<T> is just an empty template (using NotNull<T> = T) 
//   in both Debug and Release modes
//
// • NICKSV_NOT_NULL_NOT_IGNORE_RELEASE - if defined, 
//   NotNull<T> won't be ignored as a class in Release mode.
//   But nullptr assertion using C assert() from <assert.h> by default,
//   so NOT_NULL_USE_EXCEPTIONS will be auto defined with it
//   
// • NICKSV_NOT_NULL_IGNORE_DEBUG - if defined, 
//   NotNull<T> is just an empty template (using NotNull<T> = T) 
//   in Debug mode
//
template<typename T, typename = void>
using NotNull = T;


#endif // NICKSV_NOT_NULL_IGNORE


template <class T, 
    bool = std::is_default_constructible<std::hash<typename T::pointer_type>>::value>
struct NotNullHash
{
    using convert_type = const typename T::pointer_type&;
    using pointer_type = typename std::remove_cv<typename T::pointer_type>::type;
    std::size_t operator()(const T& value) const 
    { 
        return std::hash<pointer_type>{}(static_cast<convert_type>(value)); 
    }
};

template <class T>
struct NotNullHash<T, false>
{
    NotNullHash() = delete;
    NotNullHash(const NotNullHash&) = delete;
    NotNullHash& operator=(const NotNullHash&) = delete;
};



}}  /*END OF NAMESPACES*/



#ifndef NICKSV_NOT_NULL_IGNORE

namespace std
{
template <class PtrT>
struct hash<NickSV::Tools::NotNull<PtrT> > 
    : NickSV::Tools::NotNullHash<NickSV::Tools::NotNull<PtrT>>
{};

template <class PtrT, typename PtrTraits>
struct hash<NickSV::Tools::NotNull<PtrT, PtrTraits> > 
    : NickSV::Tools::NotNullHash<NickSV::Tools::NotNull<PtrT, PtrTraits>>
{};
}

#endif // NICKSV_NOT_NULL_IGNORE



namespace NickSV {
namespace Tools {


template<class T, class... Args>
inline auto MakeUnique(Args&&... args)
-> std::enable_if_t<!std::is_array<T>::value, std::unique_ptr<T>>
{
    #ifdef __cpp_lib_make_unique
    return std::make_unique<T>(std::forward<Args>(args)...);
    #else
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    #endif
}


template<class T, class... Args>
inline auto MakeShared(Args&&... args)
-> std::enable_if_t<!std::is_array<T>::value, std::shared_ptr<T>>
{
    #ifdef __cpp_lib_make_shared
    return std::make_shared<T>(std::forward<Args>(args)...);
    #else
    return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
    #endif
}


}}




#endif // _NICKSV_MEMORY