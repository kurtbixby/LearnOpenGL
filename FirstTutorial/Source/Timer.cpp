//
//  Timer.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/12/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/Timer.h"

Timer::Timer()
{
    isRunning_ = false;
    startTime_ = std::chrono::steady_clock::time_point();
    elapsed_ = 0;
}

void Timer::Reset()
{
    isRunning_ = false;
    startTime_ = std::chrono::steady_clock::time_point();
    elapsed_ = 0;
}

void Timer::Start()
{
    isRunning_ = true;
    startTime_ = std::chrono::steady_clock::now();
}

void Timer::Stop()
{
    if (!isRunning_)
    {
        return;
    }
    
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    elapsed_ = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime_).count();
    isRunning_ = false;
}

long long Timer::ElapsedTime()
{
    if (isRunning_)
    {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_).count();
        return elapsed;
    }
    else
    {
        return elapsed_;
    }
}
