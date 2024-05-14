


#define NOT_NULL_USE_EXCEPTIONS //we cant catch assert anyways 

#define NDEBUG

#define TEST_SETW_VALUE 65

#include "NickSV/Tools/Definitions.h"
#include "NickSV/Tools/Memory.h"
#include "NickSV/Tools/Testing.h"


#include <memory>
#include <sstream>

namespace NT = NickSV::Tools;



int NotNull_unspec_type_test()
{
	constexpr static int value = 1337;
	struct Base 
	{
		virtual int GetNumber() = 0;
		virtual ~Base() = default;
	};
    struct Derived : Base
	{
		int GetNumber() override
		{
			return value;
		}
	};
	Derived d;
	NT::NotNull<Derived*> notNullDerived(&d);
	NT::NotNull<Base*> notNullBase = notNullDerived;
	TEST_CHECK_STAGE((notNullDerived == notNullBase) && 
					 (notNullBase == notNullDerived));

	TEST_CHECK_STAGE((static_cast<Base*>(&d) == notNullBase) &&
					 (notNullBase == static_cast<Base*>(&d)));

	TEST_CHECK_STAGE((notNullBase->GetNumber() == value) && 
					 (value == notNullBase->GetNumber()));

	TEST_CHECK_STAGE(((*notNullBase).GetNumber() == value) && 
					 (value == (*notNullBase).GetNumber()));

	TEST_CHECK_STAGE((notNullDerived->GetNumber() == value) && 
					 (value == notNullDerived->GetNumber()));

	TEST_CHECK_STAGE(((*notNullDerived).GetNumber() == value) && 
					 (value == (*notNullDerived).GetNumber()));




	auto notNull1 = NT::MakeNotNull(&d);
	TEST_CHECK_STAGE((&d == notNull1) && 
					 (notNull1 == &d));



	std::stringstream stream1;
	std::stringstream stream2;
	stream1 << "some string + " << notNull1;
	stream2 << "some string + " << &d;
	TEST_CHECK_STAGE(stream1.str() == stream2.str());




	auto notNull2 = NT::MakeNotNull(&d);
	//cppcheck-suppress knownConditionTrueFalse
	TEST_CHECK_STAGE((notNull1 == notNull2) &&
					 (notNull2 == notNull1) &&
					 (notNull1 == &d) &&
					 (&d == notNull1) &&
					 !(notNull1 != notNull2) &&
					 !(notNull2 != notNull1) &&
					 !(notNull1 != &d) &&
					 !(&d != notNull1));



	Derived Variable;
	NT::NotNull<Derived*> notNull3(&Variable);
	TEST_CHECK_STAGE((std::hash<Derived*>{}(&Variable) 
				  == std::hash<decltype(notNull3)>{}(notNull3)) &&
				  (std::hash<Derived*>{}(&Variable)
				  == std::hash<Derived*>{}(static_cast<Derived*&>(notNull3))));

	
	return TEST_SUCCESS;
}





template<class InnerType, typename std::pointer_traits<InnerType>::element_type value = {}>
int NotNull_test()
{
	using elem_type = typename std::pointer_traits<InnerType>::element_type;
	elem_type Variable = value;
	InnerType pVariable = std::addressof(Variable);
	NT::NotNull<decltype(pVariable)> notNull1(pVariable);
	TEST_CHECK_STAGE(pVariable == notNull1);




	bool result = false;
	try
	{
		pVariable = nullptr;
		NT::NotNull<decltype(pVariable)> notNull2(pVariable);
	}
	catch(const NT::NotNullException& e) { result = true; };
	TEST_CHECK_STAGE(result);
	




	pVariable = std::addressof(Variable);
	auto notNull3 = notNull1;
	TEST_CHECK_STAGE((pVariable == notNull3) &&
					 (notNull1 == notNull3));

	



	decltype(notNull1) notNull4 = pVariable;
	(void) notNull4;
	notNull4 = notNull1;
	TEST_CHECK_STAGE((pVariable == notNull4) && 
					 (notNull1 == notNull4));




	auto notNullWillBeMoved = notNull1;
	auto notNull5 = std::move(notNullWillBeMoved);
	//cppcheck-suppress accessMoved
	TEST_CHECK_STAGE((pVariable == notNull5) &&
					 (notNullWillBeMoved == notNull5));


	return TEST_SUCCESS;
}

template<class InnerType, typename std::pointer_traits<InnerType>::element_type value = {}>
int NotNull_smart_copyable_test()
{
	using elem_type = typename std::pointer_traits<InnerType>::element_type;
	InnerType pVariable = InnerType(new elem_type(value));
	NT::NotNull<InnerType> notNull1(pVariable);
	TEST_CHECK_STAGE(pVariable == notNull1);




	bool result = false;
	try
	{
		pVariable = nullptr;
		NT::NotNull<decltype(pVariable)> notNull2(std::move(pVariable));
	}
	catch(const NT::NotNullException& e) { result = true; };
	TEST_CHECK_STAGE(result);




	pVariable = InnerType(new elem_type(value));
	notNull1 = pVariable;
	auto notNull3 = notNull1;
		result = (pVariable == notNull3) &&
				 //cppcheck-suppress knownConditionTrueFalse
				 (notNull1 == notNull3);
	TEST_CHECK_STAGE(result);



	
	decltype(notNull1) notNull4 = InnerType(new elem_type(value));
	(void) notNull4;
	notNull4 = notNull1;
		result = (pVariable == notNull4) &&
				 //cppcheck-suppress knownConditionTrueFalse
				 (notNull1 == notNull4);
	TEST_CHECK_STAGE(result);



	pVariable = InnerType(new elem_type(value));
	auto notNullWillBeMoved = pVariable;
	auto notNull5 = std::move(notNullWillBeMoved);
	 //cppcheck-suppress accessMoved
	result = (notNullWillBeMoved == nullptr) && 
			 (notNull5 == pVariable);
	TEST_CHECK_STAGE(result);




	auto notNullWillBeMoved2 = pVariable;
	auto notNull6 = InnerType(new elem_type);
	(void)notNull6;
	notNull6 = std::move(notNullWillBeMoved2);
	//cppcheck-suppress accessMoved
	result = (notNullWillBeMoved == nullptr) &&
			 (notNull6 == pVariable);
	TEST_CHECK_STAGE(result);





	return TEST_SUCCESS;
}

template<class InnerType, typename std::pointer_traits<InnerType>::element_type value = {}>
int NotNull_smart_moveonly_test()
{
	using elem_type = typename std::pointer_traits<InnerType>::element_type;
	InnerType pVariable = InnerType(new elem_type(value));
	NT::NotNull<InnerType> notNull1(std::move(pVariable));
	//cppcheck-suppress accessMoved
	TEST_CHECK_STAGE((pVariable == nullptr) && (*notNull1 == value));




	bool result = false;
	try
	{
		pVariable = nullptr;
		NT::NotNull<decltype(pVariable)> notNull2(std::move(pVariable));
	}
	catch(const NT::NotNullException& e) { result = true; };
	TEST_CHECK_STAGE(result);




	auto notNullWillBeMoved = InnerType(new elem_type(value));
	auto notNull5 = std::move(notNullWillBeMoved);
	//cppcheck-suppress accessMoved
	result = (notNullWillBeMoved == nullptr) && 
			 (*notNull5 == value);
	TEST_CHECK_STAGE(result);





	auto notNullWillBeMoved2 = InnerType(new elem_type(value));
	auto notNull6 = InnerType(new elem_type);
	(void)notNull6;
	notNull6 = std::move(notNullWillBeMoved2);
	//cppcheck-suppress knownConditionTrueFalse
	//cppcheck-suppress accessMoved
	result = (notNullWillBeMoved == nullptr) && 
			 (*notNull6 == value);
	TEST_CHECK_STAGE(result);





	return TEST_SUCCESS;
}

template<class InnerType, typename std::pointer_traits<InnerType>::element_type value = {}>
int NotNullHash_test()
{
	using elem_type = typename std::pointer_traits<InnerType>::element_type;
	elem_type Variable = value;
	InnerType pVariable = std::addressof(Variable);
	NT::NotNull<decltype(pVariable)> notNull1(pVariable);
	TEST_CHECK_STAGE((std::hash<decltype(pVariable)>{}(pVariable) 
				  == std::hash<decltype(notNull1)>{}(notNull1)) &&
				  (std::hash<decltype(pVariable)>{}(pVariable)
				  == std::hash<InnerType>{}(static_cast<InnerType&>(notNull1))));



	return TEST_SUCCESS;
}

template<class SmartPtr, typename std::pointer_traits<SmartPtr>::element_type value = {}>
int NotNullHash_smart_test()
{
	using elem_type = typename std::pointer_traits<SmartPtr>::element_type;
	NT::NotNull<SmartPtr> notNull1(SmartPtr(new elem_type(value)));
	TEST_CHECK_STAGE(std::hash<SmartPtr>{}(static_cast<SmartPtr&>(notNull1)) 
				  == std::hash<decltype(notNull1)>{}(notNull1));



	return TEST_SUCCESS;
}

template<class Ret, typename ... Args>
int NotNull_func_test(Ret(*pFun)(Args...), Args... args)
{
	NT::NotNull<Ret(*)(Args...)> notNullFun(pFun);
	TEST_CHECK_STAGE(notNullFun == pFun &&
					((*notNullFun)(args...) == (*pFun)(args...)));



	return TEST_SUCCESS;
}

int example()
{
	return 1337;
}

struct E 
{
	int a;
	E(int _a) : a(_a) {};
};

bool operator==(const E& l, const E& r)
{
	return (l.a == r.a);
}

E adv(char, void*, E&)
{
	return E(1337);
}


int main()
{
	std::cout << '\n';

	TEST_VERIFY(NotNull_unspec_type_test());

	TEST_VERIFY((NotNull_test<int*, 1337>()));
	TEST_VERIFY((NotNullHash_test<int*, 1337>()));

	E var(42);
	void* pV = nullptr;
	TEST_VERIFY((NotNull_func_test<int>(&example)));
	TEST_VERIFY((NotNull_func_test<E, char, void*, E&>(&adv, 'c', pV, var)));
	
	TEST_VERIFY((NotNull_smart_copyable_test<std::shared_ptr<int>, 1337>()))
	TEST_VERIFY((NotNullHash_smart_test<std::shared_ptr<int>, 1337>()))

	TEST_VERIFY((NotNull_smart_moveonly_test<std::unique_ptr<int>, 1337>()))
	TEST_VERIFY((NotNullHash_smart_test<std::unique_ptr<int>, 1337>()))


	std::cout << '\n' 
    << NT::Testing::TestsFailed 
    << " subtests failed" << std::endl;

    return NT::Testing::TestsFailed;
}