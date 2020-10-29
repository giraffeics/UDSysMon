#ifndef CACHED_FUNCTION_H
#define CACHED_FUNCTION_H

#include <chrono>
#include <ratio>

using std::chrono::steady_clock;

/**
 * This class assists in caching the value of functions that
 * should not be executed too frequently. When get() is called,
 * if there is not a valid value stored, the function is called and the
 * value returned is given a lifetime. If get() is called again,
 * after the lifetime has passed, the stored value will be
 * considered invalid.
 */
template<typename T, typename ... Params> class CachedFunction
{
    typedef T(*pfnT)(Params...);

public:
    CachedFunction(){}
    
    CachedFunction(long lifetime)
    {
        this->lifetime_ = lifetime;
    }

    CachedFunction(pfnT function, long lifetime)
    {
        this->function_ = function;
        this->lifetime_ = lifetime;
    }

    void setFunction(pfnT function)
    {
        this->function_ = function;
    }

    void setLifetime(long lifetime)
    {
        this->lifetime_ = lifetime;
    }

    T get(Params... p)
    {
        if(function_ == nullptr)
            return value_;

        steady_clock::time_point present = steady_clock::now();
        std::chrono::duration<float> d = present - lastUpdate_;

        if(d.count() * 1000 > lifetime_ || !everUpdated_)
        {
            lastUpdate_ = present;
            everUpdated_ = true;
            value_ = function_(p...);
        }
        return value_;
    }

    T read()
    {
        return value_;
    }

private:
    T value_;
    pfnT function_ = nullptr;
    steady_clock::time_point lastUpdate_;
    bool everUpdated_ = false;
    long lifetime_ = 1000;
};

#endif