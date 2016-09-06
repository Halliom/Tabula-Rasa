#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

class Async
{
public:
    
    //TODO: Check these on windows

    /**
     * Increments the variable and returns the old value
     */
    template<typename T>
    static T Increment(volatile T* Destination)
    {
#ifdef _WIN32
        return InterlockedIncrement(Destination) - 1; // return the previous value
#else
        return __sync_fetch_and_add(Destination, 1);
#endif
    }

    /**
     * Decrements the variable and returns the old value
     */
    template<typename T>
    static T Decrement(volatile T* Destination)
    {
#ifdef _WIN32
        return InterlockedDecrement(Destination) + 1; // return the previous value
#else
        return __sync_fetch_and_sub(Destination, 1);
#endif
    }
}
