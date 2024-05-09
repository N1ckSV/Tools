
#ifndef _NICKSV_DEFINES
#define _NICKSV_DEFINES
#pragma once




//ASSERT STUFF (EXPECT is non-fatal one)
#if defined(NDEBUG)
    #include <assert.h>
    //cppcheck-suppress incorrectStringBooleanError 
    #define NICKSV_ASSERT(exp, msg) assert((exp) && (msg))
    #define NICKSV_EXPECT(exp, msg) do { if (!(exp)){fprintf(stderr, "%s failed in %s:%d\n", #exp, __FILE__, __LINE__);}} while(0)
#else
    #define NICKSV_ASSERT(exp, msg) (void(0))
    #define NICKSV_EXPECT(exp, msg) (void(0))
#endif


#if (__cplusplus >= 201402L)
#define CONSTEXPR_SINCE_CPP14 constexpr
#else
#define CONSTEXPR_SINCE_CPP14
#endif


#define NOTHING

#define DECLARE_COPY(ClassName, prefix, postfix)                      \
        ClassName (const ClassName& lvalRef) postfix;                 \
        prefix ClassName& operator=(const ClassName& lvalRef) postfix;\

#define DECLARE_MOVE(ClassName, prefix, postfix)                     \
        ClassName (ClassName && rvalRef) postfix;                    \
        prefix ClassName& operator=(ClassName && rvalRef) postfix;   \

#define DECLARE_COPY_DELETE(ClassName)                               \
        DECLARE_COPY(ClassName, NOTHING, = delete)                   \

#define DECLARE_MOVE_DELETE(ClassName)                               \
        DECLARE_MOVE(ClassName, NOTHING, = delete)                   \

#define DECLARE_COPY_DEFAULT(ClassName, prefix)                      \
        DECLARE_COPY(ClassName, prefix, = default)                   \

#define DECLARE_MOVE_DEFAULT(ClassName, prefix)                      \
        DECLARE_MOVE(ClassName, prefix, = default)                   \

#define DECLARE_COPY_MOVE(ClassName, prefix, postfix)                \
        DECLARE_COPY(ClassName, prefix, postfix)                     \
        DECLARE_MOVE(ClassName, prefix, postfix)                     \

#define DECLARE_RULE_OF_3(ClassName, prefix, postfix)                \
        DECLARE_COPY(ClassName, prefix, postfix);                    \
        prefix ~ClassName() postfix;                                 \

#define DECLARE_RULE_OF_5(ClassName, prefix, postfix)                \
        DECLARE_COPY_MOVE(ClassName, prefix, postfix);               \
        prefix ~ClassName() postfix;                                 \

#define DECLARE_RULE_OF_0_POLYMORPHIC(ClassName, prefix)             \
        DECLARE_COPY_MOVE(ClassName, prefix, = default);             \
        virtual ~ClassName() = default;                              \

#define DECLARE_RULE_OF_0_NON_POLYMORPHIC(ClassName) \
        DECLARE_RULE_OF_5(ClassName, NOTHING, = default);

#define DECLARE_RULE_OF_3_DEFAULT(ClassName, prefix) \
        DECLARE_RULE_OF_3(ClassName, prefix, = default);

#define DECLARE_RULE_OF_5_DEFAULT(ClassName, prefix) \
        DECLARE_RULE_OF_5(ClassName, prefix, = default);

#define DECLARE_RULE_OF_3_VIRTUAL(ClassName, postfix) \
        DECLARE_RULE_OF_3(ClassName, virtual, postfix);

#define DECLARE_RULE_OF_5_VIRTUAL(ClassName, postfix) \
        DECLARE_RULE_OF_5(ClassName, virtual, postfix);

#define DECLARE_RULE_OF_3_DELETE(ClassName) \
        DECLARE_COPY(ClassName, NOTHING, = delete); //Destructor ignored

#define DECLARE_RULE_OF_5_DELETE(ClassName) \
        DECLARE_COPY_MOVE(ClassName, NOTHING, = delete); //Destructor ignored

#define DECLARE_COPY_MOVE_DEFAULT(ClassName, prefix) \
        DECLARE_COPY_MOVE(ClassName, prefix, = default);

#define DECLARE_COPY_VIRTUAL(ClassName, postfix) \
        DECLARE_COPY(ClassName, virtual, postfix);

#define DECLARE_COPY_MOVE_VIRTUAL(ClassName, postfix) \
        DECLARE_COPY_MOVE(ClassName, virtual, postfix);

#define DECLARE_RULE_OF_3_VIRTUAL_DEFAULT(ClassName) \
        DECLARE_RULE_OF_3_DEFAULT(ClassName, virtual);

#define DECLARE_RULE_OF_5_VIRTUAL_DEFAULT(ClassName) \
        DECLARE_RULE_OF_5_DEFAULT(ClassName, virtual);




#define LOCK_GUARD(mut) std::lock_guard<std::mutex> _generated_by_macro_lock_guard(mut)





#endif // _NICKSV_DEFINES