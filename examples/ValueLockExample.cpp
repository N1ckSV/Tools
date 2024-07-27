
#include <iostream>
#include <thread>
#include <cstring>
#include <vector>
#include "..\ValueLock.h"

template<class LockT, size_t threadC>
void value_lock_example()
{
	using namespace std::chrono;
    std::thread threads[threadC];
    LockT vLock;
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
        threads[i].join();
}

template<class LockT, size_t threadC>
void value_lock_all_example()
{
	using namespace std::chrono;
    std::thread threads[threadC];
    LockT vLock;
    uint32_t value = 10;
    srand(time(0));

    std::cout << "\n\nRandom values with lock_all() -> unlock_all(val) -> unlock(val)\n";
    for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock, iq, i]()
        {
            vLock.LockAll();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
            std::this_thread::sleep_for(milliseconds(i+1));
			std::cout << "thread " << iq << " has finished working on all values except " << i << std::endl;
            vLock.UnlockAll(i);
            std::this_thread::sleep_for(milliseconds(i+1));
			std::cout << "thread " << iq << " has finished working on value " << i << std::endl;
            vLock.Unlock(i);
            std::this_thread::sleep_for(milliseconds(i+1));

            vLock.LockAll();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
            std::this_thread::sleep_for(milliseconds(i+1));
			std::cout << "thread " << iq << " has finished working on all values except " << i << std::endl;
            vLock.UnlockAll(i);
            std::this_thread::sleep_for(milliseconds(i+1));
			std::cout << "thread " << iq << " has finished working on value " << i << std::endl;
            vLock.Unlock(i);
            std::this_thread::sleep_for(milliseconds(i+1));

            vLock.LockAll();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
            std::this_thread::sleep_for(milliseconds(i+1));
			std::cout << "thread " << iq << " has finished working on all values except " << i << std::endl;
            vLock.UnlockAll(i);
            std::this_thread::sleep_for(milliseconds(i+1));
			std::cout << "thread " << iq << " has finished working on value " << i << std::endl;
            vLock.Unlock(i);
            std::this_thread::sleep_for(milliseconds(i+1));
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}
	

    std::cout << "\n\nlock_all() -> unlock_all()\n";
    for (size_t iq = 0; iq < threadC; ++iq)
    {
		int i = rand() % threadC;
        threads[iq] = std::thread([&vLock, iq, i]()
        {
            vLock.LockAll();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
            std::this_thread::sleep_for(milliseconds(i+1));
			std::cout << "thread " << iq << " has finished working on all values" << std::endl;
            vLock.UnlockAll();
            std::this_thread::sleep_for(milliseconds(i+1));

   
            vLock.LockAll();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
            std::this_thread::sleep_for(milliseconds(i+1));
			std::cout << "thread " << iq << " has finished working on all values" << std::endl;
            vLock.UnlockAll();
            std::this_thread::sleep_for(milliseconds(i+1));


            vLock.LockAll();
			std::cout << "thread " << iq << " has started working on all values" << std::endl;
            std::this_thread::sleep_for(milliseconds(i+1));
			std::cout << "thread " << iq << " has finished working on all values" << std::endl;
            vLock.UnlockAll();
            std::this_thread::sleep_for(milliseconds(i+1));
        });
    }
    for (size_t i = 0; i < threadC; ++i)
	{
        threads[i].join();
	}
    
}

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

int main(int argc, char *argv[])
{

    constexpr uint32_t threadCount = 10;
    if(argc == 1 || strcmp(argv[1], "dynamic")) // select static or dynamic
    {        
        std::cout << "\n\nStatic ValueLock selected:\n\n";
        value_lock_example<NickSV::Tools::ValueLock<uint32_t, threadCount>, threadCount>();
        value_lock_all_example<NickSV::Tools::ValueLock<uint32_t, threadCount>, threadCount>();
    }
    else
    {
        std::cout << "\n\nDynamicValueLock selected:\n\n";
        value_lock_example<NickSV::Tools::DynamicValueLock<uint32_t>, threadCount>();
        value_lock_all_example<NickSV::Tools::DynamicValueLock<uint32_t>, threadCount>();
    }

    std::cout << "\n\n\nExample is finished!\nExample is finished!\nExample is finished!" << std::endl;


    return 0;
}

