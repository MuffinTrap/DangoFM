#include "synthchannel.h"
#include "type_definitions.h"
#include "memory_definitions.h"

DangoFM::SynthChannel::SynthChannel()
{
	workBuffer = (AudioBuffer)DANGO_MALLOC_FUNCTION(floatBufferSize);
}

AudioBuffer DangoFM::SynthChannel::GetWorkBuffer()
{
	return workBuffer;
}


void DangoFM::SynthChannel::Reset()
{
	activeVoices = 0;
	previousNote = 0;
}

void DangoFM::SynthChannel::NoteOff(uint8 note)
{
	for(int v = 0; v < activeVoices; v++)
	{
		if (voices[v].note == note) {
			voices[v].Off();
			break;
		}
	}
}

void DangoFM::SynthChannel::NoteOn(uint8 note, uint8 velocity)
{
	if (activeVoices < DANGO_VOICE_AMOUNT)
	{
		voices[activeVoices].On(note,(float)velocity/127.0f, instrument.Carrier.GetVolumeEnvelope(), instrument.Modulator.GetVolumeEnvelope());
		activeVoices++;
	}
}

void DangoFM::SynthChannel::SetNoteRelative(int8 noteRelative, uint8 velocity)
{
	uint8 prev_note = previousNote;
	uint8 next_note = prev_note + noteRelative;

	if (velocity == 0)
	{
		NoteOff(next_note);
	}
	else
	{
		NoteOn(next_note, velocity);
	}
	previousNote = next_note;
}

void DangoFM::SynthChannel::StopVoice(uint8 voice)
{
	// Get last active voice
	Voice t = voices[activeVoices-1];
	// Replaces it with the voices that stops
	voices[activeVoices-1] = voices[voice];
	// Replace stopped with the last one
	voices[voice] = t;
	// Decrease amount of active voices
	activeVoices--;
}



