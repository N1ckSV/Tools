#ifndef __NICKSV_MATRIX_H__
#define __NICKSV_MATRIX_H__
#pragma once


#include <type_traits>
#include <ostream>
#include <sstream>
#include <vector>
#include <functional>
#include <complex>


#include "NickSV/Tools/TypeTraits.h"


// use debug only assertions instead of excep throw, e.g matrix.At(y,x) out of bounds
#ifdef NICKSV_USE_ASSERTIONS
    #define NICKSV_MATRIX_INVALID_ARG(cond, text) NICKSV_ASSERT(!!(cond), text);
#else
    #define NICKSV_MATRIX_INVALID_ARG(cond, text) if(!(cond)) throw std::invalid_argument(text);
#endif


namespace NickSV {
namespace Tools {



template<class>
class MatrixT;

using MatrixSizeType = size_t;


template<class>
struct is_matrix : std::false_type {};


template<class ValT>
struct is_matrix<MatrixT<ValT>> : std::true_type {};


template<class, class>
class MatrixExpression;


template<class T, class ValT>
class Row
{
public:
    using ValueType = ValT;

    Row() = delete;
    Row(MatrixExpression<T, ValueType>* pMatEx, MatrixSizeType index) 
        : m_pMatEx(pMatEx), m_Index(index) {};

    inline auto operator[](MatrixSizeType index) const
    -> std::conditional_t<is_matrix<T>::value, const ValueType&, ValueType>
    {
        return m_pMatEx->AtUnsafe(m_Index, index);
    }
    
    template<class U = T>
    inline auto operator[](MatrixSizeType index)
    -> std::enable_if_t<is_matrix<U>::value, ValueType&>
    {
        return m_pMatEx->AtUnsafe(m_Index, index);
    }

private:
    MatrixExpression<T, ValueType>* m_pMatEx;
    MatrixSizeType m_Index;
};

 
template<class T, class ValT>
class MatrixExpression
{
public:

    using ValueType = ValT;

    inline MatrixSizeType Cols() const
    {
       return static_cast<const T*>(this)->Cols();
    }
    
    inline MatrixSizeType Rows() const
    {
       return static_cast<const T*>(this)->Rows();
    }

    inline bool IsEmpty() const
    {
        return !Cols() || !Rows();
    }

    inline auto At(MatrixSizeType y, MatrixSizeType x) const
    -> std::conditional_t<is_matrix<T>::value, const ValueType&, ValueType>
    {
        NICKSV_MATRIX_INVALID_ARG(y < Rows() && x < Cols(), "At(y, x) - index param out of range");
        return static_cast<const T*>(this)->AtUnsafe(y, x);
    }

    template<class U = T>
    inline auto At(MatrixSizeType y, MatrixSizeType x)
    -> std::enable_if_t<is_matrix<U>::value, ValueType&>
    {
        NICKSV_MATRIX_INVALID_ARG(y < Rows() && x < Cols(), "At(y, x) - index param out of range");
        return static_cast<T*>(this)->AtUnsafe(y, x);
    }

    inline auto AtUnsafe(MatrixSizeType y, MatrixSizeType x) const
    -> std::conditional_t<is_matrix<T>::value, const ValueType&, ValueType>
    {
        return static_cast<const T*>(this)->AtUnsafe(y, x);
    }

    template<class U = T>
    inline auto AtUnsafe(MatrixSizeType y, MatrixSizeType x)
    -> std::enable_if_t<is_matrix<U>::value, ValueType&>
    {
        return static_cast<T*>(this)->AtUnsafe(y, x);
    }

    inline auto operator[](MatrixSizeType index) const
    -> const Row<T, ValT>
    {
        return { this, index };
    }

    inline auto operator[](MatrixSizeType index)
    -> Row<T, ValT>
    {
        return { this, index };
    }


    std::string ToString(std::string rowSeparator = " ") const
    {
        std::ostringstream out;
        out << "{ ";
        for(MatrixSizeType row = 0; row < Rows(); ++row)
        {
            out << "{ ";
            for(MatrixSizeType col = 0; col < Cols(); ++col)
            {
                out << this->AtUnsafe(row, col);
                if(col < Cols() - 1)
                    out << ", ";
            }
            out << " }";
            if(row < Rows() - 1)
                    out << "," << rowSeparator;
        }
        out << " }";
        return out.str();
    }

};






template<class T, bool = std::is_class<T>::value>
struct is_matrix_expression : std::false_type {};

template<class T>
struct is_matrix_expression<T, true> 
    : std::is_base_of<MatrixExpression<std::remove_cvref_t<T>, typename std::remove_cvref_t<T>::ValueType>, std::remove_cvref_t<T>> {};

template<class T>
struct is_matrix_expression<MatrixExpression<T, typename T::ValueType>, true> : std::true_type {};









template<typename NumberT1, typename NumberT2, class Enable = void>
struct rangest_matrix_value;

template<typename NumberT1, typename NumberT2>
struct rangest_matrix_value<NumberT1, NumberT2,
    std::enable_if_t<!is_matrix_expression<NumberT1>::value && !is_matrix_expression<NumberT2>::value>>
{ 
    using type = std::conditional_t<(sizeof(NumberT1) > sizeof(NumberT2)), NumberT1, NumberT2>;
};

template<typename MatExp1, typename MatExp2>
struct rangest_matrix_value<MatExp1, MatExp2,
    std::enable_if_t<is_matrix_expression<MatExp1>::value && is_matrix_expression<MatExp2>::value>>
{ 
    using type = std::conditional_t<
        (sizeof(typename MatExp1::ValueType) > sizeof(typename MatExp2::ValueType)), 
        typename MatExp1::ValueType, typename MatExp2::ValueType
        >;
};

template<typename NumberT, typename MatExp>
struct rangest_matrix_value<NumberT, MatExp,
    std::enable_if_t<!is_matrix_expression<NumberT>::value && is_matrix_expression<MatExp>::value>>
{
    using type = std::conditional_t<
        (sizeof(NumberT) > sizeof(typename MatExp::ValueType)), 
        NumberT, typename MatExp::ValueType
        >;
};

template<typename NumberT, typename MatExp>
struct rangest_matrix_value<MatExp, NumberT,
    std::enable_if_t<!is_matrix_expression<NumberT>::value && is_matrix_expression<MatExp>::value>>
{
    using type = std::conditional_t<
        (sizeof(typename MatExp::ValueType) > sizeof(NumberT)), 
        typename MatExp::ValueType, NumberT
        >;
};

template<typename Expr1, typename Expr2>
using rangest_matrix_value_t = typename rangest_matrix_value<Expr1, Expr2>::type;





template<typename Operation>
class MatrixUnaryOp :
    public MatrixExpression<MatrixUnaryOp<Operation>, typename Operation::ReturnType>, 
    private Operation
{
public:
    using OperationType = Operation;
    using ValueType = typename OperationType::ReturnType;
    using OperandType = 
        std::conditional_t<
            is_matrix<typename OperationType::OperandType>::value,
            const typename OperationType::OperandType&,
            const typename OperationType::OperandType
            >;

    MatrixUnaryOp(const typename OperationType::OperandType& matrixE) : matEx(matrixE) {};

    inline ValueType AtUnsafe(MatrixSizeType y, MatrixSizeType x) const
    {
        return OperationType::Operate(matEx, y, x);
    }

    inline MatrixSizeType Cols() const
    {
       return OperationType::Cols(matEx);
    }
    
    inline MatrixSizeType Rows() const
    {
       return OperationType::Rows(matEx);
    }

private:

    OperandType matEx;

};

template<class Operation>
class MatrixBinaryOp :
    public MatrixExpression<MatrixBinaryOp<Operation>, typename Operation::ReturnType>,
    private Operation  
{
public:

    using OperationType = Operation;
    using ValueType = typename OperationType::ReturnType;
    using LeftType = 
        std::conditional_t<
            is_matrix<typename OperationType::LeftType>::value,
            const typename OperationType::LeftType&,
            const typename OperationType::LeftType
            >;
    
    using RightType = 
        std::conditional_t<
            is_matrix<typename OperationType::RightType>::value,
            const typename OperationType::RightType&,
            const typename OperationType::RightType
            >;


    MatrixBinaryOp(const typename OperationType::LeftType& l, const typename OperationType::RightType& r) : left(l), right(r) {};

    inline ValueType AtUnsafe(MatrixSizeType y, MatrixSizeType x) const
    {
        return OperationType::Operate(left, right, y, x);
    }

    inline MatrixSizeType Cols() const
    {
       return OperationType::Cols(left, right);
    }
    
    inline MatrixSizeType Rows() const
    {
       return OperationType::Rows(left, right);
    }



private:

    LeftType left;
    RightType right;

};

template<class ValT>
class MatrixT : public MatrixExpression<MatrixT<ValT>, ValT>
{
public:

    using ValueType = ValT;
    using ParentType = MatrixExpression<MatrixT<ValT>, ValT>;

    MatrixT() = default;
    MatrixT(const MatrixT&) = default;
    MatrixT(MatrixT&&) noexcept = default;
    MatrixT& operator=(const MatrixT&) = default;
    MatrixT& operator=(MatrixT&&) noexcept = default;

    MatrixT(MatrixSizeType rows, MatrixSizeType cols) : m_nRows(rows), m_nCols(cols), m_vData(cols*rows) {}

    template<class T, class MatExValT>
    MatrixT(const MatrixExpression<T, MatExValT>& matEx) 
        : MatrixT(matEx.Rows(), matEx.Cols())
    {
        AssignElems(matEx);
    }

    /**
     * @exception
     * Strong exception guarantee
     * 
     * @sa AssignTryNoBuffer
     */
    template<class T, class MatExValT>
    MatrixT& operator=(const MatrixExpression<T, MatExValT>& matEx)
    {
        if (static_cast<const void*>(this) == static_cast<const void*>(std::addressof(matEx)))
            return *this;

        MatrixT<ValueType> buff(matEx);
        std::swap(*this, buff);
        return *this;
    }


    template<class ListValueT>
    MatrixT(std::initializer_list<std::initializer_list<ListValueT>> list) 
        :   MatrixT(list.size(), list.size() ? list.begin()->size() : 0)
    {
        auto vIter = m_vData.begin();
        for(std::initializer_list<ListValueT> row : list)
        {
            NICKSV_MATRIX_INVALID_ARG(row.size() == m_nCols, 
                "Every row in std::initializer_list should have the same size");
            for(const ListValueT& item : row)
                *(vIter++) = item;
        }
    }

    

    inline void SetAll(const ValueType& value) 
        noexcept(noexcept(std::declval<ValueType&>() = std::declval<const ValueType&>()))
    {
        for(ValueType& item : m_vData)
            item = value;
    }

    inline MatrixSizeType Cols() const noexcept
    {
       return m_nCols;
    }
    
    inline MatrixSizeType Rows() const noexcept
    {
       return m_nRows;
    }

    
    inline ValueType& AtUnsafe(MatrixSizeType y, MatrixSizeType x) noexcept
    {
        return m_vData[ y * m_nCols + x ];
    }

    inline const ValueType& AtUnsafe(MatrixSizeType y, MatrixSizeType x) const noexcept
    {
        return m_vData[ y * m_nCols + x ];
    }

private:
    
    template<class T, class MatExValT>
    void inline AssignElems(const MatrixExpression<T, MatExValT>& matEx)
    {
        for(MatrixSizeType index = 0; index < m_nRows*m_nCols; ++index)
            m_vData[index] = matEx.AtUnsafe(index / m_nCols, index % m_nCols);
    }


    MatrixSizeType m_nRows = 0;
    MatrixSizeType m_nCols = 0;
    std::vector<ValueType> m_vData;
};

template<class T, class ValT> 
inline std::ostream& operator << (std::ostream& out, const MatrixExpression<T, ValT>& matEx)
{
    return out << matEx.ToString();
}

/**
 * @brief
 * At least one of operand types has to be matrix expression.
 * 
 * @warning
 * "ReturnType Operate(const LeftType& l, const RightType& l, MatrixSizeType y, MatrixSizeType x) const"
 * has to be defined in derived class
 */
template<class LeftT, class RightT>
struct MatrixDefaultBinaryOp
{
    static_assert(is_matrix_expression<LeftT>::value || is_matrix_expression<RightT>::value,
        "At least one of operand types has to be matrix expression");

    using LeftType = LeftT;
    using RightType = RightT;
    using ReturnType = rangest_matrix_value_t<LeftType, RightType>;

    template<class LT = LeftType>
    inline auto Cols(const LT& l, const RightType& r) const noexcept
    -> std::enable_if_t<is_matrix_expression<LT>::value, MatrixSizeType>
    {
       return l.Cols();
    }
    
    template<class LT = LeftType>
    inline auto Rows(const LT& l, const RightType& r) const noexcept
    -> std::enable_if_t<is_matrix_expression<LT>::value, MatrixSizeType>
    {
       return l.Rows();
    }

    template<class LT = LeftType, class RT = RightType>
    inline auto Cols(const LT& l, const RT& r) const noexcept
    -> std::enable_if_t<
        !is_matrix_expression<LT>::value &&
        is_matrix_expression<RT>::value,
        MatrixSizeType>
    {
       return r.Cols();
    }
    
    template<class LT = LeftType, class RT = RightType>
    inline auto Rows(const LT& l, const RT& r) const noexcept
    -> std::enable_if_t<
        !is_matrix_expression<LT>::value &&
        is_matrix_expression<RT>::value, 
        MatrixSizeType>
    {
       return r.Rows();
    }
};

/**
 * @brief
 * MatExType has to be matrix expression
 * 
 * @note
 * "ReturnType Operate(const MatExType& m, MatrixSizeType y, MatrixSizeType x) const" defining can be avoided. 
 * In this case default Operate() is just static_cast to ReturnType
 */
template<class MatExType, class RetT = typename MatExType::ValueType>
struct MatrixDefaultUnaryOp
{
    static_assert(is_matrix_expression<MatExType>::value, "MatExType has to be matrix expression");

    using OperandType = MatExType;
    using ReturnType = RetT;

    ReturnType Operate(const OperandType& m, MatrixSizeType y, MatrixSizeType x) const
    { 
        return static_cast<ReturnType>(m.AtUnsafe(y,x));
    };

    inline MatrixSizeType Rows(const OperandType& m) const noexcept
    {
       return m.Rows();
    }

    inline MatrixSizeType Cols(const OperandType& m) const noexcept
    {
       return m.Cols();
    }

};



template<class U , class V>
struct MatrixPlusOp : MatrixDefaultBinaryOp<U, V>
{
    using LeftType = U;
    using RightType = V;
    using ReturnType = rangest_matrix_value_t<LeftType, RightType>;

    ReturnType Operate(const LeftType& l, const RightType& r, MatrixSizeType y, MatrixSizeType x)  const
    { 
        return static_cast<ReturnType>(l.AtUnsafe(y,x) + r.AtUnsafe(y,x));
    };
};

template<class U , class V>
struct MatrixBinaryMinusOp : MatrixDefaultBinaryOp<U, V>
{
    using LeftType = U;
    using RightType = V;
    using ReturnType = rangest_matrix_value_t<LeftType, RightType>;

    ReturnType Operate(const LeftType& l, const RightType& r, MatrixSizeType y, MatrixSizeType x)  const
    { 
        return static_cast<ReturnType>(l.AtUnsafe(y,x) - r.AtUnsafe(y,x));
    };
};

template<class U , class V>
struct MatrixMultOp : MatrixDefaultBinaryOp<U, V>
{
    using LeftType = U;
    using RightType = V;
    using ReturnType = rangest_matrix_value_t<LeftType, RightType>;

    ReturnType Operate(const LeftType& l, const RightType& r, MatrixSizeType y, MatrixSizeType x) const
    { 
        ReturnType sum = 0;
        for(MatrixSizeType col = 0; col < r.Rows(); ++col)
            sum += static_cast<ReturnType>(l.AtUnsafe(y, col) * r.AtUnsafe(col, x));
        return sum;
    };

    inline MatrixSizeType Rows(const U& l, const V& r) const noexcept
    {
       return l.Rows();
    }

    inline MatrixSizeType Cols(const U& l, const V& r) const noexcept
    {
       return r.Cols();
    }

};

template<class NumberT, class U>
struct MatrixMultNumOp : MatrixDefaultBinaryOp<NumberT, U>
{
    using LeftType = NumberT;
    using RightType = U;
    using ReturnType = rangest_matrix_value_t<LeftType, RightType>;

    ReturnType Operate(const LeftType& n, const RightType& u, MatrixSizeType y, MatrixSizeType x)  const
    { 
        return static_cast<ReturnType>(n * u.AtUnsafe(y,x)); 
    };
};

template<class U>
struct MatrixUnaryMinusOp : MatrixDefaultUnaryOp<U>
{
    using OperandType = U;
    using ReturnType = decltype(-std::declval<OperandType>().AtUnsafe(std::declval<MatrixSizeType>(),std::declval<MatrixSizeType>()));

    ReturnType Operate(const OperandType& u, MatrixSizeType y, MatrixSizeType x)  const
    { 
        return -u.AtUnsafe(y,x); 
    };
};


template<class U>
struct MatrixTransposesOp : MatrixDefaultUnaryOp<U>
{
    using OperandType = U;
    using ReturnType = typename OperandType::ValueType;

    ReturnType Operate(const OperandType& m, MatrixSizeType y, MatrixSizeType x)  const
    { 
        return m.AtUnsafe(x,y); 
    };

    inline MatrixSizeType Rows(const OperandType& m) const noexcept
    {
       return m.Cols();
    }

    inline MatrixSizeType Cols(const OperandType& m) const noexcept
    {
       return m.Rows();
    }
};

template<class U, class RetT = decltype(std::conj(std::declval<U>().AtUnsafe(std::declval<MatrixSizeType>(),std::declval<MatrixSizeType>())))>
struct MatrixConjugateOp : MatrixDefaultUnaryOp<U, RetT>
{
    
    using OperandType = U;
    using ReturnType = RetT;

    ReturnType Operate(const OperandType& u, MatrixSizeType y, MatrixSizeType x)  const
    { 
        return std::conj(u.AtUnsafe(x,y)); 
    };
};


//////////////////////////////////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////////////////////////////////


template<class NewValT, class T, class ValT> 
inline auto MatrixStaticCast(const MatrixExpression<T, ValT>& matEx)
-> MatrixUnaryOp<MatrixDefaultUnaryOp<T, NewValT>>
{
    return { static_cast<const T&>(matEx) };
}


template<class T, class ValT> 
inline auto Transpose(const MatrixExpression<T, ValT>& matEx)
-> MatrixUnaryOp<MatrixTransposesOp<T>>
{
    return { static_cast<const T&>(matEx) };
}

template<class T, class ValT> 
inline auto Conjugate(const MatrixExpression<T, ValT>& matEx)
-> MatrixUnaryOp<MatrixConjugateOp<T>>
{
    return { static_cast<const T&>(matEx) };
}



template<class T>
MatrixT<T> DiagonalMatrix(MatrixSizeType size, T value = T{})
{
    auto mat = MatrixT<T>(size, size);
    mat.SetAll(T{});
    for (MatrixSizeType index = 0; index < size; ++index)
        mat.AtUnsafe(index, index) = value;
    return mat;
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////
// OPERATORS
//////////////////////////////////////////////////////////////////


template<class U , class UValT, class V , class VValT> 
auto operator + (
    const MatrixExpression<U, UValT>& left, 
    const MatrixExpression<V, VValT>& right)
-> MatrixBinaryOp<MatrixPlusOp<U, V>>
{
    NICKSV_MATRIX_INVALID_ARG(
        (left.Rows() == right.Rows()) &&
        (left.Cols() == right.Cols()) &&
        !left.IsEmpty(), 
        "You can only add non-empty matrices with the same size");
    return { static_cast<const U&>(left), static_cast<const V&>(right) };
}


template<class U , class UValT, class V , class VValT> 
auto operator - (const MatrixExpression<U, UValT>& left, const MatrixExpression<V, VValT>& right)
-> MatrixBinaryOp<MatrixBinaryMinusOp<U, V>>
{    
    NICKSV_MATRIX_INVALID_ARG(
        !left.IsEmpty() &&
        (left.Rows() == right.Rows()) &&
        (left.Cols() == right.Cols()),
        "You can only subtract non-empty matrices with the same size");
    return { static_cast<const U&>(left), static_cast<const V&>(right) };
}


template<class U , class UValT, class V , class VValT> 
auto operator * (const MatrixExpression<U, UValT>& left, const MatrixExpression<V, VValT>& right)
-> MatrixBinaryOp<MatrixMultOp<U, V>>
{
    NICKSV_MATRIX_INVALID_ARG(
        !left.IsEmpty() &&
        !right.IsEmpty() &&
        (left.Cols() == right.Rows()), 
        "Incorrect matrix size during multiplication");
    return { static_cast<const U&>(left), static_cast<const V&>(right) };
}

template<class NumberT , class V , class VValT> 
auto operator * (const NumberT& left, const MatrixExpression<V, VValT>& right)
-> std::enable_if_t<!is_matrix_expression<NumberT>::value && is_multiplicable<NumberT, VValT>::value, 
        MatrixBinaryOp<MatrixMultNumOp<NumberT, V>>>
{
    NICKSV_MATRIX_INVALID_ARG(!right.IsEmpty(), "Incorrect matrix size during multiplication");
    return { left, static_cast<const V&>(right) };
}

template<class NumberT , class V , class VValT> 
auto operator * (const MatrixExpression<V, VValT>& left, const NumberT& right)
-> std::enable_if_t<!is_matrix_expression<NumberT>::value && is_multiplicable<NumberT, VValT>::value, 
        MatrixBinaryOp<MatrixMultNumOp<NumberT, V>>> 
{
    return operator*(right, left);
}



template<class U , class UValT> 
auto operator - (const MatrixExpression<U, UValT>& m)
-> MatrixUnaryOp<MatrixUnaryMinusOp<U>>
{    
    NICKSV_MATRIX_INVALID_ARG(!m.IsEmpty(), "You can only subtract non-empty matrices with the same size");
    return { static_cast<const U&>(m) };
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


}}


#endif // __NICKSV_MATRIX_H__