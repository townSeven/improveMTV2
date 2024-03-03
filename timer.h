/*
 * timer.h
 *
 *  Created on: May 13, 2012
 *      Author: root
 */

#ifndef TIMER_H_
#define TIMER_H_


#ifndef _TIMER_H_
#define _TIMER_H_

#ifndef WIN32

#include <sys/time.h>
#include <cstdlib>

typedef struct timeval timer;

#define GETTIMEOFDAY(T) gettimeofday(T, NULL)

#define DIFF_MSEC(T, U) \
	((((int) ((T)->tv_sec - (U)->tv_sec)) * 1000000.0 + \
	((int) ((T)->tv_usec - (U)->tv_usec))) / 1000.0)
#else
/*
 * To get good resolution (better than ~15ms) on Windows, use
 * the high resolution performance counters. They can't be used
 * to get absolute times, but are good for measuring differences.
 */
#include <windows.h>
static inline double GetTimerFrequency(void) {
	LARGE_INTEGER f;

	QueryPerformanceFrequency(&f);
	return (double) f.QuadPart;
}

typedef LARGE_INTEGER timer;

#define GETTIMEOFDAY(T) QueryPerformanceCounter((T))
#define DIFF_MSEC(T, U) \
	(((T)->QuadPart - (U)->QuadPart) * 1000.0 / GetTimerFrequency())
#endif   /* WIN32 */

static inline timer timerStart() {
	timer T1;
	GETTIMEOFDAY(&T1);
	return T1;
}

static inline timer timerEnd() {
	timer T2;
	GETTIMEOFDAY(&T2);
	return T2;
}
static inline double elapsedTime(timer & start, timer & end) {
	return (double) DIFF_MSEC(&end, &start);
}

#endif /* _TIMER_H_ */



#endif /* TIMER_H_ */
