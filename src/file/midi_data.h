#pragma once

#include "../core/channel_event.h"
#include "../core/instrument_name.h"
#include "midi_defines.h"

namespace DangoFM
{
	// Data read from a midi file

	struct MidiData
	{
	public:
		MidiData();
		~MidiData();

		ChannelEvent* events = nullptr;
		InstrumentName instrumentsArray[MIDI_CHANNEL_AMOUNT];
		uint32 eventAmount;
		uint32 ticksPerBeat;
		uint32 bpm;
		uint16 trackAmount;
	};


}
