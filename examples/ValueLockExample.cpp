
#include <iostream>
#include <thread>
#include "ValueLock.h"

void value_lock_example()
{
	using namespace std::chrono;
    const size_t threadC = 10;
    std::thread threads[threadC];
    NickSV::Tools::ValueLock<uint32_t, threadC, 500> vLock;
    uint32_t value = 10;
	std::cout << "Same value: " << std::endl;
    for (size_t i = 0; i < threadC; ++i)
    {
        threads[i] = std::thread([&vLock, value, i]()
        {
            NickSV::Tools::ValueLockGuard<decltype(vLock)> vLockGuard(vLock, value);
			std::cout << "thread " << i << " has started working on value " << value << std::endl;
            std::this_thread::sleep_for(milliseconds(100));
			std::cout << "thread " << i << " has finished working on value " << value << std::endl;
        });
    }
    for (size_t i = 0; i < threadC; ++i)
        threads[i].join();
	
	std::mutex inputMutex;
	std::cout << std::endl << std::endl << "Diff values: " << std::endl;
	for (size_t i = 0; i < threadC; ++i)
    {
        threads[i] = std::thread([&vLock, &inputMutex, i]()
        { 
            NickSV::Tools::ValueLockGuard<decltype(vLock)> vLockGuard(vLock, i);
			inputMutex.lock();
			std::cout << "thread " << i << " has started working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(100));
			inputMutex.lock();
			std::cout << "thread " << i << " has finished working on value " << i << std::endl;
			inputMutex.unlock();
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}
	srand(time(0));
	std::cout << std::endl << std::endl << 
        "Random values.\nIf some thread starts to work on value X, it has to finish it first before next thread starts to work on value X:" 
        << std::endl;
	for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock, &inputMutex, iq, i]()
        { 
            NickSV::Tools::ValueLockGuard<decltype(vLock)> vLockGuard(vLock, i);
			inputMutex.lock();
			std::cout << "thread " << iq << " has started working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(100));
			inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on value " << i << std::endl;
			inputMutex.unlock();
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}
}

struct fake_mutex
{
    void lock() {};
    void unlock() {};
};

void value_lock_all_example()
{
	using namespace std::chrono;
    const size_t threadC = 10;
    std::thread threads[threadC];
    NickSV::Tools::ValueLock<uint32_t, threadC, 500> vLock;
    uint32_t value = 10;
	std::mutex inputMutex;
    srand(time(0));
	std::cout << "\n\n\n\nRandom values with lock(val) -> lock_all(val) -> unlock_all(val) -> unlock(val)\n";
	for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock, &inputMutex, iq, i]()
        {
            {
            VALUE_LOCK_GUARD(vLock, i);
			inputMutex.lock();
			std::cout << "thread " << iq << " has started working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.LockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.UnlockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on all values except " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
			inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on value " << i << std::endl;
			inputMutex.unlock();
            }

            {
            VALUE_LOCK_GUARD(vLock, i);
			inputMutex.lock();
			std::cout << "thread " << iq << " has started working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.LockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.UnlockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on all values except " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
			inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on value " << i << std::endl;
			inputMutex.unlock();
            }

            {
            VALUE_LOCK_GUARD(vLock, i);
			inputMutex.lock();
			std::cout << "thread " << iq << " has started working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.LockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.UnlockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on all values except " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
			inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on value " << i << std::endl;
			inputMutex.unlock();
            }
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}

    std::cout << "\n\nRandom values with lock_all() -> unlock_all(val) -> unlock(val)\n";
    for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock, &inputMutex, iq, i]()
        {
            vLock.LockAll();
            inputMutex.lock();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.UnlockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on all values except " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.Unlock(i);
			inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));

            vLock.LockAll();
            inputMutex.lock();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.UnlockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on all values except " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.Unlock(i);
			inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));

            vLock.LockAll();
            inputMutex.lock();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.UnlockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on all values except " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.Unlock(i);
			inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}
	
    std::cout << "\n\nRandom values with lock(val) -> lock_all(val) -> unlock_all()\n";
    for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock, &inputMutex, iq, i]()
        { 
            vLock.Lock(i);
			inputMutex.lock();
			std::cout << "thread " << iq << " has started working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.LockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.UnlockAll();
            inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on all values except" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));

            vLock.Lock(i);
			inputMutex.lock();
			std::cout << "thread " << iq << " has started working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.LockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.UnlockAll();
            inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on all values except" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));

            vLock.Lock(i);
			inputMutex.lock();
			std::cout << "thread " << iq << " has started working on value " << i << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.LockAll(i);
            inputMutex.lock();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
            vLock.UnlockAll();
            inputMutex.lock();
			std::cout << "thread " << iq << " has finished working on all values except" << std::endl;
			inputMutex.unlock();
            std::this_thread::sleep_for(milliseconds(i+1));
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}
}

#include <vector>

struct A
{
    int val;
    explicit A(int v) : val(v) {};
    inline static int count = 0;
    void throwFunc()
    {
        count++;
        if(count > 5)
            throw 5;
    }
};

int main()
{
    value_lock_example();
    value_lock_all_example();

    //std::vector<A> vec = {{1}, {2}, {3}, {4}, {5}, {6}, {7}};
    //try{
    //NickSV::Tools::for_each_exception_safe(vec.begin(), vec.end(), 
    //[](A& a) 
    //{
    //    a.throwFunc();
    //    a.val += 10;
    //    
    //},
    //[](A& a) 
    //{
    //    a.val -= 10;
    //    a.throwFunc();
    //});
    //}
    //catch (...) {}

    //int dummy;
    //std::cin >> dummy;


    return 0;
}

