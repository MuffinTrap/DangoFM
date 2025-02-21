#pragma once

#include "type_definitions.h"
#include "synth_definitions.h"

namespace DangoFM
{
	// Contains events of a single channel
	// ready for exporting to header file
	struct ChannelData
	{
	public:
		bool IsActive();
		uint32 timeStampsSize = 0;
		uint32 eventsSize = 0;
		uint8* timeStamps = nullptr;
		uint8* events = nullptr;
	};
	/* A Song can be imported from a file, eg. MIDI
	 * Song can be played and edited in the editor
	 * Song can be exported to a .h file for a demo or game
	 *
	 */
	class Song
	{
	public:
		uint8 CalculateActiveChannels();
		uint16 ticksPerBeat; // How many MIDI ticks per musical beat
		uint16 bpm;

		// TODO Instruments ? here or in channels in editor
		uint8 channelAmount;
 		ChannelData channelEvents[DANGO_CHANNEL_AMOUNT];

	};


}
