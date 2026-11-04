#ifndef ARDUINO_STUB_H
#define ARDUINO_STUB_H

// Minimal Arduino.h for the native tests: just the types and helpers the logic modules use

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

// Keep a value within a range, like the Arduino core
template <typename T, typename L, typename H>
inline T constrain(T value, L low, H high) {
	return value < low ? (T)low : (value > high ? (T)high : value);
}

// Smaller of two values, like the Arduino core
template <typename A, typename B>
inline auto min(A first, B second) -> decltype(first + second) {
	return first < second ? first : second;
}

#endif