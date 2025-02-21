
#pragma once

#include "type_definitions.h"
#include "datachannel.h"
#include "synth.h"
#include "song.h"

namespace DangoFM
{

	enum DriverMode
	{
		PlayHeaderSong,
		PlayLoadedSong,
		PlaySynth
	};

class Driver
{
public:
	void Init(int ticksPerBeat, int bpm, Synth* synth, uint8 channelAmount = DANGO_CHANNEL_AMOUNT);
	void SwapBuffers();

	void Play(int samplesToGenerate, float volume, SampleBuffer outBuffer);

	// From Header
	void SetSongData(uint8* songData, uint8 channelsInSong);
	bool AdvanceSong(int samplesToGenerate, float volume, SampleBuffer outBuffer);

	// From file
	void LoadSong(Song* song);
	bool PlaySong(int samplesToGenerate, float volume, SampleBuffer outBuffer);
	void ResetSongPlayback();

	// From keyboard
	void DriveSynth(int samplesToGenerate, float volume, SampleBuffer outBuffer);

	void SetChannelAmount(uint8 channelAmount);
	void SetMasterVolume(real volume);

	bool HasLoadedSong();

	DataChannel& GetDataChannel(int i);

	DriverMode activeMode = PlaySynth;
	AudioBuffer GetWorkBuffer();

	uint32 GetTicksClock();
	real GetSecondsClock();
	uint32 GetTicksTarget();
	Song* GetLoadedSong();
	Synth* GetSynth();

private:
	void ResetClock();
	void ClearBuffer(AudioBuffer buffer);
	void PrepareToGenerate();
	void ResetChannel();

	int ReadChannelTimeStampsSize();
	int ReadChannelEventsSize();
	void AdvanceChannel(DataChannel& channel);
	bool AdvanceTimeStampsAndEvents(DataChannel& channel);

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

  // Playing Song
  Song* loadedSong = nullptr;


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

  real masterVolume = 1.0f;
};
}
