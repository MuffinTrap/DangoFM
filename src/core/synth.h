#pragma once

#include "type_definitions.h"
#include "synthchannel.h"
#include "instrument.h"
#include "instrument.h"

namespace DangoFM
{

class Synth
{
public:
	void Init();
	void InitDefaultInstruments();
	void InitSongInstruments(Instrument* instruments, uint8 amount);

	void GenerateAudio_V3(uint8 channelNumber, AudioBuffer buffer, uint16 sampleAmount);

	void FillBufferFromChannel_V2(uint8 channelNumber, AudioBuffer buffer, uint16 sampleAmount);

	// Functions from BasicSynth book
	void GenerateSamples_V1(uint8 channelNumber, AudioBuffer buffer, uint16 sampleAmount);


	void SetNoteRelative(uint8 channelNumber, int8 noteRelative, uint8 velocity);
	void NoteOn(uint8 channelNumber, uint8 note, uint8 velocity);
	void NoteOff(uint8 channelNumber, uint8 note);
	void StopVoice(uint8 channelNumber, uint8 voice);
	void StopAllVoices();

	float VelocityToVolume(uint8 velocity);

	Instrument CreateDefaultInstrument();

	SynthChannel* GetChannel(uint8 channelNumber);

	SynthChannel* channels = nullptr;

	int modulationIndex= 10;

private:
	inline void UpdateOperatorState(OperatorState& state, Envelope& E);
	inline real GetEnvelopeValue_V2(EnvelopeStatus& E, Voice& V);
	Instrument DefaultInstrument;
};

}
