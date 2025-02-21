#pragma once

#include "../core/type_definitions.h"
#include "../core/song.h"
#include "midi_defines.h"
#include "midi_data.h"
#include <stdio.h>
#include "errorcode.h"

namespace DangoFM
{
	enum MidiByteType
	{
		VoiceData,
		ChannelEvent,
		SystemEvent,
		SystemMultiPartEvent,
		MetaEvent,
		Unknown
	};
	/* Converts a MIDI file to a Song
	 *
	 *
	 */
	class MidiToSongConverter
	{
	public:
		Song* Convert(std::string midiFileName);

	private:
		ErrorCode ReadMidiFile();
		ErrorCode ReformatMidi();
		void AdjustSongByMidi();
		void ReadMidiEventsToSong();
		void AllocateForEvents();
		void WriteEventToSongChannel(uint8 channelNumber, struct ChannelEvent* event);

		// Midi reading
		amount read_midi_track_events(uint8 trackNumber,
			uint8 **trackDatas,
                           int *trackDataSizes );

		ErrorCode read_midi_events_and_instruments(uint8 *events
                                            , int eventsSizeBytes
                                            , int eventIndex);

		// Reading state
		FILE* midiFile = nullptr;
		MidiData md;
		Song* song;
		byteindex channelDataIndices[MIDI_CHANNEL_AMOUNT*2];

	public:
		static bool readingMultiPartEvent;
		static MidiByteType GetByteType(uint8 event);
		static uint32 get_stamp_size(uint32 timestamp);
		static uint32 read_time_stamp(uint8 *events, byteindex *byteIndex);
		static byteindex read_system_event(uint8 *events, byteindex byteIndex);
		static byteindex read_system_multi_part_event(uint8 *events, byteindex byteIndex);
		static byteindex read_meta_event(uint8 *events, byteindex byteIndex);

		static void print_instruments(InstrumentName *instrumentsArray, int activeChannels);
		static void print_channel_event(struct ChannelEvent event);
		static void print_meta_text(uint8* events, int byteIndex, int eventLengthBytes);
		static void print_meta_number(uint8* events, int byteIndex, int eventLengthBytes);
		static int get_midi_event_size(uint8 eventByte, bool running_status);
		static struct ChannelEvent read_midi_event(uint8 eventByte, uint8* events, byteindex byteIndex);
		amount count_midi_events(uint8 *events, int dataSize);

		static bool is_supported_event(uint8 event_type);

		// TODO move these to MidiData class
		static void sort_by_time(struct ChannelEvent *events, int eventAmount);
		static void renumber_channels(struct ChannelEvent *events, int eventAmount, InstrumentName *instrumentsArray);
		static void sort_by_channel(struct ChannelEvent *events, int eventAmount);

		static void change_notes_to_relative(struct ChannelEvent *events, int eventAmount, int channels);

		static void change_times_to_relative(struct ChannelEvent *events, int eventAmount, int channels);
	};
}
