#include "timing.h"

SYSTEMTIME start;

/**
 * Subtracts two times.
 *
 * TODO: Implement carrying if t2 is bigger than t1.
 */
LPSYSTEMTIME sub(LPSYSTEMTIME t1, LPSYSTEMTIME t2)
{
    t1->wYear = t1->wYear - t2->wYear;
    t1->wMonth = t1->wMonth - t2->wMonth;
    t1->wDayOfWeek = t1->wDayOfWeek - t2->wDayOfWeek;
    t1->wDay = t1->wDay - t2->wDay;
    t1->wHour = t1->wHour - t2->wHour;
    t1->wMinute = t1->wMinute - t2->wMinute;
    t1->wSecond = t1->wSecond - t2->wSecond;
    t1->wMilliseconds = t1->wMilliseconds - t2->wMilliseconds;
    return t1;
}

LPSYSTEMTIME tic()
{
    GetLocalTime(&start);
    return &start;
}

LPSYSTEMTIME toc(unsigned int silence)
{
    SYSTEMTIME end;
    GetLocalTime(&end);
    if (!silence)
    {
        std::cout << "Toc! Time Elapsed: " << end.wHour - start.wHour << "h "
            << end.wMinute - start.wMinute << "m "
            << end.wSecond - start.wSecond << "s "
            << end.wMilliseconds - start.wMilliseconds << "ms" << std::endl;
    }
    return sub(&end, &start);
}
