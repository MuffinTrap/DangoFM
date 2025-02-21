#include "driver.h"
#include "synth_definitions.h"
#include "memory_definitions.h"
#include "util.h"
#include "variable_length.h"

void DangoFM::Driver::Init(int ticksPerBeat, int bpm, Synth* synth, uint8 channelAmount)
{
	buffer0 = (SampleBuffer)DANGO_MALLOC_FUNCTION(intBufferSize);
	buffer1 = (SampleBuffer)DANGO_MALLOC_FUNCTION(intBufferSize);
	backBuffer = buffer0;
	frontBuffer = buffer1;
	workBuffer = (AudioBuffer)DANGO_MALLOC_FUNCTION(floatBufferSize);

	this->ticksPerBeat = ticksPerBeat;
	this->bpm = bpm;
	front_buffer_filled = false;

	ResetClock();

	this->synth = synth;
	SetChannelAmount(channelAmount);
	ClearBuffer(workBuffer);
}

void DangoFM::Driver::SwapBuffers()
{
	int16 *t = frontBuffer;
	frontBuffer = backBuffer;
	backBuffer = t;
}

void DangoFM::Driver::SetChannelAmount(uint8 channelAmount)
{
	this->channelAmount = channelAmount;
	channelsArray = new DataChannel[this->channelAmount];
	for (int activeChannelIndex = 0; activeChannelIndex < channelAmount; activeChannelIndex++)
	{
		channelsArray[activeChannelIndex].SetIndex(activeChannelIndex);
		channelsArray[activeChannelIndex].ResetClock();
	}
}

void DangoFM::Driver::Play(int samplesToGenerate, float volume, SampleBuffer outBuffer)
{
	bool songContinues = true;
	switch(activeMode)
	{
		case PlayHeaderSong:
			songContinues = AdvanceSong(samplesToGenerate, volume, outBuffer);
			break;
		case PlayLoadedSong:
			songContinues = PlaySong(samplesToGenerate, volume, outBuffer);
			break;
		case PlaySynth:
			DriveSynth(samplesToGenerate, volume, outBuffer);
		break;
	}
	if (songContinues == false)
	{
		printf("Song over\n");
		activeMode = PlaySynth;
	}
}


void DangoFM::Driver::DriveSynth(int samplesToGenerate, float volume, SampleBuffer outBuffer)
{
	this->samplesToGenerate = samplesToGenerate;
	secondsTarget = samplesToSeconds(samplesToGenerate);
	ticksTarget = secondsToTicks(secondsTarget);

	ClearBuffer(workBuffer);
	// Generate audio from all channels
	for (int activeChannelIndex = 0; activeChannelIndex < DANGO_CHANNEL_AMOUNT; activeChannelIndex++)
	{
		SynthChannel* ch = synth->GetChannel(activeChannelIndex);
		ClearBuffer(ch->GetWorkBuffer());
		synth->GenerateAudio_V3(activeChannelIndex, ch->GetWorkBuffer(), samplesToGenerate);
	}

	// TODO Apply song wide effects
	for (int activeChannelIndex = 0; activeChannelIndex < DANGO_CHANNEL_AMOUNT; activeChannelIndex++)
	{
		// TODO Apply song wide effects
		// Mix all channels together
		MergeToWorkBuffer(synth->GetChannel(activeChannelIndex)->GetWorkBuffer());
	}

	FillFrontBufferWithAudio(volume * masterVolume);
	GetFrontBuffer(outBuffer);
}


void DangoFM::Driver::SetMasterVolume(real volume)
{
	masterVolume = volume;
}

void DangoFM::Driver::LoadSong(DangoFM::Song* song)
{
	loadedSong = song;
	ResetSongPlayback();
}

DangoFM::Synth * DangoFM::Driver::GetSynth()
{
	return synth;
}


void DangoFM::Driver::ResetSongPlayback()
{
	if (HasLoadedSong() == false)
	{
		return;
	}
	ResetClock();
	for (int c = 0; c < loadedSong->channelAmount; c++)
	{
		DataChannel& Ch = channelsArray[c];
		if (c < loadedSong->channelAmount)
		{
			Ch.SetIndex(c);
			ChannelData& chData = loadedSong->channelEvents[c];
			Ch.SetByteIndices(chData.timeStamps, chData.timeStampsSize, chData.events, chData.eventsSize);
		}
		else
		{
			Ch.SetByteIndices(nullptr, 0, nullptr, 0);
		}
		Ch.ResetClock();
		Ch.ResetSamplesDone();
	}
}

bool DangoFM::Driver::HasLoadedSong()
{
	return loadedSong != nullptr;
}

void DangoFM::Driver::ResetClock()
{
// TODO Combine into one Clock
	secondsClock = 0.0f;
	ticksClock = 0;
}


bool DangoFM::Driver::PlaySong(int samplesToGenerate, float volume, SampleBuffer outBuffer)
{
	if (loadedSong == nullptr)
	{
		return true;
	}
	channelsOutOfEvents = 0; // Need to reset this too!
	this->samplesToGenerate = samplesToGenerate;
	secondsTarget = secondsClock + samplesToSeconds(samplesToGenerate);
	ticksTarget = secondsToTicks(secondsTarget);

	ClearBuffer(workBuffer);
	// Generate audio from all channels
	for (int c = 0; c < loadedSong->channelAmount; c++)
	{
		channelsArray[c].ResetSamplesDone();
		AdvanceChannel(channelsArray[c]);
	}
	FillFrontBufferWithAudio(volume * masterVolume);
	GetFrontBuffer(outBuffer);

	// TODO Clock function
	secondsClock = secondsTarget;
	ticksClock = ticksTarget;

	return (channelsOutOfEvents < loadedSong->channelAmount);

}

void DangoFM::Driver::SetSongData(uint8* songData, uint8 channelsInSong)
{
	SetChannelAmount(channelsInSong);

	/*
	 *  stampsSize for ch 0
	 *  eventsSize for ch 0
	 *  All stamps for ch 0
	 *  All events for ch 0
	 *
	 *  stampsSize for ch 1
	 *  eventsSize for ch 1
	 *  All stamps for ch 1
	 *  All events for ch 1
	 *
	 *  etc...
	 */
	int activeChannelIndex = 0;
	byteindex byteIndex = 0;
	uint8* stampStart = 0;
	uint8* eventStart = 0;
	while(true)
	{
		// Timestamps and events are interleaved
		int stampsSize = read_variable_length(songData, &byteIndex);
		if (stampsSize == 0)
		{
			break;
		}
		int eventsSize = read_variable_length(songData, &byteIndex);
		stampStart = &songData[byteIndex];
		eventStart = &songData[byteIndex + stampsSize];

		channelsArray[activeChannelIndex].SetByteIndices(stampStart, stampsSize, eventStart, eventsSize);
		channelsArray[activeChannelIndex].SetIndex(activeChannelIndex);
		channelsArray[activeChannelIndex].ResetClock();

		byteIndex += stampsSize + eventsSize;
		activeChannelIndex++;
	};
	channelsOutOfEvents = 0;
}



bool DangoFM::Driver::AdvanceSong(int samplesToGenerate, float volume, SampleBuffer outBuffer)
{

	this->samplesToGenerate = samplesToGenerate;
	ClearBuffer(workBuffer);

	secondsTarget = secondsClock + samplesToSeconds(samplesToGenerate);
	ticksTarget = secondsToTicks(secondsTarget);

	// Generate audio from all channels
	for (int activeChannelIndex = 0; activeChannelIndex < channelAmount; activeChannelIndex++)
	{
		channelsArray[activeChannelIndex].ResetSamplesDone();
		AdvanceChannel(channelsArray[activeChannelIndex]);
	}


	FillFrontBufferWithAudio(volume * masterVolume);

	// TODO Clock function
	secondsClock = secondsTarget;
	ticksClock = ticksTarget;

	GetFrontBuffer(outBuffer);

	return (channelsOutOfEvents < channelAmount);
}


// TODO Combine these two
void DangoFM::Driver::AdvanceChannel(DataChannel& channel)
{
	if (channel.stampsSize > 0)
	{
		int failsafe = ticksTarget - ticksClock;
		while(AdvanceTimeStampsAndEvents(channel) && failsafe > 0)
		{
			failsafe--;
		};
		if (failsafe == 0)
		{
			printf("Channel stuck! %d/%d/%d\n", channel.TickClock, channel.NextEventTimeTicks,ticksTarget);
		}
		if (channel.stampIndex >= channel.stampsSize && channel.eventIndex >= channel.eventsSize)
		{
			channelsOutOfEvents++;
		}
	}
}

bool DangoFM::Driver::AdvanceTimeStampsAndEvents(DangoFM::DataChannel& channel)
{
	// Check if should catch up
	if (channel.TickClock >= channel.NextEventTimeTicks)
	{
		// Do not advance past end, would get random data
		if (channel.stampIndex < channel.stampsSize)
		{
			// Update starting time of next event on this channel
			channel.NextEventTimeTicks += read_variable_length(channel.stampsStart, &channel.stampIndex);
		}
	}

	// We check if the time stamp of next event is further than where we
	// can go on this iteration

	bool pastEvent = channel.TickClock >= channel.NextEventTimeTicks;
	bool futureEvent = (channel.NextEventTimeTicks > ticksTarget);
	// future event, generate sound until requirement
	// is met

	int ticksBeforeEvent = 0;
	if (futureEvent || pastEvent) {
		// The remaining time is between the end and current channel time
		// or there is nothing to do anymore, just advance
		ticksBeforeEvent = ticksTarget - channel.TickClock;
	}
	else {
		// The next event is before iteration end time. Time until event is
		// the difference between current time and event's time.
		ticksBeforeEvent = (channel.NextEventTimeTicks - channel.TickClock);
	}
	// Call synthesizer to actually generate the sound
	// Make sure to give a real address with correct
	// offset or write goes outside array
	if (ticksBeforeEvent > 0) {
		const float generationTime = ticksToSeconds(ticksBeforeEvent);

		// Do not go over buffer size
		const uint32 samples = min_int(secondsToSamples(generationTime), samplesToGenerate - channel.SamplesDone);

		float* audioBufferLocation = &workBuffer[channel.SamplesDone * DANGO_OUT_CHANNELS];
		synth->GenerateAudio_V3(channel.index, audioBufferLocation, samples);

		// We have added samples to the buffer
		channel.SamplesDone += samples;

		// Channel time has advanced
		channel.TickClock += ticksBeforeEvent;
	}

	if (futureEvent) {
		// End handling this channel for this iteration
		// Do not advance timestamps!
		return false;
	}

	// Act on the next event

	// read event
	if (channel.eventIndex < channel.eventsSize)
	{
		int8 relative_note = (int8)channel.eventsStart[channel.eventIndex];
		channel.eventIndex++;
		uint8 velocity = channel.eventsStart[channel.eventIndex];
		channel.eventIndex++;

		// Modify synth state
		synth->SetNoteRelative(channel.index, relative_note, velocity);
	}

	// Check the next event if we don't have enough samples yet.
	return (channel.TickClock < ticksTarget);
}


void DangoFM::Driver::FillFrontBufferWithAudio(float volume)
{
	int b = samplesToGenerate * DANGO_OUT_CHANNELS;
	do {
		frontBuffer[b-1] = (int16)(workBuffer[b-1] * volume * SAMPLE_MAX);
	} while(--b);
	front_buffer_filled = true;
}

void DangoFM::Driver::FillAudioBuffer(AudioBuffer buffer, real value)
{
	int b = samplesToGenerate * DANGO_OUT_CHANNELS;
	do {
		buffer[b-1]  = value;
	} while(--b);
}

void DangoFM::Driver::MergeToWorkBuffer(AudioBuffer channelBuffer)
{
		int b = DANGO_SAMPLES_PER_CALLBACK * DANGO_OUT_CHANNELS;
		do {
			workBuffer[b-1] += channelBuffer[b-1];
		} while(--b);

}

void DangoFM::Driver::GetFrontBuffer(SampleBuffer outBuffer)
{
	if (front_buffer_filled) {
		int b = DANGO_SAMPLES_PER_CALLBACK * DANGO_OUT_CHANNELS;
		do {
			outBuffer[b-1] = frontBuffer[b-1];
		} while(--b);
	}
}


void DangoFM::Driver::ClearBuffer(AudioBuffer buffer)
{
	DANGO_MEMSET_FUNCTION(buffer, 0.0f, floatBufferSize);
}

void DangoFM::Driver::PrepareToGenerate()
{
}

float DangoFM::Driver::ticksToSeconds(int ticks)
{
	float beats = (float)ticks / (float)ticksPerBeat;
	float beatsPerSeconds = (float)bpm / 60.0f;
	return beats / beatsPerSeconds;
}

float DangoFM::Driver::samplesToSeconds(int samples)
{
	return (float)samples/(float)DANGO_SAMPLES_PER_SECOND;
}


uint32 DangoFM::Driver::secondsToTicks(float seconds)
{
	float beats = seconds/60.0f * (float)bpm;
	float ticks = beats * ticksPerBeat;
	return ticks;
}

uint32 DangoFM::Driver::secondsToSamples(float seconds)
{
	return seconds * (float)DANGO_SAMPLES_PER_SECOND;
}

AudioBuffer DangoFM::Driver::GetWorkBuffer()
{
	return workBuffer;
}

DangoFM::DataChannel & DangoFM::Driver::GetDataChannel(int i)
{
	return channelsArray[i];
}

real DangoFM::Driver::GetSecondsClock()
{
	return secondsClock;
}

uint32 DangoFM::Driver::GetTicksClock()
{
	return ticksClock;
}

uint32 DangoFM::Driver::GetTicksTarget()
{
	return ticksTarget;
}

DangoFM::Song * DangoFM::Driver::GetLoadedSong()
{
	return loadedSong;
}



