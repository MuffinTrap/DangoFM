#include "midi_data.h"

DangoFM::MidiData::MidiData()
{
	for (int i = 0; i < MIDI_CHANNEL_AMOUNT; i++)
	{
		instrumentsArray[i] = AcousticPiano;
	}
}

DangoFM::MidiData::~MidiData()
{

}


