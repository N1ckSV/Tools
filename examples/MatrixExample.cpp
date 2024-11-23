

#include "NickSV/Tools/Matrix.h"
#include "NickSV/Tools/Testing.h"

namespace NT = NickSV::Tools;

template<class T>
class ShowType;

int main()
{

    using namespace NickSV;
    using namespace NickSV::Tools;

    auto Mat1 = DiagonalMatrix<double>(10, 5.5);
    auto Mat2 = DiagonalMatrix<double>(10, 10);

    MatrixT<double> Mat3 = {{1, 2, 3}, {4,5,6}, {7,8,9}};
    MatrixT<double> Mat4 = {
        {1,  1,  1,  1}, 
        {5,  6,  7,  8}, 
        {2, 2, 2, 2}
        };
    MatrixT<double> Mat5 = {
        { 1,  2,  1}, 
        { 1,  5,  1}, 
        { 1,  8,  1}, 
        { 1, 11,  1}
        };
    
    std::cout << (Mat4 * Mat5).ToString("\n  ") << std::endl << std::endl;
    std::cout << (Mat1 - Mat2).ToString("\n  ") << std::endl << std::endl;
    std::cout << (3 * Mat1 + Mat2).ToString("\n  ") << std::endl << std::endl;
    std::cout << MatrixStaticCast<int>(3 * Mat1 + Mat2).ToString("\n  ") << std::endl << std::endl;

    std::cout << (-MatrixStaticCast<int>(3 * Mat1 + Mat2)).ToString("\n  ") << std::endl << std::endl;

    std::cout << Transpose(Mat4).ToString("\n  ") << std::endl << std::endl;
    
    Conjugate(DiagonalMatrix<int>(10, 4)).Rows();

    std::cout << Conjugate(DiagonalMatrix<int>(10, 4)).ToString("\n  ") << std::endl << std::endl;

    auto matUint = DiagonalMatrix<size_t>(10, 4);
    auto matInt = DiagonalMatrix<int>(10, 4);
    

    return 0;
}