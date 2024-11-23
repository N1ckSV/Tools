


#include "NickSV/Tools/Matrix.h"
#include "NickSV/Tools/Testing.h"

namespace NT = NickSV::Tools;

static int MatrixTest()
{
    return TEST_SUCCESS;
}

template<typename T>
class ShowType;

int main()
{
    static_assert(NT::is_multiplicable<double, float>::value, "");
    static_assert(NT::is_multiplicable<int, long long>::value, "");
    static_assert(NT::is_multiplicable<char, char>::value, "");

    static_assert(!NT::is_multiplicable<NT::is_multiplicable<double, float>, int>::value, "");
    static_assert(!NT::is_multiplicable<std::string, int>::value, "");

    static_assert(std::is_same<NT::rangest_matrix_value_t<NT::MatrixT<int>, NT::MatrixT<double>>, double>::value, "");

    static_assert(NT::is_matrix_expression<NT::MatrixT<int>>::value, "");
    static_assert(!NT::is_matrix_expression<int>::value, "");

    //static_assert(std::is_same<NT::rangest_matrix_value_t<NT::Row<double>, int>, double>::value, "");

    //static_assert(std::is_same<NT::rangest_matrix_value_t<int, NT::Row<double>>, double>::value, "");




	TEST_VERIFY(MatrixTest());

    std::cout << '\n' << NickSV::Tools::Testing::TestsFailed << " subtests failed\n";

    return NT::Testing::TestsFailed;
}