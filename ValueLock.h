
#ifndef _NICKSV_VALUELOCK
#define _NICKSV_VALUELOCK
#pragma once


#include "Defines.h"
#include "Utils.h"

#include <mutex>
#include <type_traits>
#include <stdexcept>



namespace NickSV::Tools {




template<typename ValueT, size_t threadCount, ValueT defaultValue =  ValueT()>
class ValueLock
{
public:
    using ValueType = ValueT;

    static_assert(     is_equality_comparable<ValueType>::value, "ValueType must has equality operator overloaded");
    static_assert(std::is_copy_assignable<ValueType>::value, "ValueType must be copy assignable");

    struct DefaultMatchException : std::runtime_error
    {
        DefaultMatchException() : std::runtime_error(
            "Invalid parameter: value matches defaultValue") {};
    };
    struct ValueException : std::runtime_error
    {
        ValueException() : std::runtime_error(
            "Invalid parameter: value has not yet been locked") {};
    };
    struct ConcurrencyException : std::runtime_error
    {
        /* 
            One of two things happened:
            1) too many threads are locking ValueLock object,
            change threadCount to how many of them are potentially
            can simultaneously lock a value in ValueLock;
            2) one of the threads locked two or more values
            in one ValueLock object at the same time 
            (which is not allowed, see LockAll() for total lock)
            and current thread did not find a free slot to lock its value
            and threw this exception.
        */
        ConcurrencyException() : std::runtime_error("Invalid function call: all ValueLock slots are busy") {};
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
            LOCK_GUARD(m_mtx);
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
            LOCK_GUARD(m_mtx);
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
        //for_each_exception_safe(m_aValueMutexes, m_aValueMutexes + threadCount,
        //    [pI = pIgnoreMutex](ValueMutex& mut) 
        //    { 
        //        if(&mut != pI)
        //            mut.Mutex.lock();
        //    }, 
        //    [pI = pIgnoreMutex](ValueMutex& mut) noexcept
        //    {
        //        if(&mut != pI)
        //            mut.Mutex.unlock();
        //    });
        
    }
    void Unlock(const ValueType& value)  noexcept(false)
    {
        if(value == defaultValue)
            throw DefaultMatchException();

        ValueMutex * pMutexToUnlock = nullptr;
        ValueMutex const * const pEnd = m_aValueMutexes + threadCount;
        {   // lock_guard scope begin
            LOCK_GUARD(m_mtx);
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
            
            //cppcheck-suppress incorrectStringBooleanError
            NICKSV_ASSERT(pMutexToUnlock->RefCount, 
                "Unlock Value found in slots but RefCount is 0, probably ValueLock implementation is broken");
            --(pMutexToUnlock->RefCount);
            if(pMutexToUnlock->RefCount == 0)
                pMutexToUnlock->Value = defaultValue;
            pMutexToUnlock->Mutex.unlock();
        }   // lock_guard scope end
    }

    void UnlockAll() noexcept
    {
        for (ValueMutex* pVar = m_aValueMutexes; pVar < m_aValueMutexes + threadCount; ++pVar)
            pVar->Mutex.unlock();
    }

    void UnlockAll(const ValueType& keepLockedValue) noexcept(false)
    {
        if(keepLockedValue == defaultValue)
            throw DefaultMatchException();

        ValueMutex const * pKeepLockedMutex = nullptr;
        ValueMutex * pDefaultMutex = nullptr;
        {   // lock_guard scope begin
            LOCK_GUARD(m_mtx);
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

    bool TryLock(const ValueType& value)  noexcept(false)
    {
        if(value == defaultValue)
            throw DefaultMatchException();

        ValueMutex* pMutexToLock = m_aValueMutexes;
        ValueMutex const * const pEnd = m_aValueMutexes + threadCount;
        ValueMutex* pDefaultMutex = nullptr;
        {   // lock_guard code block begin
            LOCK_GUARD(m_mtx);
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

// Same as ValueLock but working like single mutex. Made it for benchmarking purpose
template<typename ValueT, size_t threadCount, ValueT>
class FakeValueLock
{
public:
    using ValueType = ValueT;
    
    // Move only non-virtual
    DECLARE_COPY_DELETE(FakeValueLock);
    DECLARE_MOVE_DEFAULT(FakeValueLock, NOTHING);

    FakeValueLock() = default;
    void Lock(const ValueType& value) noexcept(false) {  m_mtx.lock(); }
    void LockAll()   noexcept(false) { m_mtx.lock(); }
    void LockAll(const ValueType& alreadyLockedValue)  noexcept(false) {}
    void Unlock(const ValueType& value)  noexcept(false) { m_mtx.unlock(); }
    void UnlockAll() noexcept { m_mtx.unlock(); }
    void UnlockAll(const ValueType& keepLockedValue) noexcept(false) {}
    bool TryLock(const ValueType& value)  noexcept(false) { return m_mtx.try_lock(); }
    inline static constexpr size_t GetSlotsCount() { return threadCount; }
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
    ~ValueLockGuard() { m_rLock.Unlock(m_value); }
private:
    LockType& m_rLock;
    ValueType m_value;
};


#define VALUE_LOCK_GUARD(lock, value) NickSV::Tools::ValueLockGuard<typename std::remove_reference<decltype(lock)>::type> \
                                        _generated_by_macro_value_lock_guard(lock, value);


} /*END OF NAMESPACES*/




#endif // _NICKSV_VALUELOCK