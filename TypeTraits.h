/**
 * @file
 * @brief Useful type traits metafunctions.
 * 
 * Some new traits and traits 
 * pulled from higher std version
*/
#ifndef _NICKSV_TYPE_TRAITS
#define _NICKSV_TYPE_TRAITS
#pragma once



#include <string>
#include <type_traits>

#include "Definitions.h"




namespace NickSV::Tools {



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
struct is_char : details::is_char_helper<typename std::remove_cv<CharT>::type> {};

#ifdef __cpp_variable_templates
template< class T >
constexpr static bool is_char_v = is_char<T>::value;
#endif



template<typename T, typename = void>
struct is_equality_comparable : std::false_type
{ };

template<typename T>
struct is_equality_comparable<T,
    typename std::enable_if<
        true, 
        decltype(std::declval<T&>() == std::declval<T&>(), (void)0)
        >::type
    > : std::true_type
{
};


#ifdef __cpp_variable_templates
template<class T>
constexpr static bool is_equality_comparable_v = is_equality_comparable<T>::value;
#endif


template<typename T>
struct SizeOfType {
    static constexpr size_t value = sizeof(T);
};

template<typename... Types>
struct TotalSizeOfTypes {
    //cppcheck-suppress unusedStructMember
    static constexpr size_t value = (SizeOfType<Types>::value + ...);
};

template<>
struct TotalSizeOfTypes<> {
    //cppcheck-suppress unusedStructMember
    static constexpr size_t value = 0;
};


/**
 * @brief Compile time type integrity assert.
 * 
 * Usual should be put inside functions that are
 * depend on integrity of polymorphic Type 
 * (e.g. number or sizes of class members).
 * And if someone changed members of Type,
 * this assertion failed. For example,
 * functions that are serializing Type.
 * 
 * @note
 * #define NICKSV_TYPE_INTEGRITY_NO_ASSERTION 
 * to disable assertion.
 * 
 * @tparam Type type we want to check integrity
 * @tparam MTypes types pack of Type's members
 * @tparam additionalSize main reason of this template param
 *         is to specify how many vtable pointers Type has,
 *         because sizeof(Type) also counts them
 *         and we cant get their number and
 *         unfortunatelly we have no traits like
 *         std::sizeof_object_members<Type>::value,
 *         so for each virtual table pointer of Type
 *         you need to add 4 or 8 (__SIZEOF_POINTER__)
 *         to additionalSize.
 * 
 * @warning 
 * For polymorphic Type use this function 
 * and set additionalSize param.
 * For non-polymorphic Type use other overload 
 * just by ignoring additionalSize  template param.
 * 
*/
template<typename Type, size_t additionalSize, typename... MTypes>
constexpr inline void type_integrity_assert()
{
#ifndef NICKSV_TYPE_INTEGRITY_NO_ASSERTION
    if(std::is_polymorphic<Type>::value){
        static_assert(additionalSize,  
            R"(
                Your Type is polymorphic, so you need to increase additionalSize 
                by 8 or 4 (__SIZEOF_POINTER__) for each virtual table pointer Type has.
                Define NICKSV_TYPE_INTEGRITY_NO_ASSERTION to dismiss this assertion
            )");}

    static_assert((TotalSizeOfTypes<MTypes...>::value + additionalSize == sizeof(Type)),
        R"(
            Sensitive code part for Type object members. 
            Seems like you edited object members of Type and should edit code here as well. 
            Define NICKSV_TYPE_INTEGRITY_NO_ASSERTION to dismiss this assertion"
        )");
#endif
}


/**
 * @brief Compile time type integrity assert.
 * 
 * Usual should be put inside functions that are
 * depend on integrity of non-polymorphic Type
 * (e.g. number or sizes of class members).
 * And if someone changed members of Type,
 * this assertion failed. For example,
 * functions that are serializing Type.
 * 
 * @note
 * #define NICKSV_TYPE_INTEGRITY_NO_ASSERTION 
 * to disable assertion.
 * 
 * @tparam Type type we want to check integrity
 * @tparam MTypes types pack of Type's members
 * 
 * @warning 
 * For non-polymorphic Type use this function.
 * For polymorphic Type use other overload
 * ( type_integrity_assert<Type, size, MTypes...>() )
 * with additionalSize template param.
*/
template<typename Type, typename... MTypes>
constexpr inline void type_integrity_assert()
{
#ifndef NICKSV_TYPE_INTEGRITY_NO_ASSERTION
    static_assert(std::is_polymorphic<Type>::value,
        R"(
            Your Type is polymorphic, so you need to use 
            other overload ( type_integrity_assert<Type, size, MTypes...>() ) 
            of this function with additionalSize template param. 
            Define NICKSV_TYPE_INTEGRITY_NO_ASSERTION to dismiss this assertion
        )");
    static_assert((TotalSizeOfTypes<MTypes...>::value  == sizeof(Type)),
        R"(
            Sensitive code part for Type fields. 
            Seems like you edited fields of Type and should edit code here as well. 
            Define NICKSV_TYPE_INTEGRITY_NO_ASSERTION to dismiss this assertion
        )");
#endif
}



}






#ifndef __cpp_variable_templates // I hope traits helper types ( std::<trait>_t ) appeared with this feature =)
namespace std
{
    template<bool cond, typename Type = void>
    using enable_if_t = typename std::enable_if<cond, Type>::type;
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



#endif //_NICKSV_TYPE_TRAITS