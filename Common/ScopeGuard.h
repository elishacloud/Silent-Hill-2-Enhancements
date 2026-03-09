#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct ScopedCriticalSection
{
private:
    bool enable;
    CRITICAL_SECTION* cs;
public:
    // Constructor enters critical section
    ScopedCriticalSection(CRITICAL_SECTION* cs, bool setenable = true) : cs(cs), enable(setenable)
    {
        if (enable && cs)
        {
            EnterCriticalSection(cs);
        }
    }
    // Destructor leaves critical section
    ~ScopedCriticalSection()
    {
        if (enable && cs)
        {
            LeaveCriticalSection(cs);
        }
    }
};
