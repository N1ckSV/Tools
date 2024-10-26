
#include <iostream>
#include <vector>
#include <thread>
#include <utility>
#include <unordered_set>


#define TEST_IGNORE_PRINT_ON_SUCCESS

#define TEST_SETW_VALUE 65

#define TEST_IGNORE_PRINT_ON_SUCCESS

#include "NickSV/Tools/ValueLock.h"
#include "NickSV/Tools/Testing.h"


constexpr static size_t threadC = 10;

//using VLock = NickSV::Tools::ValueLock<uint32_t, threadC>;
//using DyVLock = NickSV::Tools::DynamicValueLock<uint32_t>;

enum CallType
{
    LOCK,     
    LOCK_ALL,
    UNLOCK_ALL_KEEP,

    UNLOCK_ALL,
    UNLOCK,
};

struct Trace
{
    CallType type;
    uint32_t threadID;
    uint32_t value;
    template<class Arg1, class... Args>
    auto isTypeOneOf(Arg1 arg1, Args... args) const -> std::enable_if_t<std::is_same<Arg1, CallType>::value, bool>
    {
        return  (arg1 == type) || isTypeOneOf(args...);
    }

    template<class Arg1>
    auto isTypeOneOf(Arg1 arg1) const -> std::enable_if_t<std::is_same<Arg1, CallType>::value, bool>
    {
        return  arg1 == type;
    }
};

using Tracer = std::vector<Trace>;

static bool CheckTracing(const Tracer& trace)
{
    std::unordered_set<const Tracer::value_type*> set;
    set.reserve(threadC);
    int soloLockedNow = 0; //has to be zero
    for (auto iter = trace.cbegin(); iter != trace.cend(); ++iter)
    {
        if(soloLockedNow < 0)
            return false;

        if(set.find(&(*iter)) != set.end())
            continue;

        auto next = iter + 1;
        if((next == trace.cend()) ||
            iter->isTypeOneOf(UNLOCK_ALL, UNLOCK))
            return false;
    
        CallType lastType = iter->type;
        if(lastType == LOCK_ALL)
        {
            if(!(next->isTypeOneOf(UNLOCK_ALL, UNLOCK_ALL_KEEP)) ||
                (soloLockedNow != 0) ||
                (iter->threadID != next->threadID) ||
                (iter->value != next->value)) //NOT REALLY NEEDED, because LockAll() doesnt take value
                return false;
            if(next->type == UNLOCK_ALL)
                iter = next;
            continue;
        }
        auto old = soloLockedNow++;
        for (auto iter2 = iter + 1; iter2 != trace.cend(); ++iter2)
        {
            if(iter->value != iter2->value)
            {
                if(iter->threadID == iter2->threadID)
                    return false;
                continue;
            }
            if((iter->threadID != iter2->threadID) ||
               (iter2->type != UNLOCK))
                    return false;
            --soloLockedNow;
            set.insert(&(*iter2));
            break;
        }
        if(old != soloLockedNow)
            return false;
    }
    return true;
}

static int CheckTracingTest(Tracer& tracer)
{
    TEST_CHECK_STAGE(CheckTracing(tracer));

    for (auto& trace: tracer)
    {
        trace.type = static_cast<CallType>(static_cast<int>(trace.type) + 1);
        TEST_CHECK_STAGE(!CheckTracing(tracer));
        trace.type = static_cast<CallType>(static_cast<int>(trace.type) - 1);

        trace.threadID += 1;
        TEST_CHECK_STAGE(!CheckTracing(tracer));
        trace.threadID -= 1;

        trace.value += 1;
        TEST_CHECK_STAGE(!CheckTracing(tracer));
        trace.value -= 1;
    }
    

    return TEST_SUCCESS;
}

template<class LockT>
int VL_test_same_v()
{
	using namespace std::chrono;
    std::thread threads[threadC];
	Tracer vlTrace; 
	vlTrace.reserve(2*threadC);
    LockT vLock;
    uint32_t value = 10;
	srand(static_cast<uint32_t>(time(0)));
    for (uint32_t i = 0; i < threadC; ++i)
    {
        threads[i] = std::thread([&vLock, &vlTrace, value, i]()
        {
            NickSV::Tools::ValueLockGuard<LockT> vLockGuard(vLock, value);
			vlTrace.push_back({LOCK, i, value});
            std::this_thread::sleep_for(milliseconds((rand() % 200) + 50));
			vlTrace.push_back({UNLOCK, i, value});
        });
    }
    for (uint32_t i = 0; i < threadC; ++i)
    {
        threads[i].join();
    }
    
	return CheckTracingTest(vlTrace);
}


template<class LockT>
int VL_test_diff_v()
{
	using namespace std::chrono;
    std::thread threads[threadC];
    LockT vLock;
	Tracer vlTrace; 
	vlTrace.reserve(2*threadC);
	srand(static_cast<uint32_t>(time(0)));
	std::mutex inputMutex;
	for (uint32_t i = 0; i < threadC; ++i)
    {
        threads[i] = std::thread([&vLock, &vlTrace, &inputMutex, i]()
        { 
            NickSV::Tools::ValueLockGuard<LockT> vLockGuard(vLock, i);
            inputMutex.lock();
            vlTrace.push_back({LOCK, i, i});
            inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds((rand() % 200) + 50));
            inputMutex.lock();
            vlTrace.push_back({UNLOCK, i, i});
            inputMutex.unlock();
        });
    }
    for (uint32_t i = 0; i < threadC; ++i)
    {
        threads[i].join();
    }
    return CheckTracingTest(vlTrace);
}

template<class LockT>
int VL_test_rand_v()
{
	using namespace std::chrono;
    std::thread threads[threadC];
    LockT vLock;
	std::mutex inputMutex;
	Tracer vlTrace; 
	vlTrace.reserve(2*threadC);
	srand(static_cast<uint32_t>(time(0)));	
	for (uint32_t iq = 0; iq < threadC; ++iq)
    {
		uint32_t i = static_cast<uint32_t>(rand() % threadC);
        threads[iq] = std::thread([&vLock, &vlTrace, &inputMutex, iq, i]()
        { 
            NickSV::Tools::ValueLockGuard<LockT> vLockGuard(vLock, i);
            inputMutex.lock();
            vlTrace.push_back({LOCK, iq, i});
            inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds((rand() % 200) + 50));
            inputMutex.lock();
            vlTrace.push_back({UNLOCK, iq, i});
            inputMutex.unlock();
        });
    }
    for (uint32_t i = 0; i < threadC; ++i)
    {
        threads[i].join();
    }
    return CheckTracingTest(vlTrace);
}



template<class LockT>
int VL_test_all1()
{
	using namespace std::chrono;
    std::thread threads[threadC];
    LockT vLock;
	std::mutex inputMutex;
	Tracer vlTrace; 
	vlTrace.reserve(3*threadC);
	srand(static_cast<uint32_t>(time(0)));	
    for (uint32_t iq = 0; iq < threadC; ++iq)
    {
		uint32_t i = static_cast<uint32_t>(rand() % threadC);
        threads[iq] = std::thread([&vLock, &vlTrace,&inputMutex, iq, i]()
        {
            vLock.LockAll();
            vlTrace.push_back({LOCK_ALL, iq, i});
            vlTrace.push_back({UNLOCK_ALL_KEEP, iq, i});
            vLock.UnlockAll(i);
            std::this_thread::sleep_for(milliseconds((rand() % 200) + 50));
            inputMutex.lock();
            vlTrace.push_back({UNLOCK, iq, i});
            inputMutex.unlock();
            vLock.Unlock(i);
        });
    }
    for (uint32_t i = 0; i < threadC; ++i)
    {
        threads[i].join();
    }
	return CheckTracingTest(vlTrace);
}


template<class LockT>
int VL_test_all2()
{
	using namespace std::chrono;
    std::thread threads[threadC];
    LockT vLock;
	Tracer vlTrace; 
	vlTrace.reserve(2*threadC);
	srand(static_cast<uint32_t>(time(0)));	
    for (uint32_t iq = 0; iq < threadC; ++iq)
    {
		uint32_t i = static_cast<uint32_t>(rand() % threadC);
        threads[iq] = std::thread([&vLock, &vlTrace, iq, i]()
        {
            vLock.LockAll();
            vlTrace.push_back({LOCK_ALL, iq, i});
            vlTrace.push_back({UNLOCK_ALL, iq, i});
            vLock.UnlockAll();
        });
    }
    for (uint32_t i = 0; i < threadC; ++i)
    {
        threads[i].join();
    }
	return CheckTracingTest(vlTrace);
}


int main()
{
    using namespace NickSV::Tools;
    Tracer testTracer = { //VALID TRACER
        {LOCK,8,6}, {LOCK,1,0}, {UNLOCK,1,0}, {UNLOCK,8,6},  {LOCK_ALL,0,2}, {UNLOCK_ALL,0,2}, 
        {LOCK_ALL,5,4}, {UNLOCK_ALL_KEEP,5,4},  {UNLOCK,5,4},  {LOCK_ALL,0,1}, {UNLOCK_ALL,0,1},
        {LOCK,5,7}, {UNLOCK,5,7}, {LOCK,0,3}, {UNLOCK,0,3}, 
        {LOCK_ALL,1,0}, {UNLOCK_ALL,1,0}, {LOCK_ALL,2,9}, {UNLOCK_ALL,2,9}, {LOCK_ALL,5,8}, {UNLOCK_ALL_KEEP,5,8}, {LOCK,8,2}, 
        {UNLOCK,8,2}, {UNLOCK,5,8}
    };

    TEST_VERIFY(CheckTracingTest(testTracer));

    typedef ValueLock<uint32_t, threadC> Value_Lock;

    TEST_VERIFY(VL_test_same_v<Value_Lock>());
    //
    TEST_VERIFY(VL_test_same_v<Value_Lock>());
    //
    TEST_VERIFY(VL_test_same_v<DynamicValueLock<uint32_t>>());
    //
    TEST_VERIFY(VL_test_diff_v<Value_Lock>());
    //
    TEST_VERIFY(VL_test_diff_v<DynamicValueLock<uint32_t>>());
    //
    TEST_VERIFY(VL_test_rand_v<Value_Lock>());
    //
    TEST_VERIFY(VL_test_rand_v<DynamicValueLock<uint32_t>>());
    //
    TEST_VERIFY(VL_test_all1<Value_Lock>());
    //
    TEST_VERIFY(VL_test_all1<DynamicValueLock<uint32_t>>());
    //
    TEST_VERIFY(VL_test_all2<Value_Lock>());
    //
    TEST_VERIFY(VL_test_all2<DynamicValueLock<uint32_t>>());
    
    std::cout << '\n' << Testing::TestsFailed << " subtests failed\n";
    
    return Testing::TestsFailed;
}