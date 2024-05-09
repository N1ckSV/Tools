

#include "NickSV/Tools/ValueLock.h"

#include <iostream>
#include <vector>
#include <thread>



//NOT WORKING
int value_lock_test()
{
	int stage = 0;
	using namespace std::chrono;
    const size_t threadC = 25;
	auto timeToSleep = milliseconds(5);
    std::vector<std::thread> threads;
    NickSV::Tools::ValueLock<uint32_t, threadC, 500> vLock;
    volatile uint32_t sharedValues[threadC] = {0};
    uint32_t maxIterations = 60;
	auto allGood = true;
	srand(time(0));
	uint32_t iterations = 0;
	while(iterations < maxIterations)
	{
		threads.clear();
		threads.resize(threadC);
		for (uint32_t iq = 0; iq < threadC; ++iq)
    	{
			uint32_t i = rand() % threadC;
    	    threads[iq] = std::thread([&vLock, &sharedValues, &allGood, timeToSleep, iq, i]()
    	    { 
    	        NickSV::Tools::ValueLockGuard<decltype(vLock)> vLockGuard(vLock, i);
				sharedValues[i] = iq;
    	        std::this_thread::sleep_for(timeToSleep);
                //cppcheck-suppress knownConditionTrueFalse
				if(sharedValues[i] != iq)
					allGood = false;
    	    });
    	}
    	std::this_thread::sleep_for(threadC*timeToSleep + milliseconds(10));
    	for (size_t i = 0; i < threadC; ++i)
		{
        //#ifdef _WIN32
        //    HANDLE handle = reinterpret_cast<HANDLE>(threads[i].native_handle());
        //    TerminateThread(handle, 0);
        //    CloseHandle(handle);
        //#else
        //    pthread_cancel(threads[i].native_handle());
        //#endif
		}
		for (size_t i = 0; i < threadC; ++i) {
    		try {
    		    threads[i].join();
    		} catch (const std::system_error& e) {
    		    std::cerr << "Error joining thread: " << e.what() << std::endl;
    		}
		}
		iterations++;
		if(!allGood) break;
    }
	stage = iterations + 1;
	if(!allGood) return stage;
	return 0;
}


int main()
{
	return 0;
}