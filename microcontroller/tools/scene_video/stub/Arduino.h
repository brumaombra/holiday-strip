#ifndef ARDUINO_STUB_H
#define ARDUINO_STUB_H

// Arduino.h for the scene video renderer: the types and helpers the scenes use, with a clock the renderer sets

#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>

#ifndef PI
#define PI 3.14159265358979f
#endif

// Keep a value within a range, like the Arduino core
template <typename T, typename L, typename H>
inline T constrain(T value, L low, H high) {
	return value < low ? (T)low : (value > high ? (T)high : value);
}

// Smaller and larger of two values, like the Arduino core
template <typename A, typename B>
inline auto min(A first, B second) -> decltype(first + second) {
	return first < second ? first : second;
}
template <typename A, typename B>
inline auto max(A first, B second) -> decltype(first + second) {
	return first > second ? first : second;
}

// Square of a number, like the Arduino core
template <typename T>
inline T sq(T value) {
	return value * value;
}

// Random 32 bit number (seeded by the renderer, so a video is the same every time)
inline uint32_t esp_random() {
	return ((uint32_t)rand() << 16) ^ (uint32_t)rand();
}

// The clock of the renderer: the local time it shows (0 when unknown) plus the seconds the video has played
extern time_t rendererClockStart;
extern float rendererElapsedSeconds;

// Read the local time, like the ESP32 core (false while the clock is unknown)
inline bool getLocalTime(struct tm *info, uint32_t waitMs = 5000) {
	if (rendererClockStart == 0) {
		return false;
	}
	const time_t now = rendererClockStart + (time_t)rendererElapsedSeconds;
	localtime_r(&now, info);
	return true;
}

// Milliseconds since the video started
inline uint32_t millis() {
	return (uint32_t)(rendererElapsedSeconds * 1000);
}

// The subset of the Arduino String the scenes use
class String {
public:
	String() = default;
	String(const char *text) : value(text) {}
	String(int number) : value(std::to_string(number)) {}
	String(long number) : value(std::to_string(number)) {}
	size_t length() const { return value.size(); }
	char operator[](size_t index) const { return value[index]; }
	String &operator+=(const char *text) {
		value += text;
		return *this;
	}
	const char *c_str() const { return value.c_str(); }

private:
	std::string value;
};

#endif