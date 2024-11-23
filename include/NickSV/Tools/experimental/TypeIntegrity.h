/**
 * @file
 * @brief 
 * EXPERIMENTAL HEADER
 * 
 * NOT INTENDENT TO INCLUDE
 * 
*/

#define INVALID_PACK_MESSAGE R"( \
            Invalid Members<..>::InheritedFrom<ITypes...> usage: \
            ITypes... should be pack of classes including void  \
            (not fundamental types). \
            )"


namespace NickSV 
{
namespace Tools
{
namespace Experimental
{

template<class ValueType>
constexpr ValueType _Max_(const ValueType val1, const ValueType val2)
{
    return (val1 >=  val2) ? val1  :  val2;
}

template<class... MTypes>
struct Members;

template<class T, class... MTypes>
struct Members<T, MTypes...>
{
    struct InheritedVoid
    {
        using next_member = T;
        using other_members = typename Members<MTypes...>::InheritedVoid;

        //static constexpr size_t member_size = sizeof(T) + other_members::member_size;
        //static constexpr size_t member_max_alignment = _Max_(alignof(T), other_members::member_max_alignment);
        //static constexpr size_t base_size = 0;
        //static constexpr size_t base_max_alignment = 0;
    };

    template<class... ITypes>
    struct InheritedFrom;

    template<class IT, class... ITypes>
    struct InheritedFrom<IT, ITypes...>
    {
        static_assert(std::is_class<IT>::value, INVALID_PACK_MESSAGE);
        using next_member = T;
        using next_base = IT;
        using other_bases = Members<T, MTypes...>::InheritedFrom<ITypes...>;
        using other_members = typename Members<MTypes...>::template InheritedFrom<IT, ITypes...>;

        //static constexpr size_t member_size = sizeof(T) + other_members::member_size;
        //static constexpr size_t member_max_alignment  = _Max_(alignof(T), other_members::member_max_alignment);
        //static constexpr size_t base_size = sizeof(IT) + other_bases::base_size;
        //static constexpr size_t base_max_alignment = _Max_(alignof(IT),  other_bases::base_max_alignment);
    };

    template<class IT>
    struct InheritedFrom<IT>
    {
        static_assert(std::is_class<IT>::value, INVALID_PACK_MESSAGE);
        using next_member = T;
        using next_base = IT;
        using other_bases = Members<T, MTypes...>;
        using other_members = typename Members<MTypes...>::template InheritedFrom<IT>;

        //static constexpr size_t member_size = sizeof(T) + other_members::member_size;
        //static constexpr size_t member_max_alignment = _Max_(alignof(T),  other_members::member_max_alignment);
        //static constexpr size_t base_size = sizeof(IT);
        //static constexpr size_t base_max_alignment = alignof(IT);
    };

    using next_member = T;
    using other_members = Members<MTypes...>;
    //static constexpr size_t member_size = sizeof(T) + other_members::member_size;
    //static constexpr size_t member_max_alignment = _Max_(alignof(T),  other_members::member_max_alignment);
};


template<>
struct Members<>
{
    struct InheritedVoid
    {
        //static constexpr size_t member_size = 0;
        //static constexpr size_t member_max_alignment = 0;
        //static constexpr size_t base_size = 0;
        //static constexpr size_t base_max_alignment = 0;
    };

    template<class... ITypes>
    struct InheritedFrom;

    template<class IT, class... ITypes>
    struct InheritedFrom<IT, ITypes...>
    {
        static_assert(std::is_class<IT>::value, INVALID_PACK_MESSAGE);
        using next_base = IT;
        using other_bases = Members<>::InheritedFrom<ITypes...>;
        
        //static constexpr size_t member_size = 0;
        //static constexpr size_t member_max_alignment  = 0;
        //static constexpr size_t base_size = sizeof(IT);
        //static constexpr size_t base_max_alignment = alignof(IT);
    };

    template<class IT>
    struct InheritedFrom<IT>
    {
        static_assert(std::is_class<IT>::value, INVALID_PACK_MESSAGE);
        using next_base = IT;
        using other_bases = Members<>;
        
        //static constexpr size_t member_size = 0;
        //static constexpr size_t member_max_alignment  = 0;
        //static constexpr size_t base_size =alignof(IT);
        //static constexpr size_t base_max_alignment = alignof(IT);
    };
    //static constexpr size_t member_size = 0;
    //static constexpr size_t member_max_alignment = 0;
};

template<class T>
struct Members<T>
{

    struct InheritedVoid
    {
        using next_member = T;
        using other_members = typename Members<>::InheritedVoid;

        //static constexpr size_t member_size = sizeof(T);
        //static constexpr size_t member_max_alignment = alignof(T);
        //static constexpr size_t base_size = 0;
        //static constexpr size_t base_max_alignment = 0;
    };

    template<class... ITypes>
    struct InheritedFrom;

    template<class IT, class... ITypes>
    struct InheritedFrom<IT, ITypes...>
    {
        static_assert(std::is_class<IT>::value, INVALID_PACK_MESSAGE);
        using next_member = T;
        using next_base = IT;
        using other_bases = Members<T>::InheritedFrom<ITypes...>;
        using other_members = Members<>::InheritedFrom<IT, ITypes...>;

        //static constexpr size_t member_size = sizeof(T);
        //static constexpr size_t member_max_alignment  = alignof(T);
        //static constexpr size_t base_size = sizeof(IT) + other_bases::base_size;
        //static constexpr size_t base_max_alignment = _Max_(alignof(IT),  other_bases::base_max_alignment);
    };

    template<class IT>
    struct InheritedFrom<IT>
    {
        static_assert(std::is_class<IT>::value, INVALID_PACK_MESSAGE);
        using next_member = T;
        using next_base = IT;
        using other_bases = Members<T>;
        using other_members = Members<>::InheritedFrom<IT>;
        
        //static constexpr size_t member_size = sizeof(T);
        //static constexpr size_t member_max_alignment  = alignof(T);
        //static constexpr size_t base_size = alignof(IT);
        //static constexpr size_t base_max_alignment = alignof(IT);
    };
    using next_member = T;
    using other_members = Members<>;
    //static constexpr size_t member_size = sizeof(T);
    //static constexpr size_t member_max_alignment  = alignof(T);

};


template<class... ITypes>
using Bases = Members<>::InheritedFrom<ITypes...>;



namespace details
{
    template<typename Inheritance>
    struct has_members_impl {
    private:
        template<typename U>
        static constexpr void test(typename U::next_member* = 0);

        template<typename U>
        static constexpr int test(...);
    public:
        has_members_impl() = default;
        static constexpr bool value = std::is_void<decltype(test<Inheritance>(nullptr))>::value;
    };

    template<typename Inheritance>
    struct has_bases_impl {
    private:
        template<typename U>
        static constexpr void test(typename U::next_base* = 0);

        template<typename U>
        static constexpr int test(...);
    public:
        has_bases_impl() = default;
        static constexpr bool value = std::is_void<decltype(test<Inheritance>(nullptr))>::value;
    };

    template<typename Inheritance>
    struct has_2_bases_impl {
    private:
        template<typename U>
        static constexpr void test(typename U::other_bases::next_base* = 0);

        template<typename U>
        static constexpr int test(...);
    public:
        has_2_bases_impl() = default;
        //cppcheck-suppress unusedStructMember
        static constexpr bool value = std::is_void<decltype(test<Inheritance>(nullptr))>::value;
    };
}

template<typename Inheritance>
struct has_members : std::integral_constant<bool, details::has_members_impl<Inheritance>::value> {};

template<typename Inheritance>
struct has_bases : std::integral_constant<bool, details::has_bases_impl<Inheritance>::value> {};


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

template<class Inheritance>
struct DeleteInheritance;

template<class Inheritance>
using DeleteInheritance_t = typename DeleteInheritance<Inheritance>::type;

template<class Inheritance>
struct DeleteInheritance
{
    using type = DeleteInheritance_t<typename Inheritance::other_bases>;
};

template<class... MemberTypes>
struct DeleteInheritance<Members<MemberTypes...>>
{
    using type = Members<MemberTypes...>;
};

template<class... InheritTypes>
struct DeleteInheritance<Bases<InheritTypes...>>
{
    using type = Members<>;
};


template<class Inheritance>
struct DeleteMembers;

template<class Inheritance>
using DeleteMembers_t = typename DeleteMembers<Inheritance>::type;

template<class Inheritance>
struct DeleteMembers
{
    using type = DeleteMembers_t<typename Inheritance::other_members>;
};

template<class... InheritTypes>
struct DeleteMembers<Bases<InheritTypes...>>
{
    using type = Bases<InheritTypes...>;
};

template<class... MemberTypes>
struct DeleteMembers<Members<MemberTypes...>>
{
    using type = Bases<>;
};

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


template<class Inheritance>
struct RecursivelyInherit;

template<class Inheritance>
using RecursivelyInherit_t = typename RecursivelyInherit<Inheritance>::type;

template<class Inheritance>
struct RecursivelyInherit :
    RecursivelyInherit<DeleteMembers_t<Inheritance>> {};

template<class... ITypes>
struct RecursivelyInherit<Bases<ITypes...>>
{
    struct type : ITypes ... {};
};

template<class IT>
struct RecursivelyInherit<Bases<IT>>
{
    using type = IT;
};

template<class... MemberTypes>
struct RecursivelyInherit<Members<MemberTypes...>>
{
    struct type {};
};


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

template<class, class>
struct Concatinate;

template<class M1, class M2>
using Concatinate_t = typename Concatinate<M1, M2>::type;

template<class MT, class... MemberTypes>
struct Concatinate<Members<MemberTypes...>, MT>
{
    using type = Members<MemberTypes..., MT>;
};

template<class MT, class... MemberTypes>
struct Concatinate<MT, Members<MemberTypes...>>
{
    using type = Members<MT, MemberTypes...>;
};

template<class MT>
struct Concatinate<Members<>, MT>
{
    using type = Members<MT>;
};

template<class MT>
struct Concatinate<MT, Members<>>
{
    using type = Members<MT>;
};


template<class>
struct Reverse;

template<class MembersT>
using Reverse_t = typename Reverse<MembersT>::type;

template<class MT, class... MemberTypes>
struct Reverse<Members<MT, MemberTypes...>>
{
    using type = Concatinate_t<Reverse_t<Members<MemberTypes...>>, MT>;
};

template<class MT>
struct Reverse<Members<MT>>
{
    using type = Members<MT>;
};

template<>
struct Reverse<Members<>>
{
    using type = Members<>;
};



template<class T, bool is_class = std::is_class<T>::value>
struct Wrap
{
    struct type { T member; };
};

template<class T>
struct Wrap<T, true>
{
    using type = T;
};

template<class T>
using Wrap_t = typename Wrap<T>::type;


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

template<class>
struct BuildTypeByInheritance;

template<class... MemberTypes>
struct BuildTypeByInheritance<Members<MemberTypes...>>
{
    struct type : Wrap<MemberTypes>::type ... {};
};

template<class MembersT>
using BuildTypeByInheritance_t = typename BuildTypeByInheritance<MembersT>::type;

template<class, bool reversed = false>
struct BuildTypeByMembers;


template<class MembersT, bool reversed = false>
using BuildTypeByMembers_t = typename BuildTypeByMembers<MembersT, reversed>::type;

template<class MT, class... MemberTypes>
struct BuildTypeByMembers<Members<MT, MemberTypes...>, false> 
{
    struct type : Wrap_t<MT>, BuildTypeByMembers_t<Members<MemberTypes...>, false>
    {
        //MT member;
        //BuildTypeByMembers_t<Members<MemberTypes...>, false> others;
    };
};


template<class MT, class... MemberTypes>
struct BuildTypeByMembers<Members<MT, MemberTypes...>, true> 
{
    struct type : BuildTypeByMembers_t<Members<MemberTypes...>, true>, Wrap_t<MT>
    {
        //BuildTypeByMembers_t<Members<MemberTypes...>, true> others;
        //MT member;
    };
};

template<class MT>
struct BuildTypeByMembers<Members<MT>, true> 
{
    struct type { MT member; };
    //using type = MT;
};

template<class MT>
struct BuildTypeByMembers<Members<MT>, false> 
{
    struct type { MT member; };
    //using type = MT;
};

template<>
struct BuildTypeByMembers<Members<>, true> 
{
    struct type {};
};

template<>
struct BuildTypeByMembers<Members<>, false> 
{
    struct type {};
};


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

template<class, class = void>
struct BuildType;

template<class Inheritance>
struct BuildType<Inheritance>
{
private:
    using PureMembers = DeleteInheritance_t<Inheritance>;
    using builded_members = 
    #ifdef _MSC_VER
        BuildTypeByInheritance_t<PureMembers>;
    #else
        BuildTypeByMembers_t<Reverse_t<PureMembers>, true>;
    #endif
    struct _type : RecursivelyInherit_t<Inheritance>, builded_members {};
public:
    using type = typename std::conditional<
            has_members<Inheritance>::value, _type,
            RecursivelyInherit_t<Inheritance>
        >::type;
};



template<class... MemberTypes>
struct BuildType<Members<MemberTypes...>> : 
#ifdef _MSC_VER
BuildTypeByInheritance<Members<MemberTypes...>>
#else
BuildTypeByMembers<Reverse_t<Members<MemberTypes...>>, true>
#endif
{};

template<>
struct BuildType<Members<>>
{
    struct type {};
};


template<class MembersT>
struct BuildType<MembersT, Bases<>> : 
    BuildType<MembersT> {};


template<class MembersT, class... ITypes>
struct BuildType<MembersT, Bases<ITypes...>> : 
    BuildType<typename MembersT::template InheritedFrom<ITypes...>> {};



template<class M, class B = void>
struct BuildTypeVirtual
{
    struct type : BuildType<M, B>::type
    {
        virtual ~type() = default;
    };
};

template<class M, class B = void>
using BuildType_t = typename BuildType<M, B>::type;

template<class M, class B = void>
using BuildTypeVirtual_t = typename BuildTypeVirtual<M, B>::type;



template<typename T, typename Inheritance>
struct is_child_of : std::integral_constant<bool,
std::is_base_of<typename Inheritance::next_base, T>::value &&  
!has_bases<typename Inheritance::other_bases>::value || 
is_child_of<T, typename Inheritance::other_bases>::value> {};


template<typename T>
struct is_child_of<T, Bases<>> : std::false_type {};


template<typename T, typename... MemberTypes>
struct is_child_of<T, Members<MemberTypes...>> : std::false_type {};


//template<size_t size>
//struct Debug;

/**
 * @brief Compile time type integrity assert.
 * 
 * Same as @ref type_integrity_assert() 
 * but should be used with Type,that has its own 
 * virtual methods.
*/

template<typename Type1, typename Type2, size_t depth, size_t additionalSize = 0>
struct CompareTypes
{
    using next = CompareTypes<Type1, Type2, depth - 1, additionalSize>;
    struct type1 : next::type1 { char charik; };
    struct type2 : next::type2 { char charik; };

    static constexpr bool value = (sizeof(type1) == sizeof(type2)) && next::value;
};

template<typename Type1, typename Type2, size_t additionalSize>
struct CompareTypes<Type1, Type2, 1, additionalSize>
{
    struct type1 : Type1 { char charik; };
    struct type2 : Type2
    {
        char additional[additionalSize];
        char charik;
    };
    //cppcheck-suppress unusedStructMember
    static constexpr bool value = (sizeof(type1) == sizeof(type2)) && CompareTypes<Type1, Type2, 0, additionalSize>::value;
};

template<typename Type1, typename Type2>
struct CompareTypes<Type1, Type2, 1, 0>
{
    struct type1 : Type1 { char charik; };
    struct type2 : Type2 { char charik; };

    //cppcheck-suppress unusedStructMember
    static constexpr bool value = (sizeof(type1) == sizeof(type2)) && CompareTypes<Type1, Type2, 0, 0>::value;
};

template<typename Type1, typename Type2, size_t additionalSize>
struct CompareTypes<Type1, Type2, 0, additionalSize>
{
    static constexpr bool value = sizeof(Type1) == sizeof(Type2);
};



} //END OF EXPERIMENTAL







//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////




template<typename Type, typename MembersT, typename BasesT, size_t additionalSize = 0>
constexpr inline bool type_integrity_assert()
{
#ifndef NICKSV_TYPE_INTEGRITY_NO_ASSERTION

    using builded_type = Experimental::BuildType_t<MembersT, BasesT>;

    static_assert(!Experimental::has_bases<BasesT>::value || Experimental::is_child_of<Type, BasesT>::value, 
        "Type integrity assertion failed: Type is not inherited from bases inside BasesT. Define NICKSV_TYPE_INTEGRITY_NO_ASSERTION to disable assertion.");

    static_assert(Experimental::CompareTypes<Type, builded_type, Experimental::_Max_(additionalSize, alignof(Type)), additionalSize>::value,
        "Type integrity assertion failed: Type has a size different from its TypeIdentity (MembersT and BasesT). Define NICKSV_TYPE_INTEGRITY_NO_ASSERTION to disable assertion.");
#endif
    return true;
}


template<typename Type, typename MembersT, typename BasesT, size_t additionalSize = 0>
constexpr inline bool type_integrity_assert_virtual()
{
#ifndef NICKSV_TYPE_INTEGRITY_NO_ASSERTION

    using builded_type = Experimental::BuildTypeVirtual_t<MembersT, BasesT>;

    static_assert(std::is_polymorphic<Type>::value, 
        "Type integrity assertion failed: Type is not polymorphic, so use type_integrity_assert(). Define NICKSV_TYPE_INTEGRITY_NO_ASSERTION to disable assertion.");

    static_assert(!Experimental::has_bases<BasesT>::value || Experimental::is_child_of<Type, BasesT>::value, 
        "Type integrity assertion failed: Type is not inherited from bases inside BasesT. Define NICKSV_TYPE_INTEGRITY_NO_ASSERTION to disable assertion.");

    static_assert(Experimental::CompareTypes<Type, builded_type, Experimental::_Max_(additionalSize, alignof(Type)), additionalSize>::value,
        "Type integrity assertion failed: Type has a size different from its TypeIdentity (MembersT and BasesT). Define NICKSV_TYPE_INTEGRITY_NO_ASSERTION to disable assertion.");
#endif
    return true;
}

/**
 * @brief Compile time type integrity assert.
 * 
 * Usual should be put inside functions that are
 * depend on integrity of Type
 * (e.g. number or sizes of class members).
 * And if someone changed members of Type,
 * this assertion failed (e.g functions that are serializing Type)
 * 
 * @tparam Type type to check
 * @tparam TypeIdentity special entity, that holds member and base types, can be:
 * 1) Members< Member types ... > ;
 * 2) Members< Member types ... >::InheritedFrom< Base types ... > ;
 * 3) Bases< Base types ... >.
 * @tparam additionalSize optional value, specify it (empirically)
 * if assertion fails but shouldn't or if your class Type has custon alignment
 * 
 * For example:
 * @code{.cpp} 
 * 
 * class NoBases {
 *      std::string s;
 *      double d;
 * };
 * 
 * class NoMembers : Base {};
 * 
 * class MembersAndBase : Base {};
 * {
 *      int i;
 *      char* ptr;
 * };
 * 
 * void example_function()
 * { 
 *      type_integrity_assert<NoBases, Members<std::string, double>>();
 * 
 *      type_integrity_assert<NoMembers, Bases<std::string, double>>();
 *      // same as
 *      type_integrity_assert<NoMembers, Members<>::InheritedFrom<std::string, double>>();
 * 
 *      type_integrity_assert<MembersAndBase, Members<int, char*>InheritedFrom<std::string, double>>();
 * 
 * }
 * 
 * @endcode
 * 
 * 
 * @warning
 * 1) Only for class Type that doesn't have its own virtual methods,
 * otherwise use @ref type_integrity_assert_virtual().
 * 
 * @warning
 * 2) This function is not strongly compares class Type and TypeIdentity,
 * so in some cases it may not fail if members are different but
 * sizes is the same.
 * 
 * @warning
 * 3) For class Type that has custom member aligment behavior is undefined,
 * but you can specify additionalSize value to fill padding.
 *    
*/
template<typename Type, typename TypeIdentity, size_t additionalSize = 0>
constexpr inline bool type_integrity_assert()
{
    return type_integrity_assert<Type, 
        typename Experimental::DeleteInheritance<TypeIdentity>::type, 
        typename Experimental::DeleteMembers<TypeIdentity>::type,
        additionalSize>();
}


/**
 * @brief Compile time type integrity assert.
 * 
 * Same as @ref type_integrity_assert() 
 * but should be used with Type,that has its own 
 * virtual methods.
*/
template<typename Type, typename TypeIdentity, size_t additionalSize = 0>
constexpr inline bool type_integrity_assert_virtual()
{
    return type_integrity_assert_virtual<Type, 
        typename Experimental::DeleteInheritance<TypeIdentity>::type, 
        typename Experimental::DeleteMembers<TypeIdentity>::type,
        additionalSize>();
}


/**
 * @brief Compile time type integrity assert.
 * 
 * Usual should be put inside functions that are
 * depend on integrity of Type
 * (e.g. number or sizes of class members).
 * And if someone changed members of Type,
 * this assertion failed. For example,
 * functions that are serializing Type.
 * This
 * 
 * @tparam Type type to check
 * @tparam Size implied size of Type (DO NOT USE sizeof(Type) here) 
 * 
 * For example:
 * @code{.cpp} 
 *  
 * class MyType1
 * {
 *  double d1;
 *  double d2;
 * }
 * 
 * class MyType2
 * {
 *  double d1;
 *  double d2;
 *  char d3;
 * }
 * 
 * void example_function()
 * { 
 *      type_integrity_assert<MyType, 16>(); // OK
 * 
 *      type_integrity_assert<MyType, 15>(); // FAILED
 * 
 *      type_integrity_assert<MyType2, 24, double>>(); // OK, 7 bytes padding
 * }
 * 
 * @endcode
 * 
 * 
 * @warning
 * Different compilers may have different sizes for your Type.
 * See @ref COMPILER_AWARE_VALUE macro to specify different size for 
 * each compiler. 
 *    
*/
template<typename Type, size_t Size>
constexpr inline bool type_integrity_assert()
{
#ifndef NICKSV_TYPE_INTEGRITY_NO_ASSERTION
    static_assert(sizeof(Type) == Size,
        "Type integrity assertion failed: Type has a different size than the specified one. Define NICKSV_TYPE_INTEGRITY_NO_ASSERTION to disable assertion.");
#endif
    return true;
}



/**
 * @brief Compile time type integrity assert.
 * 
 * Usual should be put inside functions that are
 * depend on integrity of Type
 * (e.g. number or sizes of class members).
 * And if someone changed members of Type,
 * this assertion failed. For example,
 * functions that are serializing Type.
 * This
 * 
 * @tparam Type type to check
 * @tparam Size implied size of Type (DO NOT USE sizeof(Type) here) 
 * 
 * For example:
 * @code{.cpp} 
 *  
 * class MyType1
 * {
 *  double d1;
 *  double d2;
 * }
 * 
 * class MyType2
 * {
 *  double d1;
 *  double d2;
 *  char d3;
 * }
 * 
 * void example_function()
 * { 
 *      type_integrity_assert<MyType, 16>(); // OK
 * 
 *      type_integrity_assert<MyType, 15>(); // FAILED
 * 
 *      type_integrity_assert<MyType2, 24, double>>(); // OK, 7 bytes padding
 * }
 * 
 * @endcode
 * 
 * 
 * @warning
 * Different compilers may have different sizes for your Type.
 * See @ref COMPILER_AWARE_VALUE macro to specify different size for 
 * each compiler. 
 *    
*/
template<typename Type, size_t Size>
constexpr inline bool type_integrity_assert_virtual()
{
    return type_integrity_assert<Type, Size>();
}






}} /*END OF NAMESPACES*/
