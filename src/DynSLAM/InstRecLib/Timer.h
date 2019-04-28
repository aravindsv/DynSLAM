//
// Created by Brendan Miller on 4/27/19.
//

#ifndef DYNSLAM_TIMER_H
#define DYNSLAM_TIMER_H

#include <chrono>
#include <string>
#include <iostream>

// Simple class for timing code in a given context.
// Prints elapsed time from creation to destruction.
class Timer {

    typedef std::chrono::time_point<std::chrono::steady_clock> time;

    std::string m_context;
    time m_start;
    bool m_done;
public:

    Timer(const std::string& context) : m_context(context), m_done(false) {

        m_start = std::chrono::steady_clock::now();
    }

    ~Timer() {
        if (!m_done) {
            print();
        }
    }

    void print() {
        time end = std::chrono::steady_clock::now();

        std::cout << "TIMER!!! " << m_context << ": " << std::chrono::duration<double>(end - m_start).count() << std::endl;
        m_done = true;
    }
};

#endif //DYNSLAM_TIMER_H
