

#include "..\ValueLock.h"


#include <thread>
#include <math.h>

#include <benchmark\benchmark.h>

#pragma optimize("g", off)
double  long_operation(unsigned long long coef)
{
  double sum = 0;
  decltype(coef) count = (coef + 1) * 100000;
  for (unsigned long long i = 0; i < count; ++i)
  {
    benchmark::DoNotOptimize(sum += cos(sum));
  }
  benchmark::DoNotOptimize(sum);
  benchmark::ClobberMemory();
  return sum;
}



template<typename LockType, size_t threadC>
double value_lock_example_same_values()
{
	using namespace std::chrono;
    std::thread threads[threadC];
    volatile double sums[threadC];
    double sum = 0;
    LockType vLock;
    typename LockType::ValueType value = 10;
    for (size_t i = 0; i < threadC; ++i)
    {
        threads[i] = std::thread([&vLock, value, i, &sums]()
        {
            NickSV::Tools::ValueLockGuard<std::remove_reference<decltype(vLock)>::type> g(vLock, value);
            benchmark::DoNotOptimize(sums[i] = long_operation(2));
        });
    }
    
    for (size_t i = 0; i < threadC; ++i)
	  {
          threads[i].join();
          benchmark::DoNotOptimize(sum += sums[i]);
	  }
    return sum;
}


template<typename LockType, size_t threadC>
double value_lock_example_different_values()
{
	using namespace std::chrono;
    std::thread threads[threadC];
    volatile double sums[threadC];
    double sum = 0;
    LockType vLock;
    typename LockType::ValueType value = 10;
	
	for (size_t i = 0; i < threadC; ++i)
    {
        threads[i] = std::thread([&vLock, i, &sums]()
        { 
            NickSV::Tools::ValueLockGuard<std::remove_reference<decltype(vLock)>::type> g(vLock, i);
            benchmark::DoNotOptimize(sums[i] += long_operation(2));
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
        benchmark::DoNotOptimize(sum += sums[i]);
	}
  return sum;
}


template<typename LockType, size_t threadC>
double value_lock_example_random_values()
{
	using namespace std::chrono;
    std::thread threads[threadC];
    volatile double sums[threadC];
    double sum = 0;
    LockType vLock;
    typename LockType::ValueType value = 10;
	
	srand(time(0));
	for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock, iq, i, &sums]()
        { 
            NickSV::Tools::ValueLockGuard<std::remove_reference<decltype(vLock)>::type> g(vLock, i);
            benchmark::DoNotOptimize(sums[i] += long_operation(2));
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
        
        benchmark::DoNotOptimize(sum += sums[i]);
	}
  return sum;
}


template<typename LockType, size_t threadC>
double value_lock_all_example()
{
	using namespace std::chrono;
    std::thread threads[threadC];
    volatile double sums[threadC];
    double sum = 0;
    LockType vLock;
    typename LockType::ValueType value = 10;
    srand(time(0));

    for (size_t iq = 0; iq < threadC; ++iq)
    {
        int i = rand() % threadC;
        threads[iq] = std::thread([&vLock,iq, i, &sums]()
        {
            vLock.LockAll();
            benchmark::DoNotOptimize(sums[iq] = long_operation(rand() % threadC + 1));
            vLock.UnlockAll(i);
            benchmark::DoNotOptimize(sums[iq] = long_operation(rand() % threadC + 1));
            vLock.Unlock(i);
            benchmark::DoNotOptimize(sums[iq] = long_operation(rand() % threadC + 1));

            vLock.LockAll();
            benchmark::DoNotOptimize(sums[iq] = long_operation(rand() % threadC + 1));
            vLock.UnlockAll(i);
            benchmark::DoNotOptimize(sums[iq] = long_operation(rand() % threadC + 1));
            vLock.Unlock(i);
            benchmark::DoNotOptimize(sums[iq] = long_operation(rand() % threadC + 1));

            vLock.LockAll();
            benchmark::DoNotOptimize(sums[iq] = long_operation(rand() % threadC + 1));
            vLock.UnlockAll(i);
            benchmark::DoNotOptimize(sums[iq] = long_operation(rand() % threadC + 1));
            vLock.Unlock(i);
            benchmark::DoNotOptimize(sums[iq] = long_operation(rand() % threadC + 1));
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
        benchmark::DoNotOptimize(sum += sums[i]);
	}
	return sum;
    
}







//cppcheck-suppress constParameterCallback
static void BM_ValueLockTimeSame(benchmark::State& state) {
  for (auto a : state)
  {
      double sum = value_lock_example_same_values<NickSV::Tools::ValueLock<uint32_t, 10>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_ValueLockTimeSame)->Unit(benchmark::kMillisecond)->Iterations(200);

//cppcheck-suppress constParameterCallback
static void BM_DynamicValueLockTimeSame(benchmark::State& state) {
  for (auto a : state)
  {
      double sum = value_lock_example_same_values<NickSV::Tools::DynamicValueLock<uint32_t>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_DynamicValueLockTimeSame)->Unit(benchmark::kMillisecond)->Iterations(200);

//cppcheck-suppress constParameterCallback
static void BM_FakeValueLockTimeSame(benchmark::State& state) {
  for (auto a : state)
  {
      double sum = value_lock_example_same_values<NickSV::Tools::FakeValueLock<uint32_t, 10>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_FakeValueLockTimeSame)->Unit(benchmark::kMillisecond)->Iterations(200);




//cppcheck-suppress constParameterCallback
static void BM_ValueLockTimeDif(benchmark::State& state) {
  for (auto a : state)
  {
      double sum = value_lock_example_different_values<NickSV::Tools::ValueLock<uint32_t, 10>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
  std::cout << "================================================================================" << std::endl ;
}

BENCHMARK(BM_ValueLockTimeDif)->Unit(benchmark::kMillisecond)->Iterations(200);


//cppcheck-suppress constParameterCallback
static void BM_DynamicValueLockTimeDif(benchmark::State& state) {
  for (auto a : state)
  {
      double sum = value_lock_example_different_values<NickSV::Tools::DynamicValueLock<uint32_t>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_DynamicValueLockTimeDif)->Unit(benchmark::kMillisecond)->Iterations(200);

//cppcheck-suppress constParameterCallback
static void BM_FakeValueLockTimeDif(benchmark::State& state) {
  for (auto a : state)
  {
      double sum = value_lock_example_different_values<NickSV::Tools::FakeValueLock<uint32_t, 10>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_FakeValueLockTimeDif)->Unit(benchmark::kMillisecond)->Iterations(200);










//cppcheck-suppress constParameterCallback
static void BM_ValueLockTimeRandom(benchmark::State& state) {
  for (auto a : state)
  {
      double sum = value_lock_example_random_values<NickSV::Tools::ValueLock<uint32_t, 10>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
  std::cout << "================================================================================" << std::endl ;
}

BENCHMARK(BM_ValueLockTimeRandom)->Unit(benchmark::kMillisecond)->Iterations(200);

//cppcheck-suppress constParameterCallback
static void BM_DynamicValueLockTimeRandom(benchmark::State& state) {
  for (auto a : state)
  {
      double sum = value_lock_example_random_values<NickSV::Tools::DynamicValueLock<uint32_t>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_DynamicValueLockTimeRandom)->Unit(benchmark::kMillisecond)->Iterations(200);


//cppcheck-suppress constParameterCallback
static void BM_FakeValueLockTimeRandom(benchmark::State& state) {
  for (auto a : state)
  {
      double sum = value_lock_example_random_values<NickSV::Tools::FakeValueLock<uint32_t, 10>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_FakeValueLockTimeRandom)->Unit(benchmark::kMillisecond)->Iterations(200);















//cppcheck-suppress constParameterCallback
static void BM_ValueLockAllTime(benchmark::State& state) {
  for (auto a : state)
  {
      double sum = value_lock_all_example<NickSV::Tools::ValueLock<uint32_t, 10>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
  std::cout << "================================================================================" << std::endl ;
}

BENCHMARK(BM_ValueLockAllTime)->Unit(benchmark::kMillisecond)->Iterations(50);


//cppcheck-suppress constParameterCallback
static void BM_DynamicValueLockAllTime(benchmark::State& state) {
  for (auto a : state)
  {
    double sum = value_lock_all_example<NickSV::Tools::DynamicValueLock<uint32_t>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_DynamicValueLockAllTime)->Unit(benchmark::kMillisecond)->Iterations(50);


//cppcheck-suppress constParameterCallback
static void BM_FakeValueLockAllTime(benchmark::State& state) {
  for (auto a : state)
  {
    
    double sum = value_lock_all_example<NickSV::Tools::FakeValueLock<uint32_t, 10>, 10>();
      benchmark::DoNotOptimize(sum);
      benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_FakeValueLockAllTime)->Unit(benchmark::kMillisecond)->Iterations(50);


BENCHMARK_MAIN();