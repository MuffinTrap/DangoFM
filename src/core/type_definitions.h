#pragma once
#include <stdbool.h>
#include <stdint.h> // Specific sizes
#include <cstdlib>
#include <string>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

typedef float real;

typedef real* AudioBuffer;
typedef int16* SampleBuffer;
typedef uint32 SampleCount;


enum Interval : uint8
{
	Unison = 0,
	min2nd = 1,
	M2nd = 2,
	min3rd = 3,
	M3erd = 4,
	P4th = 5,
	Dim5th = 6,
	P5th = 7,
	min6th = 8,
	M6th = 9,
	min7th = 10,
	M7th = 11,
	Octave = 12
};

const std::string IntervalNames[] =
{
	"Unison",
	"Minor 2nd",
	"Major 2nd",
	"Minor 3rd",
	"Major 3rd",
	"Perfect 4th",
	"Aug4/Dim5",
	"Perfect 5th",
	"Minor 6th",
	"Major 6th",
	"Minor 7th",
	"Major 7th"
};

enum OperatorRole : uint8
{
	Carrier = 0,
	Modulator = 1
};
