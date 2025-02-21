#pragma once
#include "type_definitions.h"
#include "synth_definitions.h"
#include "instrument.h"
#include "voice.h"

namespace DangoFM
{
	class SynthChannel
	{
	public:

		void Reset();

		SynthChannel();
		void NoteOn(uint8 note, uint8 velocity);
		void NoteOff(uint8 note);
		void SetNoteRelative(int8 noteRelative, uint8 velocity);
		void StopVoice(uint8 voice);
		void SetInstrument(Instrument* instrument);
		AudioBuffer GetWorkBuffer();

		real volume = 1.0f;

		// TODO swapback list
		uint8 activeVoices = 0;
		Voice voices[DANGO_VOICE_AMOUNT];

		int8 previousNote;

		real peakValue = 0.0f;

		Instrument* instrument = nullptr;

		AudioBuffer workBuffer = nullptr;
	};


}
