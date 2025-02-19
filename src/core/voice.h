
#pragma once

#include "type_definitions.h"
#include "envelope.h"


namespace DangoFM
{
	// Forward declare
	struct EnvelopeSlope;
	class Envelope;

	struct OperatorState
	{
	public:
		EnvelopeSlope slope;
		real phase;
		real feedback;
		real level;
		EnvelopeState state;
		bool Advance(bool debug = false);
		OperatorState();
	};

	class Voice
	{
	public:
		uint8 note = 0;
		real volume = 0.0f;

		// For keeping track between audio callbacks
		OperatorState carrierState;
		OperatorState modulatorState;

		SampleCount noteOffTime = 0;
		SampleCount elapsed = 0;
		bool noteOn = false;

		void On(uint8 note, real volume, Envelope& carrierEnvelope, Envelope& modulatorEnvelope);
		void Off();
	};


}
