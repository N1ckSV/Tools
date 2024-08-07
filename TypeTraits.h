/**
 * @file
 * @brief Useful type traits metafunctions.
 * 
 * @details
 * Some new traits and traits 
 * pulled from higher std version
*/
#ifndef _NICKSV_TYPE_TRAITS__
#define _NICKSV_TYPE_TRAITS__
#pragma once


#include "Definitions.h"



#include <string>
#include <type_traits>






#ifndef __cpp_variable_templates // I hope traits helper types ( std::<trait>_t ) appeared with this feature =)
namespace std
{
    template<bool cond, typename Type = void>
    using enable_if_t = typename std::enable_if<cond, Type>::type;

    template<typename Type>
    using remove_cv_t = typename std::remove_cv<Type>::type;

    template< bool B, typename T, typename F >
    using conditional_t = typename std::conditional<B,T,F>::type;
    
    template<typename T>
    using decay_t = typename std::decay<T>::type;    
}
#endif

#ifndef __cpp_lib_remove_cvref
namespace std
{
    template<class T>
    struct remove_cvref
    {
        using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
    };
    template<class T>
    using remove_cvref_t = typename remove_cvref<T>::type;
}
#endif


namespace NickSV 
{
namespace Tools
{

namespace details
{
    template<typename CharT>
    struct is_char_helper : std::false_type {};
    template<> struct is_char_helper<char> : std::true_type {};
    template<> struct is_char_helper<wchar_t> : std::true_type {};
    template<> struct is_char_helper<char16_t> : std::true_type {};
    template<> struct is_char_helper<char32_t> : std::true_type {};
#ifdef __cpp_lib_char8_t
    template<> struct is_char_helper<char8_t> : std::true_type {};
#endif
}


/**
 * @struct is_char
 * @brief Metafunction trait to check if 
 * the type is character-like.
 * 
 * @tparam CharT type to check
 * 
 * @return
 * std::true_type if CharT is (also cv-qualified):
 * char, wchar_t, char16_t, char32_t, char8_t
*/
template<typename CharT>
struct is_char : details::is_char_helper<std::remove_cv_t<CharT>> {};



template<typename T, typename = void>
struct is_equality_comparable : std::false_type
{ };

template<typename T>
struct is_equality_comparable<T,
        std::enable_if_t<
        true, 
        decltype(std::declval<T&>() == std::declval<T&>(), (void)0)
        >
    > : std::true_type
{
};


template<typename T1, typename T2, typename... Types>
struct are_all_same : std::integral_constant<bool, std::is_same<T1, T2>::value && are_all_same<T2, Types...>::value> {};


template<typename T1, typename T2>
struct are_all_same<T1, T2> : std::is_same<T1, T2> {};



#ifdef __cpp_variable_templates
template< class T >
constexpr static bool is_char_v = is_char<T>::value;

template<class T>
constexpr static bool is_equality_comparable_v = is_equality_comparable<T>::value;
#endif



}} /*END OF NAMESPACES*/



#include "experimental/TypeIntegrity.h"




#endif //_NICKSV_TYPE_TRAITS__