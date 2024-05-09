

#include "ValueLock.h"


#include <thread>

#include <benchmark\benchmark.h>


void long_operation(unsigned long long coef)
{
  double sum = 0;
  decltype(coef) count = (coef + 1) * 100000;
  for (unsigned long long i = 0; i < count; ++i)
    sum += cos(sum);
}



template<typename LockType>
void value_lock_example()
{
	using namespace std::chrono;
    const size_t threadC = LockType::GetSlotsCount();
    std::thread threads[threadC];
    LockType vLock;
    typename LockType::ValueType value = 10;
    for (size_t i = 0; i < threadC; ++i)
    {
        threads[i] = std::thread([&vLock, value, i]()
        {
            VALUE_LOCK_GUARD(vLock, value);
            long_operation(2);
        });
    }
    for (size_t i = 0; i < threadC; ++i)
        threads[i].join();
	
	for (size_t i = 0; i < threadC; ++i)
    {
        threads[i] = std::thread([&vLock, i]()
        { 
            VALUE_LOCK_GUARD(vLock, i);
            long_operation(2);;
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}
	srand(time(0));
	for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock, iq, i]()
        { 
            VALUE_LOCK_GUARD(vLock, i);
            long_operation(2);;
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}
}


template<typename LockType>
void value_lock_all_example()
{
	using namespace std::chrono;
    const size_t threadC = LockType::GetSlotsCount();
    std::thread threads[threadC];
    LockType vLock;
    typename LockType::ValueType value = 10;
    srand(time(0));
	for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock, iq, i]()
        {
            {
            VALUE_LOCK_GUARD(vLock, i);
            long_operation(i+1);;
            vLock.LockAll(i);
            long_operation(i+1);;
            vLock.UnlockAll(i);
            long_operation(i+1);;
            }

            {
            VALUE_LOCK_GUARD(vLock, i);
            long_operation(i+1);;
            vLock.LockAll(i);
            long_operation(i+1);;
            vLock.UnlockAll(i);
            long_operation(i+1);;
            }

            {
            VALUE_LOCK_GUARD(vLock, i);
            long_operation(i+1);;
            vLock.LockAll(i);
            long_operation(i+1);;
            vLock.UnlockAll(i);
            long_operation(i+1);;
            }
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}

    for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock,iq, i]()
        {
            vLock.LockAll();
            long_operation(i+1);;
            vLock.UnlockAll(i);
            long_operation(i+1);;
            vLock.Unlock(i);
            long_operation(i+1);;

            vLock.LockAll();
            long_operation(i+1);;
            vLock.UnlockAll(i);
            long_operation(i+1);;
            vLock.Unlock(i);
            long_operation(i+1);;

            vLock.LockAll();
            long_operation(i+1);;
            vLock.UnlockAll(i);
            long_operation(i+1);;
            vLock.Unlock(i);
            long_operation(i+1);;
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}
	
    for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock, iq, i]()
        { 
            vLock.Lock(i);
            long_operation(i+1);;
            vLock.LockAll(i);
            long_operation(i+1);;
            vLock.UnlockAll();
            long_operation(i+1);;

            vLock.Lock(i);
            long_operation(i+1);;
            vLock.LockAll(i);
            long_operation(i+1);;
            vLock.UnlockAll();
            long_operation(i+1);;

            vLock.Lock(i);
            long_operation(i+1);;
            vLock.LockAll(i);
            long_operation(i+1);;
            vLock.UnlockAll();
            long_operation(i+1);;
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}
}

//cppcheck-suppress constParameterCallback
static void BM_ValueLockTime(benchmark::State& state) {
  for (auto a : state)
  {
    value_lock_example<NickSV::Tools::ValueLock<uint32_t, 10, 500>>();
    value_lock_all_example<NickSV::Tools::ValueLock<uint32_t, 10, 500>>();
  }
}

BENCHMARK(BM_ValueLockTime);

//cppcheck-suppress constParameterCallback
static void BM_FakeValueLockTime(benchmark::State& state) {
  for (auto a : state)
  {
    value_lock_example<NickSV::Tools::FakeValueLock<uint32_t, 10, 500>>();
    value_lock_all_example<NickSV::Tools::FakeValueLock<uint32_t, 10, 500>>();
  }
}

BENCHMARK(BM_FakeValueLockTime);


BENCHMARK_MAIN();