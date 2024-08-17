
#ifndef _NICKSV_VALUELOCK
#define _NICKSV_VALUELOCK
#pragma once


#include "Definitions.h"
#include "Utils.h"


#include <condition_variable>
#include <mutex>
#include <type_traits>
#include <list>
#include <array>
#include <algorithm>
#include <iostream>
#include <exception>




namespace NickSV {
namespace Tools {





/** 
 * Error when:
 * 
 * 1) Too many threads are locking ValueLock object,
 *    change slotsCount to how many of them are potentially
 *    can simultaneously lock a value in ValueLock.
 * 
 * 2) One of the threads locked two or more values
 *    in one ValueLock object at the same time 
 *    (which is not allowed, see LockAll() for total lock)
 *    and current thread did not find a free slot to lock its value
 *    and threw this exception.
*/
#define CONCURRENCY_ERROR_TEXT "Invalid function call: all ValueLock slots are busy"


#define INVALID_VALUE_ERROR_TEXT "Invalid function call: value has not yet been locked"

/**
 * @class ValueLock
 * 
 * @brief This class helps to control OBJECTS
 *        that are binded to some Value
 *        across multiple threads.
 * 
 * @tparam ValueT type of value to lock
 * @tparam slotCount max number of OBJECTS binded to ValueT
 * that can potentially and simultaneously be handled by threads
 * (for an indefinite number of needed slots see @ref DynamicValueLock)
 *
 * For example:
 * Imagine you have std::map<ID, User> mapUsers,
 * so mapUsers is OBJECT listed above and ID is ValueT.
 * And multiple threads can change mapUsers 
 * by adding, removing, changing Users.
 * So the first thought that came to my mind was to make 
 * std::map<ID, std::mutex> mapMutexes and
 * lock needed ID before changing mapUsers:
 * @code{.cpp} 
 *     mapMutexes[id].lock();
 *     mapUsers.at(id).doSomething();
 *     mapMutexes[id].unlock();
 * @endcode
 * But this is too heavy, especially when you need to lock
 * "all" IDs. So ValueLock helps in this situation:
 * @code{.cpp}
 *     // ValueLock<ID, slotCount> usersLock declared before
 *     // with lifetime is about the same as mapUsers
 *     usersLock.Lock(id);
 *     mapUsers.at(id).doSomething();
 *     usersLock.Unlock(id);
 * @endcode
 * or RAII-style
 * @code{.cpp}
 *     // ValueLock<ID, slotCount> usersLock declared before
 *     // with lifetime is about the same as mapUsers
 *     {
 *         // Same work as std::lock_guard
 *         ValueLockGuard<decltype(usersLock)> lockGuard(usersLock, id);
 *         mapUsers.at(id).doSomething();
 *     }
 * @endcode
 * More info in methods description.
 *
*/
template<typename ValueT, size_t slotCount>
class ValueLock
{
public:

    static_assert(std::is_default_constructible<ValueT>::value, "ValueT must be default constructible");
    static_assert(     is_equality_comparable<ValueT>::value, "ValueT must has equality operator overloaded");
    static_assert(std::is_copy_constructible<ValueT>::value, "ValueT must be copy constructible");
    static_assert(std::is_copy_assignable<ValueT>::value, "ValueT must be copy assignable");

    using ValueType = ValueT;

    struct ValueMutex
    {
        ValueMutex() = default;
        DECLARE_COPY_DELETE(ValueMutex);
        explicit ValueMutex(const ValueType& val) : Value(val) {}
        ValueMutex(ValueMutex&& rvalRef)
            : Value(std::move(rvalRef.Value)),
            RefCount(std::move(rvalRef.RefCount)) {}

        ValueMutex& operator=(ValueMutex&& rvalRef)
        {
            Value = std::move(rvalRef.Value);
            RefCount = std::move(rvalRef.RefCount);
            return *this;
        }

        std::mutex Mutex;
        ValueType Value = ValueType();
        uint32_t RefCount = 0;
    };

    using Container = std::array<ValueMutex, slotCount>;

    /**
     * @class Unlocker
     * 
     * @brief Unary functor that calls Unlock(value)
     *        on given pointer to ValueLock
     *        with value.
     * 
     * @warning 
     * Invoking throws the same exception as ValueLock::Unlock(value) if there is no stack unwinding,
     * otherwise printing error message to std::cerr and returns
     * 
    */
    class Unlocker
    {
        ValueType m_Value;
    public:
        Unlocker() = default;
        DECLARE_RULE_OF_5_DEFAULT(Unlocker, NOTHING);
        explicit Unlocker(const ValueType& value) : m_Value(value) {}

        /**
         * @throws 
         * Same exception as ValueLock::Unlock(value) if there is no stack unwinding,
         * otherwise printing error message to std::cerr and returns
         */
        inline void operator()(ValueLock* pValueLock) const
        {
            try { pValueLock->Unlock(m_Value); }
            catch(const std::exception& e)
            {
                #ifdef __cpp_lib_uncaught_exceptions
                if(!std::uncaught_exceptions()) throw;
                #else
                if(!std::uncaught_exception()) throw;
                #endif
                std::cerr << "Unlocker::operator() caught std::exception in call of ValueLock::Unlock()"
                             "during stack unwinding, it won't be rethrown. std::exception::what(): "
                          << e.what() << std::endl;
            }
        }
    };

    /**
     * @class UnlockerAll
     * 
     * @brief Unary functor that calls UnlockAll([value])
     *        on given pointer to ValueLock
     *        with value.
     * 
     * @warning 
     * Invoking throws the same exception as ValueLock::UnlockAll([value]) if there is no stack unwinding,
     * otherwise printing error message to std::cerr and returns
     * 
    */
    class UnlockerAll
    { 
        std::unique_ptr<ValueType> m_upKeepLockedValue;
    public:
        UnlockerAll() = default;
        DECLARE_RULE_OF_5_DEFAULT(UnlockerAll, NOTHING);
        explicit UnlockerAll(const ValueType& keepLockedValue) 
            : m_upKeepLockedValue(new ValueType(keepLockedValue)) {}

        /**
         * @throws 
         * Same exception as ValueLock::UnlockAll([value]) if there is no stack unwinding,
         * otherwise printing error message to std::cerr and returns
         */
        inline void operator()(ValueLock* pValueLock) const
        {
            try 
            {
                if(m_upKeepLockedValue) 
                    pValueLock->UnlockAll(*m_upKeepLockedValue);
                else 
                    pValueLock->UnlockAll();
            }
            catch(const std::exception& e)
            {
                #ifdef __cpp_lib_uncaught_exceptions
                if(!std::uncaught_exceptions()) throw;
                #else
                if(!std::uncaught_exception()) throw;
                #endif
                std::cerr << "UnlockerAll::operator() caught std::exception in call of ValueLock::UnlockAll()"
                             "during stack unwinding, it won't be rethrown. std::exception::what(): "
                          << e.what() << std::endl;
            }
        }
    };

    
    // Move only non-virtual
    DECLARE_COPY_DELETE(ValueLock);
    DECLARE_MOVE_DEFAULT(ValueLock, NOTHING);

    ValueLock() = default;

    void Lock(const ValueType& value) noexcept(false)
    {
        std::unique_lock<std::mutex> uLock(m_mtx);
        auto iterMutex = FindBusySlot(value);
        if(iterMutex == m_aValueMutexes.end())
        {
            iterMutex = FindEmptySlot();
            NICKSV_ASSERT(iterMutex != m_aValueMutexes.end(), CONCURRENCY_ERROR_TEXT);
            iterMutex->Value = value;
        }      
        ++(iterMutex->RefCount);
        uLock.unlock();
        iterMutex->Mutex.lock();
    }

    /**
     * @brief Locks every slot/value
     * 
     * @throws
     * the same exception that std::mutex::lock() throws and
     * unlocks everything that was successfully locked.
     */
    void LockAll() noexcept(false)
    {
        for_each_exception_safe(m_aValueMutexes.begin(), m_aValueMutexes.end(),
        [](ValueMutex& mut) { mut.Mutex.lock(); }, 
        [](ValueMutex& mut) noexcept { mut.Mutex.unlock(); });
    }
    
    /**
     * @brief Unlocks given value.
     * 
     * @param value is value to unlock
     * 
     * @throws - Same as std::mutex::lock(): can be thrown 
     *           by inner std::mutex (rare case)
     * 
     * @warning ValueLock::Lock(value) must be called 
     * by the current thread of execution, 
     * otherwise, the behavior is undefined.
    */
    void Unlock(const ValueType& value)  noexcept(false)
    {
        std::unique_lock<std::mutex> uLock(m_mtx);
        auto iterMutex = FindBusySlot(value);
        NICKSV_ASSERT(iterMutex != m_aValueMutexes.end(), INVALID_VALUE_ERROR_TEXT);
        --(iterMutex->RefCount);
        iterMutex->Mutex.unlock();
    }

    
    /**
     * @brief Unlocks all values.
     * 
     * @warning ValueLock::LockAll() must be called 
     * by the current thread of execution, 
     * otherwise, the behavior is undefined.
    */
    void UnlockAll() noexcept
    {
        for (auto& vMutex: m_aValueMutexes)
            vMutex.Mutex.unlock();
    }

    /**
     * @brief Unlocks all values except given one.
     * 
     * For noexcept verison see UnlockAllNoExcept().
     * 
     * @param keepLockedValue value to keep locked
     * 
     * @throws - Same as std::mutex::lock(): can be thrown 
     * by inner std::mutex (rare case)
     * 
     * @warning ValueLock::LockAll() must be called 
     * by the current thread of execution, 
     * otherwise, the behavior is undefined.
     * 
    */
    void UnlockAll(const ValueType& keepLockedValue) noexcept(false)
    {
        auto iterMutex = FindBusySlot(keepLockedValue);
        if(iterMutex == m_aValueMutexes.end())
        {
            iterMutex = FindEmptySlot();
            NICKSV_ASSERT(iterMutex != m_aValueMutexes.end(), CONCURRENCY_ERROR_TEXT);
            iterMutex->Value = keepLockedValue;
        }
        ++(iterMutex->RefCount); 
        for (auto& vMutex: m_aValueMutexes)
        {
            if(&vMutex != &(*iterMutex))
                vMutex.Mutex.unlock();
        }
    }


    bool TryLock(const ValueType& value)  noexcept(false)
    {
        std::unique_lock<std::mutex> uLock(m_mtx);
        auto iterMutex = FindBusySlot(value);
        if(iterMutex == m_aValueMutexes.end())
        {
            iterMutex = FindEmptySlot();
            NICKSV_ASSERT(iterMutex != m_aValueMutexes.end(), CONCURRENCY_ERROR_TEXT);
            iterMutex->Value = value;
        }      
        ++(iterMutex->RefCount);
        auto isLocked = iterMutex->Mutex.try_lock();
        if(!isLocked)
            --(iterMutex->RefCount);
        return isLocked;
    }

private:
    inline auto FindEmptySlot() -> typename Container::iterator
    {
        return std::find_if(m_aValueMutexes.begin(), m_aValueMutexes.end(), 
                [](const ValueMutex & mutex) noexcept { return mutex.RefCount == 0; });
    }
    inline auto FindBusySlot(const ValueType& value) -> typename Container::iterator
    {
        return std::find_if(m_aValueMutexes.begin(), m_aValueMutexes.end(), 
                [&value](const ValueMutex & mutex) noexcept { return (value == mutex.Value) && (mutex.RefCount > 0); });
    }
    Container m_aValueMutexes;
    std::mutex m_mtx;
};


/**
 * @class FakeValueLock
 * 
 * @brief Same as @ref ValueLock but working like a single mutex.
 *        Was made for benchmarking purpose.
 * 
*/
template<typename ValueT, size_t slotCount>
class FakeValueLock
{
public:
    using ValueType = ValueT;

    /**
     * @class Unlocker
     * 
     * @brief Same as ValueLock::Unlocker,
     *        but for FakeValueLock
     * 
    */
    class Unlocker
    {
    public:
        Unlocker() = default;
        DECLARE_RULE_OF_5_DEFAULT(Unlocker, NOTHING);
        explicit Unlocker(const ValueType& value) {}

        /**
         * @throws 
         * Same exception as FakeValueLock::Unlock(value) if there is no stack unwinding,
         * otherwise printing error message to std::cerr and returns
         */
        inline void operator()(FakeValueLock* pValueLock) const
        {
            try { pValueLock->Unlock(ValueType{}); }
            catch(const std::exception& e)
            {
                #ifdef __cpp_lib_uncaught_exceptions
                if(!std::uncaught_exceptions()) throw;
                #else
                if(!std::uncaught_exception()) throw;
                #endif
                std::cerr << "Unlocker::operator() caught std::exception in call of DynamicValueLock::Unlock()"
                             "during stack unwinding, it won't be rethrown. std::exception::what(): "
                          << e.what() << std::endl;
            }
        }
    };

    /**
     * @class UnlockerAll
     * 
     * @brief Same as ValueLock::UnlockerAll,
     *        but for FakeValueLock
     * 
    */
    class UnlockerAll
    { 
        bool haveKeepLockedValue = false;
    public:
        UnlockerAll() = default;
        DECLARE_RULE_OF_5_DEFAULT(UnlockerAll, NOTHING);
        explicit UnlockerAll(const ValueType& keepLockedValue) 
            : haveKeepLockedValue(true) {}

        /**
         * @throws 
         * Same exception as ValueLock::UnlockAll([value]) if there is no stack unwinding,
         * otherwise printing error message to std::cerr and returns
         */
        inline void operator()(FakeValueLock* pValueLock) const
        {
            try 
            {
                if(haveKeepLockedValue) 
                    pValueLock->UnlockAll(ValueType{});
                else 
                    pValueLock->UnlockAll();
            }
            catch(const std::exception& e)
            {
                #ifdef __cpp_lib_uncaught_exceptions
                if(!std::uncaught_exceptions()) throw;
                #else
                if(!std::uncaught_exception()) throw;
                #endif
                std::cerr << "UnlockerAll::operator() caught std::exception in call of DynamicValueLock::UnlockAll()"
                             "during stack unwinding, it won't be rethrown. std::exception::what(): "
                          << e.what() << std::endl;
            }
        }
    };
    
    // Move only non-virtual
    DECLARE_COPY_DELETE(FakeValueLock);
    DECLARE_MOVE_DEFAULT(FakeValueLock, NOTHING);

    FakeValueLock() = default;

    void Lock(const ValueType& value) noexcept(false) {  m_mtx.lock(); }
    void LockAll() noexcept(false) { m_mtx.lock(); }

    void Unlock(const ValueType& value)  noexcept(false) { m_mtx.unlock(); }
    void UnlockAll() noexcept { m_mtx.unlock(); }
    void UnlockAll(const ValueType& keepLockedValue) noexcept(false) {}

    bool TryLock(const ValueType& value)  noexcept(false) { return m_mtx.try_lock(); }

private:
    std::mutex m_mtx;
};


template<typename ValueT>
class DynamicValueLock
{
public:

    static_assert(std::is_default_constructible<ValueT>::value, "ValueT must be default constructible");
    static_assert(     is_equality_comparable<ValueT>::value, "ValueT must has equality operator overloaded");
    static_assert(std::is_copy_constructible<ValueT>::value, "ValueT must be copy constructible");
    static_assert(std::is_copy_assignable<ValueT>::value, "ValueT must be copy assignable");

    using ValueType = ValueT;

    struct ValueMutex
    {
        ValueMutex() = default;
        DECLARE_COPY_DELETE(ValueMutex);
        explicit ValueMutex(const ValueType& val) : Value(val) {}
        ValueMutex(ValueMutex&& rvalRef)
            : Value(std::move(rvalRef.Value)),
            RefCount(std::move(rvalRef.RefCount)) {}

        ValueMutex& operator=(ValueMutex&& rvalRef)
        {
            Value = std::move(rvalRef.Value);
            RefCount = std::move(rvalRef.RefCount);
            return *this;
        }

        std::mutex Mutex;
        ValueType Value;
        uint32_t RefCount = 0;
    };

    using Container = std::list<ValueMutex>;


    /**
     * @class Unlocker
     * 
     * @brief Unary functor that calls Unlock(value)
     *        on given pointer to DynamicValueLock
     *        with value.
     * 
     * @warning 
     * Invoking throws the same exception as DynamicValueLock::Unlock(value) if there is no stack unwinding,
     * otherwise printing error message to std::cerr and returns
     * 
    */
    class Unlocker
    {
        ValueType m_Value;
    public:
        Unlocker() = default;
        DECLARE_RULE_OF_5_DEFAULT(Unlocker, NOTHING);
        explicit Unlocker(const ValueType& value) : m_Value(value) {}

        /**
         * @throws 
         * Same exception as DynamicValueLock::Unlock(value) if there is no stack unwinding,
         * otherwise printing error message to std::cerr and returns
         */
        inline void operator()(DynamicValueLock* pValueLock) const
        {
            try { pValueLock->Unlock(m_Value); }
            catch(const std::exception& e)
            {
                #ifdef __cpp_lib_uncaught_exceptions
                if(!std::uncaught_exceptions()) throw;
                #else
                if(!std::uncaught_exception()) throw;
                #endif
                std::cerr << "Unlocker::operator() caught std::exception in call of DynamicValueLock::Unlock()"
                             "during stack unwinding, it won't be rethrown. std::exception::what(): "
                          << e.what() << std::endl;
            }
        }
    };

    /**
     * @class UnlockerAll
     * 
     * @brief Unary functor that calls UnlockAll([value])
     *        on given pointer to DynamicValueLock
     *        with value.
     * 
     * @warning 
     * Invoking throws the same exception as DynamicValueLock::UnlockAll([value]) if there is no stack unwinding,
     * otherwise printing error message to std::cerr and returns
     * 
    */
    class UnlockerAll
    { 
        std::unique_ptr<ValueType> m_upKeepLockedValue;
    public:
        UnlockerAll() = default;
        DECLARE_RULE_OF_5_DEFAULT(UnlockerAll, NOTHING);
        explicit UnlockerAll(const ValueType& keepLockedValue) 
            : m_upKeepLockedValue(new ValueType(keepLockedValue)) {}

        /**
         * @throws 
         * Same exception as DynamicValueLock::UnlockAll([value]) if there is no stack unwinding,
         * otherwise printing error message to std::cerr and returns
         */
        inline void operator()(DynamicValueLock* pValueLock) const
        {
            try 
            {
                if(m_upKeepLockedValue) 
                    pValueLock->UnlockAll(*m_upKeepLockedValue);
                else 
                    pValueLock->UnlockAll();
            }
            catch(const std::exception& e)
            {
                #ifdef __cpp_lib_uncaught_exceptions
                if(!std::uncaught_exceptions()) throw;
                #else
                if(!std::uncaught_exception()) throw;
                #endif
                std::cerr << "UnlockerAll::operator() caught std::exception in call of DynamicValueLock::UnlockAll()"
                             "during stack unwinding, it won't be rethrown. std::exception::what(): "
                          << e.what() << std::endl;
            }
        }
    };



    
    // Move only non-virtual
    DECLARE_COPY_DELETE(DynamicValueLock);
    DECLARE_MOVE_DEFAULT(DynamicValueLock, NOTHING);

    DynamicValueLock() = default;

    void Lock(const ValueType& value) noexcept(false)
    {
        std::unique_lock<std::mutex> uLock(m_mtx);
        m_cvLockAllWaiter.wait(uLock, [this]{ return !m_bIsLockingAll; });
        auto iterMutex = FindSlot(value);
        if(iterMutex == m_listValueMutexes.end())
            iterMutex = TakeSlot(value);
        else
            ++(iterMutex->RefCount);
        uLock.unlock();
        iterMutex->Mutex.lock();
    }
    
    /**
     * @brief Locks every slot/value
     */
    void LockAll() noexcept(false)
    {
        std::unique_lock<std::mutex> uLock(m_mtx);
        m_cvLockAllWaiter.wait(uLock, [this]{ return !m_bIsLockingAll; });
        m_bIsLockingAll = true;
        m_cvEmptyListWaiter.wait(uLock, [this]{ return m_listValueMutexes.empty(); });
    }


    /**
     * @brief Unlocks given value.
     * 
     * @param value is value to unlock
     * 
     * @exception - Same as std::mutex::lock(): can be thrown 
     *              by inner std::mutex (rare case)
     * 
     * @warning DynamicValueLock::Lock(value) must
     * be called  by the current thread of execution, 
     * otherwise, the behavior is undefined.
     * 
    */
    void Unlock(const ValueType& value)  noexcept(false)
    {
        std::unique_lock<std::mutex> uLock(m_mtx);
        auto iterMutex = FindSlot(value);
        NICKSV_ASSERT(iterMutex != m_listValueMutexes.end(), INVALID_VALUE_ERROR_TEXT);
        LeaveSlotAndUnlock(iterMutex);
        if(m_listValueMutexes.empty())
            m_cvEmptyListWaiter.notify_one();
    }

    /**
     * @brief Unlocks all values.
     * 
     * @warning DynamicValueLock::LockAll() must
     * be called  by the current thread of execution, 
     * otherwise, the behavior is undefined.
     * 
    */
    void UnlockAll() noexcept
    {
        NICKSV_ASSERT(m_bIsLockingAll, CONCURRENCY_ERROR_TEXT);
        NICKSV_ASSERT(m_listValueMutexes.empty(), 
            "m_listValueMutexes not empty at UnlockAll() call, probably DynamicValueLock implementation is broken");
        m_bIsLockingAll = false;
        m_cvLockAllWaiter.notify_all();
    }


    /**
     * @brief Unlocks all values except given one.
     * 
     * @details
     * For noexcept verison see UnlockAllNoExcept().
     * 
     * @param keepLockedValue value to keep locked
     * 
     * @exception - Same as std::mutex::lock(): can be thrown 
     *              by inner std::mutex (rare case)
     * 
     * @warning DynamicValueLock::LockAll() must
     * be called  by the current thread of execution, 
     * otherwise, the behavior is undefined.
     * 
    */
    void UnlockAll(const ValueType& keepLockedValue) noexcept(false)
    {
        NICKSV_ASSERT(m_bIsLockingAll, CONCURRENCY_ERROR_TEXT);
        NICKSV_ASSERT(m_listValueMutexes.empty(), 
            "m_listValueMutexes not empty at UnlockAll(value) call, probably DynamicValueLock implementation is broken");
        TakeSlot(keepLockedValue)->Mutex.lock();
        m_bIsLockingAll = false;
        m_cvLockAllWaiter.notify_all();
    }

    bool TryLock(const ValueType& value)  noexcept(false)
    {
        std::unique_lock<std::mutex> uLock(m_mtx);
        m_cvLockAllWaiter.wait(uLock, [this]{ return !m_bIsLockingAll; });
        auto iterMutex = FindSlot(value);
        if(iterMutex == m_listValueMutexes.end())
            iterMutex = TakeSlot(value);
        else
            ++(iterMutex->RefCount);
        auto isLocked = iterMutex->Mutex.try_lock();
        if(!isLocked) 
            LeaveSlot(iterMutex);
        return isLocked;
    }

private:
    inline auto FindSlot(const ValueType& value) noexcept -> typename Container::iterator
    {
        return std::find_if(m_listValueMutexes.begin(), m_listValueMutexes.end(), 
                [&value](const ValueMutex & mutex) noexcept { return value == mutex.Value; });
    }
    inline auto TakeSlot(const ValueType& value) -> typename Container::iterator
    {
        m_listValueMutexes.push_back(ValueMutex(value));
        auto iter = m_listValueMutexes.end();
        --iter; ++iter->RefCount;
        return iter;
    }
    bool LeaveSlot(typename Container::iterator iter)
    {
        NICKSV_ASSERT(iter->RefCount, "Leaving ValueMutex slot with RefCount == 0, probably DynamicValueLock implementation is broken");
        bool erase = !(--(iter->RefCount));
        if(erase) 
            m_listValueMutexes.erase(iter);
        return erase;
    }
    inline bool LeaveSlotAndUnlock(typename Container::iterator iter)
    {
        iter->Mutex.unlock();
        return LeaveSlot(iter);
    }
    std::list<ValueMutex> m_listValueMutexes;
    std::mutex m_mtx;
    std::condition_variable m_cvLockAllWaiter;
    std::condition_variable m_cvEmptyListWaiter;
    volatile bool m_bIsLockingAll = false;
};


template<typename LockType>
struct is_value_lock : std::false_type {};

template<typename ValueT, size_t threadCount>
struct is_value_lock<ValueLock<ValueT, threadCount>> : std::true_type {};

template<typename ValueT, size_t threadCount>
struct is_value_lock<FakeValueLock<ValueT, threadCount>> : std::true_type {};

template<typename ValueT>
struct is_value_lock<DynamicValueLock<ValueT>> : std::true_type {};

#ifdef __cpp_variable_templates
template<typename LockType>
static constexpr bool is_value_lock_v = is_value_lock<LockType>::value;
#endif


template<typename LockT>
class ValueLockGuard final
{
public:
    static_assert(is_value_lock<LockT>::value, "LockT should be ValueLock/DynamicValueLock/FakeValueLock");
    using LockType = LockT;
    using ValueType = typename LockType::ValueType;

    ValueLockGuard() = delete;
    DECLARE_RULE_OF_5_DELETE(ValueLockGuard);
    
    ValueLockGuard(LockType& lock, const ValueType& value) :
        m_rLock(lock), m_value(value) { m_rLock.Lock(m_value);}
    ~ValueLockGuard() { typename LockType::Unlocker{m_value}(&m_rLock); }
private:
    LockType& m_rLock;
    const ValueType m_value;
};


template<typename LockT>
class ValueLockAllGuard final
{
public:
    using LockType = LockT;
    using ValueType = typename LockType::ValueType;
    

    ValueLockAllGuard() = delete;
    DECLARE_RULE_OF_5_DELETE(ValueLockAllGuard);
    
    explicit ValueLockAllGuard(LockType& lock) noexcept(false) : m_rLock(lock)
    { 
        m_rLock.LockAll();
    }

    ValueLockAllGuard(LockType& lock, const ValueType& keepLockedValue) noexcept(false) 
        : m_rLock(lock), m_upKeepLockedValue(new ValueType(keepLockedValue))
    {
        m_rLock.LockAll();
    }
    
    ~ValueLockAllGuard()
    {
        if(m_upKeepLockedValue) 
            typename LockType::UnlockerAll{*m_upKeepLockedValue}(&m_rLock);
        else 
            typename LockType::UnlockerAll{}(&m_rLock);
    }

    void SetKeepLockedValue(const ValueType& keepLockedValue) noexcept
    {
        m_upKeepLockedValue = std::unique_ptr<ValueType>(new ValueType(keepLockedValue));
    }

private:
    LockType& m_rLock;
    std::unique_ptr<ValueType> m_upKeepLockedValue;
};


}}  /*END OF NAMESPACES*/




#endif // _NICKSV_VALUELOCK