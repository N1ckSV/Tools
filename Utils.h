#ifndef _NICKSV_UTILS
#define _NICKSV_UTILS
#pragma once



#include <cstring>


#include "Definitions.h"
#include "TypeTraits.h"



namespace NickSV::Tools {







/*
 Runtime conversation from [char const *] to [std::basic_string<CharT>]
*/
template <typename CharT, typename NoCVCharT = typename std::remove_cv<CharT>::type>
std::basic_string<NoCVCharT> basic_string_cast(char const *pcszToConvert)
{
    std::basic_string<NoCVCharT> bstr;
    size_t sz = strlen(pcszToConvert);
    bstr.resize(sz);
    std::copy(pcszToConvert, pcszToConvert + sz, bstr.begin());
    return bstr; 
}

template <> std::string inline basic_string_cast<               char, char>(char const * pcszToConvert) { return pcszToConvert; };
template <> std::string inline basic_string_cast<         const char, char>(char const * pcszToConvert) { return pcszToConvert; };
template <> std::string inline basic_string_cast<      volatile char, char>(char const * pcszToConvert) { return pcszToConvert; };
template <> std::string inline basic_string_cast<const volatile char, char>(char const * pcszToConvert) { return pcszToConvert; };




















// Hope in 13.2+ GCC ver noexcept_expr mangling is impl-ted
// Seems like in GCC 14 they did it. Assumed that other compilers have it.
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
    InputIt first, InputIt last, TryFunctorType tryFunc, CatchFunctorType catchFunc) -> typename 
    std::enable_if<!noexcept(tryFunc(*first)) && 
    !noexcept(catchFunc(*first)), void>::type
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
    InputIt first, InputIt last, TryFunctorType tryFunc, CatchFunctorType catchFunc) -> typename 
    std::enable_if<!noexcept(tryFunc(*first)) && 
    !noexcept(catchFunc(*first)), void>::type
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
    InputIt first, InputIt last, TryUnaryFunc tryFunc, CatchUnaryFunc catchFunc) -> typename 
    std::enable_if<!noexcept(tryFunc(*first)) && 
    noexcept(catchFunc(*first)), void>::type
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
    InputIt first, InputIt last, TryUnaryFunc tryFunc, CatchUnaryFunc catchFunc) -> typename 
    std::enable_if<!noexcept(tryFunc(*first)) && 
    noexcept(catchFunc(*first)), void>::type
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
    InputIt first, InputIt last, TryUnaryFunc tryFunc, CatchUnaryFunc catchFunc) noexcept -> typename 
    std::enable_if<noexcept(tryFunc(*first)), void>::type
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


} /*END OF NAMESPACES*/




#endif // _NICKSV_UTILS