#pragma once

#include "type_definitions.h"

namespace DangoFM
{
	/* Channel events are similar to MIDI events
	 * In a song, each channel has a list of events
	 *
	 *
	 */
	struct ChannelEvent
	{
	public:
		TickCount time = 0;
		uint8 channel = 0;
		int8 note = 0; // Can be absolute or relative
		MidiEventType type;

		// Store either note velocity
		// or instrument index
		union
		{
			uint8 velocity = 0;
			uint8 instrument;
		};
	};
}
