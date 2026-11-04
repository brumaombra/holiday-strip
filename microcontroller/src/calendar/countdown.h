#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <stddef.h>
#include <time.h>

// Christmas countdown and New Year dates, kept free of the hardware so the native tests can check them

bool isClockSet(const struct tm &now);
bool formatChristmasCountdown(const struct tm &now, char *text, size_t length);
int getUpcomingYear(const struct tm &now);

#endif