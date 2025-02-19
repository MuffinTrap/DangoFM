#include "driver.h"
#include "synth_definitions.h"
#include "memory_definitions.h"
#include "util.h"

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

	// TODO Combine into one Clock
	secondsClock = 0.0f;
	ticksClock = 0;

	this->synth = synth;
	this->channelAmount = channelAmount;
	// SetChannelAmount(channelAmount);
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
	if (this->channelAmount < channelAmount)
	{
		if (channelsArray != nullptr)
		{
			delete[] channelsArray;
		}
		printf("Create channels\n");
		channelsArray = new DataChannel[this->channelAmount];
		this->channelAmount = channelAmount;
	}

	for (int activeChannelIndex = 0; activeChannelIndex < channelAmount; activeChannelIndex++)
	{
		channelsArray[activeChannelIndex].SetIndex(activeChannelIndex);
		channelsArray[activeChannelIndex].ResetClock();
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

	FillFrontBufferWithAudio(volume);
	GetFrontBuffer(outBuffer);
}





void DangoFM::Driver::SetSongData(uint8* songData, uint8 channelsInSong)
{
	this->songData = songData;
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
	int byteIndex = 0;
	int stampStart = 0;
	int eventStart = 0;
	while(true)
	{
		// Timestamps and events are interleaved
		int stampsSize = ReadVariableLength(songData, byteIndex);
		if (stampsSize == 0)
		{
			break;
		}
		int eventsSize = ReadVariableLength(songData, byteIndex);
		stampStart = byteIndex;
		eventStart = stampStart + stampsSize;


		channelsArray[activeChannelIndex].SetByteIndices(stampStart, stampsSize, eventStart, eventsSize);
		channelsArray[activeChannelIndex].SetIndex(activeChannelIndex);
		channelsArray[activeChannelIndex].ResetClock();

		byteIndex += stampsSize + eventsSize;
		activeChannelIndex++;
	};
	channelsOutOfEvents = 0;
}



bool DangoFM::Driver::AdvanceSong(int samplesToGenerate, float volume)
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


	FillFrontBufferWithAudio(volume);

	// TODO Clock function
	secondsClock = secondsTarget;
	ticksClock = ticksTarget;

	return (channelsOutOfEvents < channelAmount);

}


// TODO Combine these two
void DangoFM::Driver::AdvanceChannel(DataChannel& channel)
{
	if (channel.stampsSize > 0)
	{
		while(AdvanceTimeStampsAndEvents(channel))
		{

		};
		if (channel.stampIndex >= channel.stampsEnd)
		{
			channelsOutOfEvents++;
		}
	}
}

bool DangoFM::Driver::AdvanceTimeStampsAndEvents(DangoFM::DataChannel& channel)
{
	// Check that there are timestamps left on this channel or not
	if (channel.stampIndex >= channel.stampsEnd)
	{
		return false;
	}

	if (channel.TickClock >= channel.NextEventTimeTicks)
	{
		// Update starting time of next event on this channel
		channel.NextEventTimeTicks += ReadVariableLength(songData, channel.stampIndex);
	}

	// We check if the time stamp of next event is further than where we
	// can go on this iteration
	bool futureEvent = (channel.NextEventTimeTicks >= ticksTarget);
	// future event, generate sound until requirement
	// is met

	int ticksBeforeEvent = 0;
	if (futureEvent) {
		// The remaining time is between the end and current channel time
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
		return false;
	}

	// Act on the next event

	// read event
	int8 relative_note = (int8)songData[channel.eventIndex];
	channel.eventIndex++;

	uint8 velocity = songData[channel.eventIndex];
	channel.eventIndex++;

	// Modify synth state
	synth->SetNoteRelative(channel.index, relative_note, velocity);

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

// Data reading
uint32 DangoFM::Driver::ReadVariableLength(uint8* dataPointer, int& byteIndex)
{
	 // if bytes matches with 128, it means most significant
  // byte is 1 and this is a multi byte value
  // Only read the 7 bytes
  uint32 value = 0;
  uint8 byte = 0;

  /* Midi standard code */
  if ((value = dataPointer[byteIndex++]) & 0x80) {// If first byte masked with multi byte is > 0
    value &= 0x7F; //  remove continuation bit == mask with 127
    do {
      value = (value << 7) + ((byte = dataPointer[byteIndex++]) & 0x7F); // Shift left and add next byte masked with 127
    } while (byte & 0x80);// as long as last read byte has the continuation bit
  }

  return value;
}

AudioBuffer DangoFM::Driver::GetWorkBuffer()
{
	return workBuffer;
}


