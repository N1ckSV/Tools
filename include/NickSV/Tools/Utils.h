#ifndef _NICKSV_UTILS
#define _NICKSV_UTILS
#pragma once



#include <cstring>
#include <future>


#include "NickSV/Tools/TypeTraits.h"






namespace NickSV {
namespace Tools {




template<class CharT, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT>>
CharT* MutableStringData(std::basic_string<CharT, Traits, Allocator>& str)
{
#if __cplusplus >= CXX17_VERSION
    return str.data();
#else
    return &(str[0]);
#endif
}







/**
 * @brief Runtime conversation from [char const *]
 *        to [std::basic_string<CharT>]
 * @note 
 * For compile time 
*/
template <typename CharT, typename NoCVCharT = std::remove_cv_t<CharT>>
std::basic_string<NoCVCharT> basic_string_convert(char const *pcszToConvert)
{
    std::basic_string<NoCVCharT> bstr;
    size_t sz = strlen(pcszToConvert);
    bstr.resize(sz);
    std::copy(pcszToConvert, pcszToConvert + sz, bstr.begin());
    return bstr; 
}

template <> std::string inline basic_string_convert<               char, char>(char const * pcszToConvert) { return pcszToConvert; };
template <> std::string inline basic_string_convert<         const char, char>(char const * pcszToConvert) { return pcszToConvert; };
template <> std::string inline basic_string_convert<      volatile char, char>(char const * pcszToConvert) { return pcszToConvert; };
template <> std::string inline basic_string_convert<const volatile char, char>(char const * pcszToConvert) { return pcszToConvert; };




namespace details
{
    template<class  > struct _is_reference_wrapper : std::false_type {};
    template<class T> struct _is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

    template<class T> struct _remove_reference_wrapper { using type = T; };
    template<class T> struct _remove_reference_wrapper<std::reference_wrapper<T>> { using type = T; };

    template<class T> using _remove_reference_wrapper_t = typename _remove_reference_wrapper<T>::type;

}

template<typename T, 
        typename V = typename std::conditional_t<
        details::_is_reference_wrapper<std::decay_t<T>>::value,
        details::_remove_reference_wrapper_t<std::decay_t<T>>&,
        std::decay_t<T>
        >>
inline std::future<V> MakeReadyFuture(T&& value)
{
    std::promise<V> tmpPromise;
    auto fut = tmpPromise.get_future();
    tmpPromise.set_value(std::forward<T>(value));
    return fut;
}

inline std::future<void> MakeReadyFuture()
{
    std::promise<void> tmpPromise;
    auto future = tmpPromise.get_future();
    tmpPromise.set_value();
    return future;
}



// In GCC 14.1 version noexcept_expr mangling is finally impl-ted
// (otherwise SFINAE noexcept doesn't work here).
// Assumed that other compilers just have it
#if !defined(__GNUC__) || (__GNUC__ * 10 + __GNUC_MINOR__ > 132)



// Same as std::for_each(first, last, tryFunc):
// iterates range and invokes tryFunc with each element,
// but if tryFunc throws an exception - iterates back 
// and invokes catchFunc with each element.
//
// ATTENTION:
//      catchFunc is NOT invoked with element 
//      that tryFunc was invoked and thrown exception with
//      (see for_each_exception_safe_last to change it).
//
// THROWS:
//      Same exception as tryFunc.
//      Exception thrown by catchFunc is ignored
template<class InputIt, class TryFunctorType, class CatchFunctorType>
auto for_each_exception_safe(
    InputIt first, InputIt last, TryFunctorType tryFunc, CatchFunctorType catchFunc) -> 
    std::enable_if_t<!noexcept(tryFunc(*first)) && 
    !noexcept(catchFunc(*first)), void>
 {
    InputIt var = first;
    try
    {
        while(var != last)
        {
            tryFunc(*var);
            ++var;
        }
    }
    catch(...)
    {
        while(var != first)
        {
            --var;
            try { catchFunc(*var); }
            catch(...) {}
        }
        throw;
    }
}

// Same as std::for_each(first, last, tryFunc):
// iterates range and invokes tryFunc with each element,
// but if tryFunc throws an exception - iterates back 
// and invokes catchFunc with each element.
//
// ATTENTION:
//      catchFunc IS invoked with element 
//      that tryFunc was invoked and thrown exception with
//      (see for_each_exception_safe to change it).
//
// THROWS:
//      Same exception as tryFunc.
//      Exception thrown by catchFunc is ignored
template<class InputIt, class TryFunctorType, class CatchFunctorType>
auto for_each_exception_safe_last(
    InputIt first, InputIt last, TryFunctorType tryFunc, CatchFunctorType catchFunc) -> 
    std::enable_if_t<!noexcept(tryFunc(*first)) && 
    !noexcept(catchFunc(*first)), void>
 {
    InputIt var = first;
    try
    {
        while(var != last)
        {
            tryFunc(*var);
            ++var;
        }
    }
    catch(...)
    {
        try { catchFunc(*var); }
        catch(...) {}
        while(var != first)
        {
            --var;
            try { catchFunc(*var); }
            catch(...) {}
        }
        throw;
    }
}
                                                                
// Same as std::for_each(first, last, tryFunc):
// iterates range and invokes tryFunc with each element,
// but if tryFunc throws an exception - iterates back 
// and invokes catchFunc with each element.
// Optimized for noexcept catchFunc.
//
// ATTENTION:
//      catchFunc is NOT invoked with element 
//      that tryFunc was invoked and thrown exception with
//      (see for_each_exception_safe_last to change it)
//
// THROWS:
//      Same exception as tryFunc
template<class InputIt, class TryUnaryFunc, class CatchUnaryFunc>
auto for_each_exception_safe(
    InputIt first, InputIt last, TryUnaryFunc tryFunc, CatchUnaryFunc catchFunc) -> 
    std::enable_if_t<!noexcept(tryFunc(*first)) && 
    noexcept(catchFunc(*first)), void>
{
    InputIt var = first;
    try
    {
        while(var != last)
        {
            tryFunc(*var);
            ++var;
        }
    }
    catch(...)
    {
        while(var != first)
        {
            --var;
            catchFunc(*var);
        }
        throw;
    }
}

// Same as std::for_each(first, last, tryFunc):
// iterates range and invokes tryFunc with each element,
// but if tryFunc throws an exception - iterates back 
// and invokes catchFunc with each element.
// Optimized for noexcept catchFunc.
//
// ATTENTION:
//      catchFunc IS invoked with element 
//      that tryFunc was invoked and thrown exception with
//      (see for_each_exception_safe to change it)
//
// THROWS:
//      Same exception as tryFunc
template<class InputIt, class TryUnaryFunc, class CatchUnaryFunc>
auto for_each_exception_safe_last(
    InputIt first, InputIt last, TryUnaryFunc tryFunc, CatchUnaryFunc catchFunc) -> 
    std::enable_if_t<!noexcept(tryFunc(*first)) && 
    noexcept(catchFunc(*first)), void>
{
    InputIt var = first;
    try
    {
        while(var != last)
        {
            tryFunc(*var);
            ++var;
        }
    }
    catch(...)
    {
        catchFunc(*var);
        while(var != first)
        {
            --var;
            catchFunc(*var);
        }
        throw;
    }
}


// Same as std::for_each(first, last, tryFunc):
// iterates range and invokes tryFunc with each element,
// tryFunc is noexcept so this function is optimized,
// specified as noexcept and catchFunc is ignored.
//
// THROWS: Nothing
template<class InputIt, class TryUnaryFunc, class CatchUnaryFunc>
auto for_each_exception_safe(
    InputIt first, InputIt last, TryUnaryFunc tryFunc, CatchUnaryFunc) noexcept -> 
    std::enable_if_t<noexcept(tryFunc(*first)), void>
{
    for (; first != last; ++first)
        tryFunc(*first);
}


template<class InputIt, class TryUnaryFunc, class CatchUnaryFunc>
auto for_each_exception_safe_last(
    InputIt first, InputIt last, TryUnaryFunc tryFunc, CatchUnaryFunc) noexcept -> 
    std::enable_if_t<noexcept(tryFunc(*first)), void>
{
    for (; first != last; ++first)
        tryFunc(*first);
}


#else



// Same as std::for_each(first, last, tryFunc):
// iterates range and invokes tryFunc with each element,
// but if tryFunc throws an exception - iterates back 
// and invokes catchFunc with each element.
//
// ATTENTION:
//      catchFunc is NOT invoked with element 
//      that tryFunc was invoked and thrown exception with
//      (see for_each_exception_safe_last to change it).
//
// THROWS:
//      Same exception as tryFunc.
//      Exception thrown by catchFunc is ignored
template<class InputIt, class TryFunctorType, class CatchFunctorType>
void for_each_exception_safe(InputIt first, InputIt last, TryFunctorType tryFunc, CatchFunctorType catchFunc)
{
    InputIt var = first;
    try
    {
        while(var != last)
        {
            tryFunc(*var);
            ++var;
        }
    }
    catch(...)
    {
        while(var != first)
        {
            --var;
            try { catchFunc(*var); }
            catch(...) {}
        }
        throw;
    }
}

// Same as std::for_each(first, last, tryFunc):
// iterates range and invokes tryFunc with each element,
// but if tryFunc throws an exception - iterates back 
// and invokes catchFunc with each element.
//
// ATTENTION:
//      catchFunc IS invoked with element 
//      that tryFunc was invoked and thrown exception with
//      (see for_each_exception_safe to change it).
//
// THROWS:
//      Same exception as tryFunc.
//      Exception thrown by catchFunc is ignored
template<class InputIt, class TryFunctorType, class CatchFunctorType>
void for_each_exception_safe_last(InputIt first, InputIt last, TryFunctorType tryFunc, CatchFunctorType catchFunc)
{
    InputIt var = first;
    try
    {
        while(var != last)
        {
            tryFunc(*var);
            ++var;
        }
    }
    catch(...)
    {
        try { catchFunc(*var); }
        catch(...) {}
        while(var != first)
        {
            --var;
            try { catchFunc(*var); }
            catch(...) {}
        }
        throw;
    }
}
#endif


}}  /*END OF NAMESPACES*/




#endif // _NICKSV_UTILS
