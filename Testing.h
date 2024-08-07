#ifndef _NICKSV_TESTING
#define _NICKSV_TESTING
#pragma once



#include <iostream>
#include <string>
#include <iomanip>
#include <exception>
#include <type_traits>
#include <stdio.h>

// EXAMPLE BELOW
// EXAMPLE BELOW
// EXAMPLE BELOW

namespace NickSV {
namespace Tools {
namespace Testing {


#ifndef TEST_IGNORE_PRINT_ON_SUCCESS
/**
 * @def TEST_IGNORE_PRINT_ON_SUCCESS
 * @brief Special define disabling @ref TEST_VERIFY
 *        to print succeeded tests
*/
#define TEST_IGNORE_PRINT_ON_SUCCESS
#undef  TEST_IGNORE_PRINT_ON_SUCCESS
#endif


// define TEST_SETW_VALUE with your value for pretty text alignment
#ifndef TEST_SETW_VALUE
#define TEST_SETW_VALUE 50
#endif

// Insert it inside the test's function body after each test stage with its result 
// returns if the result is false (EXAMPLE BELOW)
#define TEST_CHECK_STAGE(result) ++(NickSV::Tools::Testing::FailedStage);                  \
                                 if(!(result)) return NickSV::Tools::Testing::FailedStage

#define TEST_SUCCESS 0

#define TEST_PRINT_RESULT(testName)                                                 \
        std::cout << std::setw(TEST_SETW_VALUE) << std::string(testName) << ": "

#ifndef TEST_IGNORE_PRINT_ON_SUCCESS 
    #define TEST_PRINT_PASSED(testName)                                             \
            TEST_PRINT_RESULT(testName) << "PASSED" << std::endl                       
#else
    #define TEST_PRINT_PASSED(testName) ((void)0)
#endif 

// Executes the test and prints the result,
// also handles an exception and treats it as a failure (EXAMPLE BELOW)
#define TEST_VERIFY(test)                                                           \
    static_assert(std::is_convertible<decltype(test), size_t>::value,               \
     "test function returns type that is not converible to size_t");                \
    try {                                                                           \
        NickSV::Tools::Testing::FailedStage = 0;                                    \
        size_t testOut = test; /*test called here*/                                 \
        if(!testOut) {                                                              \
            TEST_PRINT_PASSED(#test);                                               \
        }                                                                           \
        else {                                                                      \
            ++NickSV::Tools::Testing::TestsFailed;                                  \
            size_t stageOut = NickSV::Tools::Testing::FailedStage ?                 \
                NickSV::Tools::Testing::FailedStage : testOut;                      \
            TEST_PRINT_RESULT(#test)                                                \
                << "FAILED on stage "                                               \
                << stageOut << std::endl;                                           \
        }                                                                           \
    } catch (const std::exception& ex) {                                            \
        ++NickSV::Tools::Testing::TestsFailed;                                      \
        TEST_PRINT_RESULT(#test)                                                    \
            << "FAILED on stage "                                                   \
            << ++NickSV::Tools::Testing::FailedStage                                \
            <<  ": " << ex.what() << std::endl;                                     \
    } catch (...) {                                                                 \
        ++NickSV::Tools::Testing::TestsFailed;                                      \
        TEST_PRINT_RESULT(#test)                                                    \
            << "Unknown exception thrown on stage "                                 \
            << ++NickSV::Tools::Testing::FailedStage << std::endl;                  \
    }                                                                               \

// How many tests are failed for each TEST_VERIFY()  (EXAMPLE BELOW)
int TestsFailed = 0;

// At what stage the last test is failed in call of TEST_VERIFY()  (EXAMPLE BELOW)
int FailedStage = 0;

}}}  /*END OF NAMESPACES*/

//USAGE EXAMPLE
#if false
size_t test_foo()
{
    //... first stage code ...                                  |
    bool result = true; // is first stage succeded? (yes)       | STAGE 1
    TEST_CHECK_STAGE(result);  //returns 1 if result is false   |

    //... second stage code ...                                 |
    result = true; // is second stage succeded? (yes)           | STAGE 2
    TEST_CHECK_STAGE(result);  //returns 2 if result is false   |

    //... third stage code ...                                  |
    result = true; // is third stage succeded? (yes)            | STAGE 3
    TEST_CHECK_STAGE(result);  //returns 3 if result is false   |

    return TEST_SUCCESS; //returns 0
}

size_t test_bar()
{
    //... first stage code ...                                  |
    bool result = true; // is first stage succeded? (yes)       | STAGE 1
    TEST_CHECK_STAGE(result);  //returns 1 if result is false   |

    //... second stage code with ERROR ...                      |
    result = false; // is second stage succeded? (no)           | STAGE 2
    TEST_CHECK_STAGE(result);  //returns 2 if result is false   |

    return TEST_SUCCESS; //returns 0
}

int main()
{
    using namespace NickSV::Tools;

    TEST_VERIFY(test_foo());

    // Testing::FailedStage is a stage number where last TEST_VERIFY() is failed, 0 otherwise  

    if(Testing::FailedStage)
    {
        // Some code if test_foo() is failed
    }    

    TEST_VERIFY(test_bar());

    if( ! Testing::FailedStage) // FailedStage refreshes each TEST_VERIFY
    {
        // Some code if test_bar() is succeded
    }  

    // Testing::TestsFailed is a count of how many of tests are failed (for each TEST_VERIFY())
    std::cout << '\n' 
    << Testing::TestsFailed 
    << " subtests failed" << std::endl;
    // "subtests" here because the "test" is a whole binary runtime file (in CTest e.g.),
    // so actually test_foo and test_bar are subtests


    // if TestsFailed is 0 - whole test is succeded
    return Testing::TestsFailed;
}

//POSSIBLE OUTPUT:
//
//      test_foo(): PASSED
//      test_bar(): FAILED on stage 2
//
//  1 subtests failed
//
// Program returned 1

#endif //EXAMPLE



#endif //_NICKSV_TESTING