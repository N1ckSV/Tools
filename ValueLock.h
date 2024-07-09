
#ifndef _NICKSV_VALUELOCK
#define _NICKSV_VALUELOCK
#pragma once


#include "Definitions.h"
#include "Utils.h"

#include <mutex>
#include <type_traits>
#include <stdexcept>



namespace NickSV {
namespace Tools {

/** 
 * @class DefaultMatchException
 * 
 * @brief Thrown by ValueLock's methods.
 * 
 * Thrown when value matches defaultValue.
*/
struct DefaultMatchException : std::invalid_argument
{
    DefaultMatchException() : std::invalid_argument(
        "Value matches defaultValue") {};
};

/** 
 * @class ValueException
 * 
 * @brief Thrown by ValueLock's methods.
 * 
 * Thrown when value has not yet been locked.
*/
struct ValueException : std::invalid_argument
{
    ValueException() : std::invalid_argument(
        "Value has not yet been locked") {};
};

/** 
 * @class ConcurrencyException
 * 
 * @brief Thrown by ValueLock's methods.
 * 
 * Thrown when one of two things happened:
 * 1) too many threads are locking ValueLock object,
 *    change threadCount to how many of them are potentially
 *    can simultaneously lock a value in ValueLock;
 * 
 * 2) one of the threads locked two or more values
 *    in one ValueLock object at the same time 
 *    (which is not allowed, see LockAll() for total lock)
 *    and current thread did not find a free slot to lock its value
 *    and threw this exception.
*/
struct ConcurrencyException : std::runtime_error
{
    ConcurrencyException() : std::runtime_error("Invalid function call: all ValueLock slots are busy") {};
};

/**
 * @class ValueLock
 * 
 * @brief This class helps to control OBJECTS
 *        that are binded to some Value
 *        across multiple threads.
 * 
 * @tparam ValueT type of value to lock
 * @tparam threadCount how many threads are
 *         handling OBJECTS binded to ValueT
 * @tparam defaultValue unresolved value that
 *         are typically not used in these OBJECTS
 *         (e.g 0, NULL, nullptr, '\0').
 *         ValueT{} by default.
 *
 * 
 *  Imagine you have std::map<UserID, User> mapUsers,
 *  so mapUsers is OBJECT listed above and UserID is ValueT.
 *  And multiple (threadCount) threads can change mapUsers 
 *  by adding, removing, changing Users.
 *  So the first thought that came to my mind was to make 
 *  std::map<UserID, std::mutex> mapMutexes and
 *  lock needed UserID before changing mapUsers:
 *  @code{.cpp} 
 *      mapMutexes[id].lock();
 *      mapUsers[id].doSomething();
 *      mapMutexes[id].unlock();
 *  @endcode
 *  But this is too heavy, especially when you need to lock
 *  "all" UserIDs. So ValueLock helps in this situation:
 *  @code{.cpp}
 *      // ValueLock<UserID, threadsCount> usersLock declared before
 *      // with lifetime is about the same as mapUsers
 *      usersLock.Lock(id);
 *      mapUsers[id].doSomething();
 *      usersLock.Unlock(id);
 *  @endcode
 *  or RAII-style
 *  @code{.cpp}
 *  // ValueLock<UserID, threadsCount> usersLock declared before
 *  // with lifetime is about the same as mapUsers
 *  {
 *      // Same work as std::lock_guard
 *      ValueLockGuard<decltype(usersLock)> lockGuard(usersLock, id);
 *      mapUsers[id].doSomething();
 *  }
 *  @endcode
 *  More info in methods description.
 * 
 * 
 *  @todo add link to pictures with ValueLock resolved coding
*/
template<typename ValueT, size_t threadCount, ValueT defaultValue = ValueT{}>
class ValueLock
{
public:
    using ValueType = ValueT;

    static_assert(     is_equality_comparable<ValueType>::value, "ValueType must has equality operator overloaded");
    static_assert(std::is_copy_assignable<ValueType>::value, "ValueType must be copy assignable");


    /**
     * @class Unlocker
     * 
     * @brief Functor that calls Unlock(value)
     *        on given pointer to ValueLock
     *        with value.
     * 
     * Useful when applied with RAII-style:
     *  std::unique_ptr<ValueLock, ValueLock::Unlocker>
     * 
    */
    class Unlocker
    {
        static_assert(noexcept(std::declval<ValueLock>().UnlockNoExcept(std::declval<ValueType>())), 
            "UnlockNoExcept is not noexcept");
        ValueType m_Value = defaultValue;
    public:
        Unlocker() = default;
        DECLARE_RULE_OF_5_DEFAULT(Unlocker, NOTHING);
        explicit Unlocker(const ValueType& value) : m_Value(value) {}
        inline void operator()(ValueLock* pValueLock) const noexcept
        {
            if(m_Value != defaultValue)
                pValueLock->UnlockNoExcept(m_Value);
        }
    };


    /**
     * @class AllUnlocker
     * 
     * @brief Functor that conditionally calls
     *        UnlockAll() or UnlockAll(value)
     *        on given pointer to ValueLock.
     * 
     * Useful when applied with RAII-style:
     *  std::unique_ptr<ValueLock, ValueLock::AllUnlocker>
     * 
    */
    class AllUnlocker
    {
        static_assert(noexcept(std::declval<ValueLock>().UnlockAll()), "UnlockAll is not noexcept");
        static_assert(noexcept(std::declval<ValueLock>().UnlockAllNoExcept(std::declval<ValueType>())),
            "UnlockAllNoExcept is not noexcept");
        ValueType m_keepLockedValue = defaultValue;
    public:
        AllUnlocker() = default;
        DECLARE_RULE_OF_5_DEFAULT(AllUnlocker, NOTHING);
        explicit AllUnlocker(const ValueType& keepLockedValue) : m_keepLockedValue(keepLockedValue) {}
        inline void operator()(ValueLock* pValueLock) const noexcept
        {
            if(m_keepLockedValue == defaultValue)
                pValueLock->UnlockAll();
            else
                pValueLock->UnlockAllNoExcept(m_keepLockedValue);
        }
    };
    
    // Move only non-virtual
    DECLARE_COPY_DELETE(ValueLock);
    DECLARE_MOVE_DEFAULT(ValueLock, NOTHING);

    ValueLock() = default;

    void Lock(const ValueType& value) noexcept(false)
    {
        if(value == defaultValue)
            throw DefaultMatchException();

        ValueMutex* pMutexToLock = m_aValueMutexes;
        ValueMutex const * const pEnd = m_aValueMutexes + threadCount;
        ValueMutex* pDefaultMutex = nullptr;
        {   // lock_guard scope begin
            std::lock_guard<std::mutex> lock_g(m_mtx); 
            for (; pMutexToLock < pEnd; ++pMutexToLock)
            {
                if(value == pMutexToLock->Value) break;
                if(defaultValue == pMutexToLock->Value && pDefaultMutex == nullptr) pDefaultMutex = pMutexToLock;
            }
            if(pMutexToLock == pEnd)
            {
                if(pDefaultMutex == nullptr)
                    throw ConcurrencyException();

                pDefaultMutex->Value = value;
                pMutexToLock = pDefaultMutex;
            }
            ++pMutexToLock->RefCount;

        }   // lock_guard scope end
        pMutexToLock->Mutex.lock();
    }
    // Locks every slot/value
    //
    // THROWS: 
    // the same exception that std::mutex::Lock() throws and
    // unlocks everything that was successfully locked.
    void LockAll() noexcept(false)
    {
        for_each_exception_safe(m_aValueMutexes, m_aValueMutexes + threadCount,
        [](ValueMutex& mut) { mut.Mutex.lock(); }, 
        [](ValueMutex& mut) noexcept { mut.Mutex.unlock(); });
    }
    // Locks every slot/value in ValueLock.
    // Use it only if you are locked exactly one value already
    // (that you should specify by alreadyLockedValue param).
    //
    // THROWS:
    // the same exception that std::mutex::Lock() throws and
    // unlocks everything that was successfully locked.
    void LockAll(const ValueType& alreadyLockedValue) noexcept(false)
    {
        if(alreadyLockedValue == defaultValue)
            throw DefaultMatchException();

        ValueMutex * pIgnoreMutex = nullptr;
        {   // lock_guard scope begin
            std::lock_guard<std::mutex> lock_g(m_mtx); 
            for (ValueMutex* pVar = m_aValueMutexes; pVar < m_aValueMutexes + threadCount; ++pVar)
            {
                if(pVar->Value != alreadyLockedValue)
                    continue;
                // In debug mode checks all values and if at least two of them equals alreadyLockedValue - assertion failed.
                // In non debug mode if alreadyLockedValue found - for-loop breaks
                #if defined(NDEBUG)
                    //cppcheck-suppress unmatchedSuppression
                    //cppcheck-suppress incorrectStringBooleanError
                    NICKSV_ASSERT(!pIgnoreMutex,
                        "alreadyLockedValue found in two slots, probably ValueLock implementation is broken");
                    pIgnoreMutex = pVar;
                #else 
                    pIgnoreMutex = pVar;
                    break;
                #endif
            }
            if(!pIgnoreMutex)
                throw ValueException();
            pIgnoreMutex->Mutex.unlock();
        }   // lock_guard scope end
        LockAll();
    }

    /**
     * @brief Unlocks given value.
     * 
     * For noexcept verison see UnlockNoExcept().
     * 
     * @param value is value to unlock
     * 
     * @exception - @ref DefaultMatchException : value == defaultValue;
     * @exception - @ref ValueException : value has not yet been locked;
     * @exception - Same as std::mutex::lock(): can be thrown 
     *              by inner std::mutex (rare case)
    */
    void Unlock(const ValueType& value)  noexcept(false)
    {
        if(value == defaultValue)
            throw DefaultMatchException();

        ValueMutex * pMutexToUnlock = nullptr;
        ValueMutex const * const pEnd = m_aValueMutexes + threadCount;
        {   // lock_guard scope begin
            std::lock_guard<std::mutex> lock_g(m_mtx);
            for (ValueMutex * pVar = m_aValueMutexes; pVar < pEnd; ++pVar) 
            {
                if(pVar->Value != value)
                    continue;
                // In debug mode checks all values and if at least two of them equals parameter value - assertion failed.
                // In non debug mode if parameter value found - for-loop breaks
                #if defined(NDEBUG)
                    //cppcheck-suppress unmatchedSuppression
                    //cppcheck-suppress incorrectStringBooleanError
                    NICKSV_ASSERT(!pMutexToUnlock, 
                        "value found in two slots, probably ValueLock implementation is broken");
                    pMutexToUnlock = pVar;
                #else 
                    pMutexToUnlock = pVar;
                    break;
                #endif
            }

            if(pMutexToUnlock == pEnd)
                throw ValueException();
            
            NICKSV_ASSERT(pMutexToUnlock->RefCount, 
                "Unlock Value found in slots but RefCount is 0, probably ValueLock implementation is broken");
            --(pMutexToUnlock->RefCount);
            if(pMutexToUnlock->RefCount == 0)
                pMutexToUnlock->Value = defaultValue;
            pMutexToUnlock->Mutex.unlock();
        }   // lock_guard scope end
    }

    /**
     * @brief Unlocks given value without exceptions.
     * 
     * Doing nothing at conditions when Unlock() throws.
     * 
     * @param value value to unlock
     * 
     * @warning - Rare potential program termination:
     *            exception may be thrown by inner 
     *            std::mutex and the program supposed
     *            to terminate in this case
     * 
     * @todo maybe noexcept is redundant here
    */
    void UnlockNoExcept(const ValueType& value) noexcept
    {
        if(value == defaultValue)
            return;

        ValueMutex * pMutexToUnlock = nullptr;
        ValueMutex const * const pEnd = m_aValueMutexes + threadCount;
        {   // lock_guard scope begin
            std::lock_guard<std::mutex> lock_g(m_mtx);  // if m_mtx throws - prog termination 
            for (ValueMutex * pVar = m_aValueMutexes; pVar < pEnd; ++pVar) 
            {
                if(pVar->Value != value)
                    continue;
                // In debug mode checks all values and if at least two of them equals parameter value - assertion failed.
                // In non debug mode if parameter value found - for-loop breaks
                #if defined(NDEBUG)
                    //cppcheck-suppress unmatchedSuppression
                    //cppcheck-suppress incorrectStringBooleanError
                    NICKSV_ASSERT(!pMutexToUnlock, 
                        "value found in two slots, probably ValueLock implementation is broken");
                    pMutexToUnlock = pVar;
                #else 
                    pMutexToUnlock = pVar;
                    break;
                #endif
            }

            if(pMutexToUnlock == pEnd)
                return;
            
            //cppcheck-suppress incorrectStringBooleanError
            NICKSV_ASSERT(pMutexToUnlock->RefCount, 
                "Unlock Value found in slots but RefCount is 0, probably ValueLock implementation is broken");
            --(pMutexToUnlock->RefCount);
            if(pMutexToUnlock->RefCount == 0)
                pMutexToUnlock->Value = defaultValue;
            pMutexToUnlock->Mutex.unlock();
        }   // lock_guard scope end
    }

    /**
     * @brief Unlocks all values.
     * 
    */
    void UnlockAll() noexcept
    {
        for (ValueMutex* pVar = m_aValueMutexes; pVar < m_aValueMutexes + threadCount; ++pVar)
            pVar->Mutex.unlock();
    }

    /**
     * @brief Unlocks all values except given one.
     * 
     * For noexcept verison see UnlockAllNoExcept().
     * 
     * @param keepLockedValue value to keep locked
     * 
     * @exception - @ref DefaultMatchException : value == defaultValue, thrown without unlocking;
     * @exception - @ref ConcurrencyException : thrown after unlocking every value;
     * @exception - Same as std::mutex::lock(): can be thrown 
     *              by inner std::mutex (rare case)
     * 
    */
    void UnlockAll(const ValueType& keepLockedValue) noexcept(false)
    {
        if(keepLockedValue == defaultValue)
            throw DefaultMatchException();

        ValueMutex const * pKeepLockedMutex = nullptr;
        ValueMutex * pDefaultMutex = nullptr;
        {   // lock_guard scope begin
            std::lock_guard<std::mutex> lock_g(m_mtx); 
            for (ValueMutex* pVar = m_aValueMutexes; pVar < m_aValueMutexes + threadCount; ++pVar)
            {
                if(!pDefaultMutex && !pKeepLockedMutex && (pVar->Value == defaultValue)) {
                    pDefaultMutex = pVar;
                    continue; }

                if(pVar->Value != keepLockedValue) {
                    pVar->Mutex.unlock();
                    continue; }
                
                NICKSV_ASSERT(!pKeepLockedMutex, "keepLockedValue found in two slots, probably ValueLock implementation is broken");
                pKeepLockedMutex = pVar;
            }
            if(pDefaultMutex && pKeepLockedMutex)
                pDefaultMutex->Mutex.unlock();
            else if(pDefaultMutex && !pKeepLockedMutex)
            {
                pDefaultMutex->Value = keepLockedValue;
                ++(pDefaultMutex->RefCount);
            }
            else if (!pKeepLockedMutex)
                throw ConcurrencyException();
        }   // lock_guard scope end
    }

    /**
     * @brief Unlocks all values except given one without exceptions.
     * 
     * @param keepLockedValue value to keep locked
     * 
     * @warning - Rare potential program termination:
     *            exception may be thrown by inner 
     *            std::mutex and the program supposed
     *            to terminate in this case
     *
     * @todo maybe noexcept is redundant here
    */
    void UnlockAllNoExcept(const ValueType& keepLockedValue) noexcept
    {
        if(keepLockedValue == defaultValue)
            return;

        ValueMutex const * pKeepLockedMutex = nullptr;
        ValueMutex * pDefaultMutex = nullptr;
        {   // lock_guard scope begin
            std::lock_guard<std::mutex> lock_g(m_mtx); 
            for (ValueMutex* pVar = m_aValueMutexes; pVar < m_aValueMutexes + threadCount; ++pVar)
            {
                if(!pDefaultMutex && !pKeepLockedMutex && (pVar->Value == defaultValue)) {
                    pDefaultMutex = pVar;
                    continue; }

                if(pVar->Value != keepLockedValue) {
                    pVar->Mutex.unlock();
                    continue; }
                
                NICKSV_ASSERT(!pKeepLockedMutex, "keepLockedValue found in two slots, probably ValueLock implementation is broken");
                pKeepLockedMutex = pVar;
            }
            if(pDefaultMutex && pKeepLockedMutex)
                pDefaultMutex->Mutex.unlock();
            else if(pDefaultMutex && !pKeepLockedMutex)
            {
                pDefaultMutex->Value = keepLockedValue;
                ++(pDefaultMutex->RefCount);
            }
            else if (!pKeepLockedMutex)
                return;
        }   // lock_guard scope end
    }

    bool TryLock(const ValueType& value)  noexcept(false)
    {
        if(value == defaultValue)
            throw DefaultMatchException();

        ValueMutex* pMutexToLock = m_aValueMutexes;
        ValueMutex const * const pEnd = m_aValueMutexes + threadCount;
        ValueMutex* pDefaultMutex = nullptr;
        {   // lock_guard code block begin
            std::lock_guard<std::mutex> lock_g(m_mtx); 
            while(pMutexToLock < pEnd)
            {
                if(value == pMutexToLock->Value) break;
                if(!pDefaultMutex && defaultValue == pMutexToLock->Value) pDefaultMutex = pMutexToLock;
                ++pMutexToLock;
            }
            if(pMutexToLock == pEnd)
            {
                if(!pDefaultMutex)
                    throw ConcurrencyException();

                pDefaultMutex->Value = value;
                pMutexToLock = pDefaultMutex;
            }
            ++(pMutexToLock->RefCount);
            auto result = pMutexToLock->Mutex.try_lock();
            if(!result)
            {
                --pMutexToLock->RefCount;
                if(pMutexToLock->RefCount == 0)
                    pMutexToLock->Value = defaultValue;
            }
            return result;
        }   // lock_guard code block end
    }
    
    inline static constexpr size_t GetSlotsCount() { return threadCount; }

    inline static constexpr ValueType GetDefaultValue() { return defaultValue; }

private:
    struct ValueMutex
    {
        // Move only
        DECLARE_COPY_DELETE(ValueMutex);
        DECLARE_MOVE_DEFAULT(ValueMutex, NOTHING);
        ValueMutex() = default;
        std::mutex Mutex;
        ValueType Value = defaultValue;
        uint32_t RefCount = 0;
    };
    ValueMutex m_aValueMutexes[threadCount];
    std::mutex m_mtx;
};


/**
 * @class FakeValueLock
 * 
 * @brief Same as @ref ValueLock but working like a single mutex.
 *        Was made for benchmarking purpose.
 * 
*/
template<typename ValueT, size_t threadCount, ValueT defaultValue>
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
        explicit Unlocker(const ValueType&) : Unlocker() {}
        inline void operator()(FakeValueLock* pFakeValueLock) const noexcept
        {
            pFakeValueLock->UnlockNoExcept(defaultValue);
        }
    };

    /**
     * @class AllUnlocker
     * 
     * @brief Same as ValueLock::AllUnlocker,
     *        but for FakeValueLock
    */
    class AllUnlocker
    {
        ValueType m_keepLockedValue = defaultValue;
    public:
        AllUnlocker() = default;
        DECLARE_RULE_OF_5_DEFAULT(AllUnlocker, NOTHING);
        explicit AllUnlocker(const ValueType& keepLockedValue) : m_keepLockedValue(keepLockedValue) {}
        inline void operator()(FakeValueLock* pFakeValueLock) const noexcept
        {
            if(m_keepLockedValue == defaultValue)
                pFakeValueLock->UnlockAll();
        }
    };
    
    // Move only non-virtual
    DECLARE_COPY_DELETE(FakeValueLock);
    DECLARE_MOVE_DEFAULT(FakeValueLock, NOTHING);

    FakeValueLock() = default;

    void Lock(const ValueType& value) noexcept(false) {  m_mtx.lock(); }
    void LockAll()   noexcept(false) { m_mtx.lock(); }
    void LockAll(const ValueType& alreadyLockedValue)  noexcept(false) {}

    void Unlock(const ValueType& value)  noexcept(false) { m_mtx.unlock(); }
    void UnlockNoExcept(const ValueType& value)  noexcept { m_mtx.unlock(); }
    void UnlockAll() noexcept { m_mtx.unlock(); }
    void UnlockAll(const ValueType& keepLockedValue) noexcept(false) {}
    void UnlockAllNoExcept(const ValueType& keepLockedValue) noexcept {}

    bool TryLock(const ValueType& value)  noexcept(false) { return m_mtx.try_lock(); }

    inline static constexpr size_t GetSlotsCount() { return threadCount; }
    inline static constexpr ValueType GetDefaultValue() { return defaultValue; }
private:
    std::mutex m_mtx;
};


template<typename LockType>
struct is_value_lock : std::false_type {};

template<typename ValueT, size_t threadCount, ValueT defValue>
struct is_value_lock<ValueLock<ValueT, threadCount, defValue>> : std::true_type {};

template<typename ValueT, size_t threadCount, ValueT defValue>
struct is_value_lock<FakeValueLock<ValueT, threadCount, defValue>> : std::true_type {};

#if __cplusplus > 201103L
template<typename LockType>
inline static constexpr bool is_value_lock_v = is_value_lock<LockType>::value;
#endif


template<typename LockT>
class ValueLockGuard final
{
public:
    static_assert(is_value_lock<LockT>::value, "LockT should be ValueLock/FakeValueLock");
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
    static_assert(is_value_lock<LockT>::value, "LockT should be ValueLock/FakeValueLock");
    using LockType = LockT;
    using ValueType = typename LockType::ValueType;
    

    ValueLockAllGuard() = delete;
    DECLARE_RULE_OF_5_DELETE(ValueLockAllGuard);
    
    explicit ValueLockAllGuard(LockType& lock) noexcept(false) : m_rLock(lock)
    { 
        m_rLock.LockAll();
    }
    // alreadyLockedValue can't be ValueLock's defaultValue (exception)
    ValueLockAllGuard(LockType& lock, const ValueType& alreadyLockedValue) noexcept(false) :
        m_rLock(lock), m_alreadyLockedValue(alreadyLockedValue) 
    {
        m_rLock.LockAll(m_alreadyLockedValue);
    }

    // alreadyLockedValue and keepLockedValue can't be LockType's defaultValue (exception)
    ValueLockAllGuard(LockType& lock, const ValueType& alreadyLockedValue, const ValueType& keepLockedValue) noexcept(false) :
        m_rLock(lock), m_alreadyLockedValue(alreadyLockedValue), m_keepLockedValue(keepLockedValue) 
    {
        if(m_keepLockedValue == LockType::GetDefaultValue() || 
           m_alreadyLockedValue == LockType::GetDefaultValue())
            throw DefaultMatchException();

        m_rLock.LockAll(m_alreadyLockedValue);
    }
    
    ~ValueLockAllGuard() noexcept
    { 
        typename LockType::AllUnlocker{m_keepLockedValue}(&m_rLock);
    }

    // setting keepLockedValue to LockType::GetDefaultValue() 
    // means that destructor ~ValueLockAllGuard() will unlock all values
    void SetKeepLockedValue(const ValueType& keepLockedValue) noexcept
    {
        m_keepLockedValue = keepLockedValue;
    }

private:
    LockType& m_rLock;
    const ValueType m_alreadyLockedValue = LockType::GetDefaultValue();
    ValueType m_keepLockedValue = LockType::GetDefaultValue();
};


}}  /*END OF NAMESPACES*/




#endif // _NICKSV_VALUELOCK