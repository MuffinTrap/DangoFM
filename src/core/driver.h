
#pragma once

#include "type_definitions.h"
#include "datachannel.h"
#include "synth.h"

namespace DangoFM
{

class Driver
{
public:
	void Init(int ticksPerBeat, int bpm, Synth* synth, uint8 channelAmount = DANGO_CHANNEL_AMOUNT);
	void SwapBuffers();

	// In PLAY mode
	void SetSongData(uint8* songData, uint8 channelsInSong);
	bool AdvanceSong(int samplesToGenerate, float volume);

	// In Editor mode can either play song
	// or free play
	void SetChannelAmount(uint8 channelAmount);
	void DriveSynth(int samplesToGenerate, float volume, SampleBuffer outBuffer);

	AudioBuffer GetWorkBuffer();

private:
	void ClearBuffer(AudioBuffer buffer);
	void PrepareToGenerate();
	void ResetChannel();

	int ReadChannelTimeStampsSize();
	int ReadChannelEventsSize();
	void AdvanceChannel(DataChannel& channel);
	bool AdvanceTimeStampsAndEvents(DataChannel& channel);

	// Data reading TODO: move to own?
	uint32 ReadVariableLength(uint8* dataPointer, int& byteIndex);

	// Data functions
	void FillFrontBufferWithAudio(float volume);
	void GetFrontBuffer(SampleBuffer outBuffer);
	void MergeToWorkBuffer(AudioBuffer channelBuffer);


	// Time functions
	float ticksToSeconds(int ticks);
	float samplesToSeconds(int samples);
	uint32 secondsToTicks(float seconds);
	uint32 secondsToSamples(float seconds);

	uint32 msToSamples(int ms);
	int samplesToBytes(int samples);
	int samplesToMs(int samples);

	void FillAudioBuffer(AudioBuffer buffer, real value);

  // Permanent
  float secondsClock; // Song running time in seconds
  uint32 ticksClock; // Song running time in ticks
  int ticksPerBeat;
  int bpm;

  // Per drive iteration
  float secondsTarget; // Time in seconds to generate in this iteration
  uint32 ticksTarget;  // Time in ticks
  uint32 samplesToGenerate;
  int channelsOutOfEvents;

  // Playing music
  Synth* synth = nullptr;


  // Advancing channels
  DataChannel* channelsArray = nullptr;
  int channelAmount = 0;

  // Buffers
  bool front_buffer_filled = false;
  SampleBuffer frontBuffer = nullptr;
  SampleBuffer backBuffer = nullptr;

  SampleBuffer buffer0 = nullptr;
  SampleBuffer buffer1 = nullptr;
  AudioBuffer workBuffer = nullptr;

  // Song data
  uint8* songData = nullptr;
};
}
