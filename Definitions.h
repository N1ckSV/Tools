/**
 * @file
 * @brief Definitions of useful macros
*/
#ifndef _NICKSV_DEFINITIONS
#define _NICKSV_DEFINITIONS
#pragma once

#define CXX98_VERSION 199711L
#define CXX11_VERSION 201103L
#define CXX14_VERSION 201402L 
#define CXX17_VERSION 201703L
#define CXX20_VERSION 202002L
#define CXX23_VERSION 202302L


//ASSERT STUFF (EXPECT is non-fatal one)
#if !defined(NDEBUG) || defined(_DEBUG) 
    #include <assert.h>
    //cppcheck-suppress incorrectStringBooleanError 
    #define NICKSV_ASSERT(exp, msg) assert((exp) && (msg))
    #define NICKSV_EXPECT(exp, msg) do { if (!(exp)){fprintf(stderr, "%s failed in %s:%d\n", #exp, __FILE__, __LINE__);}} while(0)
    #define NICKSV_DEBUG(...) (__VA_ARGS__)
    #define _NICKSV_DEBUG
#else
    #define NICKSV_ASSERT(exp, msg) (void(0))
    #define NICKSV_EXPECT(exp, msg) (void(0))
    #define NICKSV_DEBUG(...)       (void(0))
#endif


#if (__cplusplus >= CXX14_VERSION)
#define CONSTEXPR_SINCE_CPP14 constexpr
#else
#define CONSTEXPR_SINCE_CPP14
#endif

#ifdef __cpp_inline_variables
#define INLINE_SINCE_CPP17 inline
#else
#define INLINE_SINCE_CPP17
#endif


#ifdef __cpp_if_consteval
#define STATIC_IF consteval
#elif defined(__cpp_if_constexpr)
#define STATIC_IF constexpr
#else
#define STATIC_IF 
#endif



#if defined(__GNUC__)
#define COMPILER_AWARE_VALUE(GNU_VAL, MSVC_VAL, OTHER_VAL) GNU_VAL
#elif defined(_MSC_VER)
#define COMPILER_AWARE_VALUE(GNU_VAL, MSVC_VAL, OTHER_VAL) MSVC_VAL
#else
#define COMPILER_AWARE_VALUE(GNU_VAL, MSVC_VAL, OTHER_VAL) OTHER_VAL
#endif


#define _NickSV_TEXT_(text, Type)                                                              \
             std::is_same<Type, char32_t>::value ? static_cast<const void*> (U"" text)    :    \
            (std::is_same<Type, char16_t>::value ? static_cast<const void*> (u"" text)    :    \
            (std::is_same<Type,  wchar_t>::value ? static_cast<const void*> (L"" text)    :    \
                                                   static_cast<const void*> (    text)))
                                            
#ifdef __cpp_char8_t                                                                       
    #define NickSV_TEXT(text, Type) static_cast<const std::enable_if_t<                                                     \
                             NickSV::Tools::is_char_v<Type>, std::remove_cv_t<Type>>*>(                                     \
                             (std::is_same_v<std::remove_cv_t<Type>,  char8_t> ? static_cast<const void*> (u8"" text)   :   \
                              _NickSV_TEXT_(text, std::remove_cv_t<Type>)))
#else
    #define NickSV_TEXT(text, Type) static_cast<const std::enable_if_t<                     \
                             NickSV::Tools::is_char<Type>::value,                           \
                             std::remove_cv_t<Type>>*>(                                     \
                             _NickSV_TEXT_(text, std::remove_cv_t<Type>)) 
#endif


#define NOTHING

#define DECLARE_COPY(ClassName, prefix, postfix)                              \
        ClassName (const ClassName& lvalRef) postfix;                         \
        prefix ClassName& operator=(const ClassName& lvalRef) postfix         \

#define DECLARE_MOVE(ClassName, prefix, postfix)                              \
        ClassName (ClassName && rvalRef) noexcept postfix;                    \
        prefix ClassName& operator=(ClassName && rvalRef) noexcept postfix    \

#define DECLARE_COPY_DELETE(ClassName)                                        \
        DECLARE_COPY(ClassName, NOTHING, = delete)                            \

#define DECLARE_MOVE_DELETE(ClassName)                                        \
        DECLARE_MOVE(ClassName, NOTHING, = delete)                            \

#define DECLARE_COPY_DEFAULT(ClassName, prefix)                               \
        DECLARE_COPY(ClassName, prefix, = default)                            \

#define DECLARE_MOVE_DEFAULT(ClassName, prefix)                               \
        DECLARE_MOVE(ClassName, prefix, = default)                            \

#define DECLARE_COPY_MOVE(ClassName, prefix, postfix)                         \
        DECLARE_COPY(ClassName, prefix, postfix);                             \
        DECLARE_MOVE(ClassName, prefix, postfix)                              \

#define DECLARE_RULE_OF_3(ClassName, prefix, postfix)                         \
        DECLARE_COPY(ClassName, prefix, postfix);                             \
        prefix ~ClassName() postfix                                           \

#define DECLARE_RULE_OF_5(ClassName, prefix, postfix)                         \
        DECLARE_COPY_MOVE(ClassName, prefix, postfix);                        \
        prefix ~ClassName() postfix                                           \
         
#define DECLARE_RULE_OF_0_POLYMORPHIC(ClassName, prefix)                      \
        DECLARE_COPY_MOVE(ClassName, prefix, = default);                      \
        virtual ~ClassName() = default                                        \

#define DECLARE_RULE_OF_0_NON_POLYMORPHIC(ClassName) \
        DECLARE_RULE_OF_5(ClassName, NOTHING, = default) 

#define DECLARE_RULE_OF_3_DEFAULT(ClassName, prefix) \
        DECLARE_RULE_OF_3(ClassName, prefix, = default) 

#define DECLARE_RULE_OF_5_DEFAULT(ClassName, prefix) \
        DECLARE_RULE_OF_5(ClassName, prefix, = default) 

#define DECLARE_RULE_OF_3_VIRTUAL(ClassName, postfix) \
        DECLARE_RULE_OF_3(ClassName, virtual, postfix) 

#define DECLARE_RULE_OF_5_VIRTUAL(ClassName, postfix) \
        DECLARE_RULE_OF_5(ClassName, virtual, postfix) 

#define DECLARE_RULE_OF_3_DELETE(ClassName) \
        DECLARE_COPY(ClassName, NOTHING, = delete)  //Destructor ignored

#define DECLARE_RULE_OF_5_DELETE(ClassName) \
        DECLARE_COPY_MOVE(ClassName, NOTHING, = delete)  //Destructor ignored

#define DECLARE_COPY_MOVE_DEFAULT(ClassName, prefix) \
        DECLARE_COPY_MOVE(ClassName, prefix, = default) 

#define DECLARE_COPY_VIRTUAL(ClassName, postfix) \
        DECLARE_COPY(ClassName, virtual, postfix) 

#define DECLARE_COPY_MOVE_VIRTUAL(ClassName, postfix) \
        DECLARE_COPY_MOVE(ClassName, virtual, postfix) 

#define DECLARE_RULE_OF_3_VIRTUAL_DEFAULT(ClassName) \
        DECLARE_RULE_OF_3_DEFAULT(ClassName, virtual) 

#define DECLARE_RULE_OF_5_VIRTUAL_DEFAULT(ClassName) \
        DECLARE_RULE_OF_5_DEFAULT(ClassName, virtual) 

#define DECLARE_MOVE_ONLY(ClassName) \
        DECLARE_COPY_DELETE(ClassName); \
        DECLARE_MOVE_DEFAULT(ClassName, NOTHING)




#ifndef NICKSV_TYPE_INTEGRITY_NO_ASSERTION
/**
 * @def NICKSV_TYPE_INTEGRITY_NO_ASSERTION
 * @brief Special define disabling @ref type_integrity_assert()
*/
#define NICKSV_TYPE_INTEGRITY_NO_ASSERTION
#undef  NICKSV_TYPE_INTEGRITY_NO_ASSERTION
#endif


#define while_limit(cond, maxIters) for (uint64_t iters = 0; (iters < maxIters) && (cond); ++iters)



// namespaces below are for Doxygen 

///@brief Global namespace of Nikita "N1ckSV" Kurchev [<a href="https://github.com/N1ckSV">Github</a>]
namespace NickSV {

/// @brief Namespace of N1ckSV's tool library [<a href="https://github.com/N1ckSV/Tools">Github</a>]
namespace Tools {}}





#endif // _NICKSV_DEFINITIONS