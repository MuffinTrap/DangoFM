#pragma once
#include "type_definitions.h"
#include <SDL_stdinc.h>

#ifndef DANGO_OUT_CHANNELS
#define DANGO_OUT_CHANNELS 1
#endif

#ifndef DANGO_SAMPLES_PER_CALLBACK
#define DANGO_SAMPLES_PER_CALLBACK 1024
#endif

#define DANGO_SAMPLES_PER_SECOND 44100

static const int16 SAMPLE_MAX = 32765;
static const std::size_t floatBufferSize = sizeof(real) * DANGO_SAMPLES_PER_CALLBACK * DANGO_OUT_CHANNELS;
static const std::size_t intBufferSize = sizeof(int16) * DANGO_SAMPLES_PER_CALLBACK * DANGO_OUT_CHANNELS;
static const real TAU = (real)M_PI*2.0;
static const real Freq2Rad = M_PI*2.0/(real)DANGO_SAMPLES_PER_SECOND;

static const int MinFreqMultiplier = 0;
static const int MaxFreqMultiplier = 12;

#define DANGO_CHANNEL_AMOUNT 16
#define DANGO_VOICE_AMOUNT 16
#define DANGO_OPERATORS 3

#define DANGO_SYNTH_ENVELOPE_CUTOFF 0.0f
#define DANGO_SYNTH_AMPLITUDE_CUTOFF 0.01f
