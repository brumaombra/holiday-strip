#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <stdint.h>

// The languages of the texts on the matrix (the countdown and the default greetings), in the order of their codes; free of the hardware so the native tests can use them
enum Language : uint8_t {
	ITALIAN = 0,
	ENGLISH = 1
};
constexpr uint8_t LANGUAGE_COUNT = 2;
constexpr uint8_t DEFAULT_LANGUAGE = ITALIAN; // The countdown was always in Italian before the languages

#endif