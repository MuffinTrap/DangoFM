#include "song.h"
#include "../file/midi_defines.h"

bool DangoFM::ChannelData::IsActive()
{
	if (timeStampsSize > 0 && eventsSize > 0) {
		return true;
	}
	return false;
}

uint8 DangoFM::Song::CalculateActiveChannels()
{
	uint8 activeChannels = 0;
	for (int c = 0; c < MIDI_CHANNEL_AMOUNT; c++) {
		ChannelData& ch = channelEvents[c];
		if (ch.IsActive()) {
			activeChannels++;
		}
	}
	channelAmount = activeChannels;
	return activeChannels;
}


