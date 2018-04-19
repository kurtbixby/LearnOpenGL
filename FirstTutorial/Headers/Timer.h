//
//  Timer.hpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/12/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef TIMER_H
#define TIMER_H

#include <stdio.h>
#include <chrono>

class Timer {
    bool isRunning_;
    std::chrono::steady_clock::time_point startTime_;
    long long elapsed_;
    
public:
    Timer();
    
    void Reset();
    void Start();
    void Stop();
    
    long long ElapsedTime();
};

#endif /* Timer_hpp */
