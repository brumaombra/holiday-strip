#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <stddef.h>
#include <time.h>

#include "config/language.h"

// Christmas countdown and New Year dates, kept free of the hardware so the native tests can check them

bool isClockSet(const struct tm &now);
long getDaysToChristmas(const struct tm &now);
void formatChristmasCountdown(long days, uint8_t language, char *text, size_t length);
int getUpcomingYear(const struct tm &now);

#endif