#ifndef __NICKSV_STRING_H__
#define __NICKSV_STRING_H__


#include "Definitions.h"



#include <string>



namespace NickSV {
namespace Tools {


template<class T>
class BasicStringView
{
    using CharType = T;
    using SizeType = size_t;

    BasicStringView() noexcept = default;

    BasicStringView(const std::basic_string<CharType>& str) : m_Size(str.size()), m_pzsData(std::addressof(str[0])) {}

    BasicStringView(const char* pzs, SizeType count) : m_Size(count), m_pzsData(pzs) {}

    BasicStringView(const char* pzs) : m_Size(strlen(pzs)), m_pzsData(pzs) {}

    BasicStringView( const BasicStringView& other ) noexcept = default;

    template< class IterBegin, class IterEnd >
    BasicStringView( IterBegin first, IterEnd last ) : m_pzsData(std::addressof(*first))
    {
        auto dist = std::distance(first, last);
        NICKSV_ASSERT(dist >= 0, "Iterator range is invalid [ std::distance(first, last) < 0 ]");
        m_Size = static_cast<SizeType>(dist);
        std::string_view::iterator;
    };
    
    BasicStringView( std::nullptr_t ) = delete;

    BasicStringView& operator=( const BasicStringView& view ) noexcept = default;


    [[nodiscard]] SizeType Size() const
    { return m_Size; }
private:

    const char* m_pzsData = nullptr;
    SizeType m_Size = 0;
};










}}























#endif // __NICKSV_STRING_H__