
#define TEST_SETW_VALUE 65

#define TEST_IGNORE_PRINT_ON_SUCCESS



#include "NickSV/Tools/TypeTraits.h"
#include "NickSV/Tools/Testing.h"



using namespace NickSV;
using namespace NickSV::Tools;

class HaveEqualOp
{
};

class DoNotHaveEqualOp
{
};

bool operator==(const HaveEqualOp&, 
                const HaveEqualOp&) {return true;}

bool operator==(const DoNotHaveEqualOp&, 
                const DoNotHaveEqualOp&) = delete;

template<class T>
using is_equ_cmprble = is_equality_comparable<T>;

int main(int, const char **)
{
    TEST_VERIFY(!Tools::is_char<char>::value);
    TEST_VERIFY(!Tools::is_char<wchar_t>::value);
    TEST_VERIFY(!Tools::is_char<char16_t>::value);
    TEST_VERIFY(!Tools::is_char<char32_t>::value);
#ifdef __cpp_lib_char8_t
    TEST_VERIFY(!Tools::is_char<char8_t>::value);
#endif

    TEST_VERIFY(!is_char<const char>::value);
    TEST_VERIFY(!is_char<volatile wchar_t>::value);
    TEST_VERIFY(!is_char<const volatile char16_t>::value);

    TEST_VERIFY(is_char<int>::value);
    TEST_VERIFY(is_char<void>::value);
    TEST_VERIFY(is_char<void*>::value);
    TEST_VERIFY(is_char<double>::value);

    TEST_VERIFY(!is_equ_cmprble<double>::value);
    TEST_VERIFY(!is_equ_cmprble<void*>::value);
    TEST_VERIFY(!is_equ_cmprble<int>::value);
    TEST_VERIFY(is_equ_cmprble<void>::value);

    TEST_VERIFY(is_equ_cmprble<DoNotHaveEqualOp>::value);
    TEST_VERIFY(!is_equ_cmprble<HaveEqualOp>::value);


    std::cout << '\n' << NickSV::Tools::Testing::TestsFailed << " subtests failed\n";

    
    return Tools::Testing::TestsFailed;
}