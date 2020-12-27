#include "timing.h"

#define MS_IN_SECS 1000
#define SECS_IN_MIN 60
#define MIN_IN_HR 60
#define HR_IN_DAY 24

SYSTEMTIME start;
SYSTEMTIME end;

/**
 * Subtracts two times.
 *
 * This function only handles up to hours and ignores the rest.
 */
LPSYSTEMTIME sub(LPSYSTEMTIME t1, LPSYSTEMTIME t2)
{
    int ms = 0;
    int s = 0;
    int m = 0;
    int h = 0;

    if (t1->wMilliseconds >= t2->wMilliseconds) {
        ms = t1->wMilliseconds - t2->wMilliseconds;
    }
    else {
        ms = MS_IN_SECS - (t2->wMilliseconds - t1->wMilliseconds);
        s -= 1;
    }

    if (t1->wSecond >= t2->wSecond) {
        s += t1->wSecond - t2->wSecond;
    }
    else {
        s += SECS_IN_MIN - (t2->wSecond - t1->wSecond);
        m -= 1;
    }

    if (t1->wMinute >= t2->wMinute) {
        m += t1->wMinute - t2->wMinute;
    }
    else {
        m += MIN_IN_HR - (t2->wMinute - t1->wMinute);
        h -= 1;
    }

    // Don't really care about the hours as it shouldn't get to this long
    if (t1->wHour >= t2->wHour) {
        h += t1->wHour - t2->wHour;
    }
    else {
        h += HR_IN_DAY - (t2->wHour - t1->wHour);
    }

    t1->wMilliseconds = ms;
    t1->wSecond = s;
    t1->wMinute = m;
    t1->wHour = h;
    return t1;
}

const LPSYSTEMTIME tic()
{
    GetLocalTime(&start);
    return &start;
}

const LPSYSTEMTIME toc(bool silence, const char* message)
{
    GetLocalTime(&end);
    end = *sub(&end, &start);
    if (!silence)
    {
        std::cout << message << std::endl;
        std::cout << "Toc! Time Elapsed: "
            << end.wHour << "h "
            << end.wMinute << "m "
            << end.wSecond << "s "
            << end.wMilliseconds << "ms" << std::endl;
    }
    return &end;
}
