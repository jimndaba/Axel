#pragma once
#ifndef AXTIMER_H
#define AXTIMER_H

#include "Core.h"
#include <chrono>

namespace Axel
{

	class  AX_API AxTimer
	{
    public:
        AxTimer() {
            Reset();
        }

        // Resets the start time to now
        void Reset() {
            m_Start = std::chrono::high_resolution_clock::now();
            m_PreviousFrame = m_Start;
        }

        // Returns the time in seconds since the last call to GetDeltaTime()
        float GetDeltaTime() {
            auto currentFrame = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> delta = currentFrame - m_PreviousFrame;
            m_PreviousFrame = currentFrame;
            return delta.count();
        }

        // Returns total time since the timer started (or reset)
        float GetTotalTime() const {
            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> total = now - m_Start;
            return total.count();
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_PreviousFrame;
	};




}



#endif
