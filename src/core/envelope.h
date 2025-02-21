#pragma once
#include "type_definitions.h"
#include "synth_definitions.h"

namespace DangoFM
{
	enum EnvelopeState
	{
		Attack = 0,
		Decay1 = 1,
		Decay2 = 2,
		Release = 3,
		End = 4
	};

	struct EnvelopeSlope
	{
	public:
		real increment;
		real targetLevel;
		real advanceSpeed; // Set from the operator
	};

	struct EnvelopeStatus
	{
	public:
		// From BasicSynth book
		SampleCount attackTime;
		SampleCount decay1Time;
		real sustainLevel;
		SampleCount decay2Time;
		real releaseRate;
		real increment;
		real value;
		EnvelopeState state;
	};
	class Envelope
	{
	public:
		// Third try! / All realtime
		real rates[4];
		real sustainLevel;
		Envelope();
		Envelope(real attackRate, real decay1Rate, real sustainLevel, real decay2Rate, real releaseRate);
		EnvelopeSlope GetSlopeAt(EnvelopeState state);
		real advanceSpeedOffset = 0.0f;
		real advanceSpeed = 1.0f;



		//EnvelopeStatus CalculateStatus(const Voice& v, real previousValue);

	};
}
