#include "midi_to_song.h"
#include <stdio.h>
#include "../core/memory_definitions.h"
#include "../core/variable_length.h"
#include "../core/instrument_name.h"
#include <assert.h>
#include "errorcode.h"

#define W_VERBOSE true
#define VERBOSE true
#define R_VERBOSE true

// byte 0 to byte 3
#define SWAP32(num) (((num>>24)&0xff)    |      \
                     ((num<<8)&0xff0000) |      \
                     ((num>>8)&0xff00)   |      \
                     ((num<<24)&0xff000000))
#define SWAP16(v) ((v >> 8) | (v << 8))

bool DangoFM::MidiToSongConverter::readingMultiPartEvent;

DangoFM::Song * DangoFM::MidiToSongConverter::Convert(std::string midiFileName)
{
	song = nullptr;
	readingMultiPartEvent = false;
    ErrorHandler::clear_system_error();
	midiFile = fopen(midiFileName.c_str(), "rb");
	if (midiFile != nullptr)
	{
		ErrorCode c = ReadMidiFile();
        if (c != NO_ERROR)
        {
          ErrorHandler::print_dango_error(c);
        }
		else if (md.events != nullptr)
		{
            ReformatMidi();
			ReadMidiEventsToSong();
		}
		// MidiData is destroyed
	}

	return song;
}
DangoFM::ErrorCode DangoFM::MidiToSongConverter::ReadMidiFile()
{
  // Read the header (22 bytes) into memory
  uint8 headerASCII[4];

  fread(headerASCII, 1, 4, midiFile);

  // First 8 bytes are header and
  // header length 00 00 00 06
  if (VERBOSE) {
    printf("Midi file header %c %c %c %c\n"
           , headerASCII[0]
           , headerASCII[1]
           , headerASCII[2]
           , headerASCII[3]);
  }
  int32 headerSizeBytes = 0;
  fread(&headerSizeBytes, 4, 1, midiFile);
  headerSizeBytes = SWAP32(headerSizeBytes);
  if (VERBOSE) {
    printf("Midi header size %d\n", headerSizeBytes);
  }

  // Next 3 16bit words:
  // format, track amount and ticks per beat

  // Word 1 format
  uint16 midiFormat = 0;
  fread(&midiFormat, 2, 1, midiFile);
  midiFormat = SWAP16(midiFormat);

  if (VERBOSE) {
    printf("Midi file format %d\n", midiFormat);
  }
  if (midiFormat != 1) {
    fclose(midiFile);
    return FORMAT_NOT_SUPPORTED;
  }

  // Word 2 track amount
  uint16 trackAmount = 0;
  fread(&trackAmount, 2, 1, midiFile);
  trackAmount = SWAP16(trackAmount);
  md.trackAmount = trackAmount;

  // Word 3 ticks per beat
  // or fancy SMPTE format which this converter does not understand
  uint16 tpb = 0;
  fread(&tpb, 2, 1, midiFile);
  tpb = SWAP16(tpb);
  if (tpb & 0xFFFF8000) {
    printf("Midi uses SMPTE timing, cannot read that!\n");
    fclose(midiFile);
    return FORMAT_NOT_SUPPORTED;
  }
  md.ticksPerBeat = tpb;

  if (VERBOSE) {
    printf("Midi file track amount %d\n", trackAmount);
    printf("Midi ticks per beat %d\n", tpb);
  }

  // For MIDI file that has multiple tracks
  // loop here
  // and collect all track sizes and add all
  // channel events and then sort
  // the events based on time stamps

  // NOTE. Array size must be constant. But GCC does not give a warning.

  uint8 *trackDatas[MIDI_CHANNEL_AMOUNT];
  int trackDataSizes[MIDI_CHANNEL_AMOUNT];
  int trackEventAmounts[MIDI_CHANNEL_AMOUNT];
  for (int i = 0; i < trackAmount; i++)
    {
      trackDatas[i] = NULL;
      trackDataSizes[i] = 0;
      trackEventAmounts[i] = 0;
    }

  int eventAmount = 0;
  for (int track = 0; track < trackAmount; track++) {
    amount track_event_amount = read_midi_track_events(track,
                                             trackDatas,
                                             trackDataSizes);

    trackEventAmounts[track] = track_event_amount;
    eventAmount += track_event_amount;
  }

  // END FOR TRACKS
  fclose(midiFile);

  // MALLOC
  int needed_memory = sizeof(ChannelEvent) * eventAmount;
  if (needed_memory == 0) {
    return ErrorCode::ZERO_READ;
  }
  md.eventAmount = eventAmount;
  md.events = new struct ChannelEvent[eventAmount];

  int eventIndex = 0;
  ErrorCode read_error = NO_ERROR;
  for (int track = 0; track < trackAmount; track++) {
    read_error = read_midi_events_and_instruments(trackDatas[track],
                                                  trackDataSizes[track],
                                                  eventIndex);


    if (read_error != NO_ERROR) {
      break;
    }

    eventIndex += trackEventAmounts[track];
  }

  if (VERBOSE) {
    printf("All %d events read from tracks\n", md.eventAmount);
    printf("Midi file track amount %d\n", trackAmount);
  }

#ifdef DANGO_MEM
  // nop
#else
  if (VERBOSE) {
    printf("Freeing track memory\n");
  }
  for (int t = 0; t < trackAmount; t++) {
    if (trackDataSizes[t] > 0) {
      void *ptr = trackDatas[t];
      printf("Freeing track %u memory\n", t);
      DANGO_FREE_FUNCTION(ptr);
    }
  }
#endif // DANGO_MEM

	return NO_ERROR;
}

DangoFM::ErrorCode DangoFM::MidiToSongConverter::ReformatMidi()
{
// Sort read events by channel
  sort_by_channel(md.events, md.eventAmount);

  printf("Sorted by channel\n");


  // Change channels to 0,1,2...
  renumber_channels(md.events, md.eventAmount, md.instrumentsArray);

  printf("Renumbered channels\n");


  // Sort each channel by time
  sort_by_time(md.events, md.eventAmount);

  printf("Sorted by time\n");


  // Since we are sorted by channel, change
  // channel bits to voice bits
  /*
    Version 2, not needed anymore.
    change_channel_to_voice(midi.events, midi.eventAmount);
    if (VERBOSE) {
    printf("Changed channel to voice\n");
    }
  */

  // Change times to relative
  change_times_to_relative(md.events, md.eventAmount, md.trackAmount);

  printf("Times changed to relative\n");

  change_notes_to_relative(md.events, md.eventAmount, md.trackAmount);

  printf("Notes changed to relative\n");


  // MALLOC
  // also sets song->channels
  // Data write heads


  if (VERBOSE) {
    print_instruments(md.instrumentsArray, md.trackAmount);
  }


  if (R_VERBOSE) {
    printf("\n");
    printf("\n");
    for (int i = 0; i < md.eventAmount; i++) {
      printf("------------------------\n");
      struct ChannelEvent *event = &md.events[i];
      print_channel_event(*event);
    }
  }
  return NO_ERROR;
}



void DangoFM::MidiToSongConverter::ReadMidiEventsToSong()
{
	AllocateForEvents();
	int prev_channel = -1;
	for (int i = 0; i < md.eventAmount; i++) {
		struct ChannelEvent *event = &md.events[i];
		int channel = event->channel;
		if (channel != prev_channel)
		{
			if (W_VERBOSE)
			{
				printf("\nWriting channel %d\n", channel);
				prev_channel = channel;
			}
		}
		if (channel < song->channelAmount) {
			WriteEventToSongChannel(channel, event);
		}
		else if (W_VERBOSE) {
			printf("Event %d/%d has an invalid channel %d\n", i, md.eventAmount, channel);
		}
	}
    if (W_VERBOSE) {
      printf("Write done!\n");
    }
}

void DangoFM::MidiToSongConverter::AllocateForEvents()
{

// Init song data
  song = new Song();
  ChannelData* channelEvents = song->channelEvents;
  for (int i = 0; i < MIDI_CHANNEL_AMOUNT; i++) {
    channelEvents[i].events = nullptr;
    channelEvents[i].timeStamps = nullptr;
    channelEvents[i].eventsSize = 0;
    channelEvents[i].timeStampsSize = 0;
  }

  for (int e = 0; e < md.eventAmount; e++) {
    struct ChannelEvent event = md.events[e];
    // Count event and stamp sizes
    channelEvents[event.channel].eventsSize += CHANNEL_EVENT_SIZE;
    channelEvents[event.channel].timeStampsSize += get_stamp_size(event.time);
  }

  int ev = 0;
  int ts = 0;
  for (int i = 0; i < MIDI_CHANNEL_AMOUNT; i++) {
    int es = channelEvents[i].eventsSize;
    int ss = channelEvents[i].timeStampsSize;
    ev += es;
    ts += ss;
    if (VERBOSE) {
      if (es > 0 && ss > 0) {
        printf("Allocating for Ch:%d stamps and events: %d\t%d\n", i, ss, es);
      }
    }
  }
  if (VERBOSE) {
    printf("Allocating for total stamps and events: %d\t%d\n", ts, ev);
  }

  // Reserve memory and
  // Prepare to write
  song->CalculateActiveChannels();
  //song->channels = midi->track_amount;

  for (int i = 0; i < song->channelAmount; i++) {
    size_t ss = channelEvents[i].timeStampsSize;
    size_t es = channelEvents[i].eventsSize;
    assert((ss > 0 && es > 0) || (ss == 0 && es == 0) && "Channel timestamps and events size do not make sense"); // Both nonzero or both zero!

    if (ss > 0 && es > 0) {
      channelEvents[i].timeStamps = (uint8*)DANGO_MALLOC_FUNCTION(ss);
      channelEvents[i].events = (uint8*)DANGO_MALLOC_FUNCTION(es);
      channelDataIndices[i * 2] = 0;
      channelDataIndices[i * 2 + 1] = 0;
    }
  }

  if (VERBOSE) {
    printf("Allocation done\n");
  }
}

amount DangoFM::MidiToSongConverter::read_midi_track_events(uint8 track, uint8 ** trackDatas, int* trackDataSizes)
{
  if (VERBOSE) {
    printf("\nReading track %d\n", track);
  }
  // Track header 4 bytes
  // Track size 4 bytes
  uint8 trkHdr[4];
  fread(trkHdr, 1, 4, midiFile);
  int32 trackSize = 0;
  fread(&trackSize, 1, 4, midiFile);
  uint32 dataLength = SWAP32(trackSize);

  if (VERBOSE) {
    printf("Midi track header %c %c %c %c\n"
    , trkHdr[0]
    , trkHdr[1]
    , trkHdr[2]
    , trkHdr[3]);

    printf("Midi data block length %d\n", dataLength);
  }
  if (VERBOSE)
  {
    printf("Reserving %u bytes of memory for track %u\n", dataLength, track);
  }
  trackDataSizes[track] = dataLength;
  uint8* trackMemoryPtr = NULL;
  if (dataLength > 0) {
    trackMemoryPtr = (uint8*)DANGO_MALLOC_FUNCTION(dataLength);

    if (trackMemoryPtr == NULL)
    {
      printf("Failed to reserve memory for track %u\n", track);
      return 0;
    }
  }
  trackDatas[track] = trackMemoryPtr;

  int read_block_size = 1;
  size_t readBytes = fread(trackMemoryPtr, read_block_size, dataLength, midiFile);
  uint32 real_length = (uint32)readBytes;
  if (VERBOSE)
  {
    printf("Read %lu bytes from file to track data for track %u\n", (unsigned long)readBytes, track);
  }

  if (ErrorHandler::has_system_error("Midi to song::read midi track") || real_length != dataLength) {
    printf("Error, could not read all of the data! Read only %u/%u\n\n", real_length, dataLength);

    trackDataSizes[track] = real_length;
    return 0;
  }

  if (VERBOSE) {
    printf("Bytes for track %u\n", track);
    for (uint32 i = 0; i < real_length; i++) {
      if (i % 16 == 0){
        printf("\n");
      }
      printf("%X ", trackDatas[track][i]);
    }
    printf("\n");
  }
  int event_count = count_midi_events(trackDatas[track], real_length);

  if (VERBOSE) {
    #ifdef DANGO_LINUX
    printf("Read %I64lu event bytes from midi file\n", readBytes);
    #else
    printf("Read %u event bytes from midi file\n", (uint32)readBytes);
    #endif
    printf("Counted %d events on track\n", event_count);
  }

  return event_count;
}

void DangoFM::MidiToSongConverter::WriteEventToSongChannel(uint8 channelNumber, struct ChannelEvent* event)
{
  byteindex stampIndex = channelDataIndices[channelNumber * 2];
  byteindex eventIndex = channelDataIndices[channelNumber * 2 + 1];

  ChannelData& channel = song->channelEvents[channelNumber];

  channel.events[eventIndex] = event->note;
  eventIndex++;
  channel.events[eventIndex] = event->velocity;
  eventIndex++;

  if (W_VERBOSE) {
    for (int c = 0; c < event->channel; c++)
      printf("\t");
    if(event->velocity > 0)
      {
        printf("%d|%d\n", event->note, event->velocity);
      }
    else {
      printf("R%d\n",  event->note);
    }
  }

  // Timestamp
  stampIndex = write_variable_length(event->time, channel.timeStamps, stampIndex);

  channelDataIndices[channelNumber * 2] = stampIndex;
  channelDataIndices[channelNumber * 2 + 1] = eventIndex;
}


amount DangoFM::MidiToSongConverter::count_midi_events(uint8* events, int dataSize)
{
  if (VERBOSE)
  {
    printf("Counting %d bytes of events\n", dataSize);
    printf("Byte\tTime\tEvent\tCh\n");
  }

  int32 masterTime = 0;
  int32 timeStamp = 0;
  bool running_status = false;
  uint8 last_event_byte = 0;

  struct ChannelEvent channelEvent;

  int eventCount = 0;
  byteindex byteIndex = 0;
  byteindex byteIndexBefore = 0;

  if (VERBOSE){
    printf("Byte\tEvent\n");
  }
  while (byteIndex < dataSize) {
    // Increases byteIndex

    byteIndexBefore = byteIndex;
    timeStamp = read_time_stamp(events, &byteIndex);
    masterTime += timeStamp;

    if (VERBOSE){
      printf("%zX time %d\t%zu bytes\n", byteIndexBefore, timeStamp, byteIndex - byteIndexBefore);
    }

    uint8 eventByte = events[byteIndex];

    if (VERBOSE){
      printf("%zX : %X", byteIndex, eventByte);
    }

    bool read_channel_event = false;

    MidiByteType type = GetByteType(eventByte);
    switch(type)
    {
      case SystemMultiPartEvent:
      {
        byteIndex++;
        byteIndex = read_system_multi_part_event(events, byteIndex);
        running_status = false;
      }
      break;
      case SystemEvent:
      {
        byteIndex++;
        byteIndex = read_system_event(events, byteIndex);
        running_status = false;
      }
      break;
      case MetaEvent:
      {
        byteIndex++;
        // DELTA TIME EVENT IS IMPORTANT SINCE
        // THE BPM MIGHT NOT BE 120
        if (events[byteIndex] == META_DELTA_TIME) {
          uint32 tempo = 0;
          // + 1 is 03
          tempo += events[byteIndex + 2] << 16;
          tempo += events[byteIndex + 3] << 8;
          tempo += events[byteIndex + 4];

          int bpm = (60 * 1000000) / tempo;
          md.bpm = bpm;

          if (false)
          {
            printf("Tempo is %u usec/quarter note = %u bpm\n", tempo, bpm);
          }
        }

        byteIndex = read_meta_event(events, byteIndex);
        running_status = false;
      }

      break;
      case ChannelEvent:
      {
        byteIndex++;
        running_status = true;
        last_event_byte = eventByte;
        read_channel_event = true;
      }
      break;
      case VoiceData:
        if (running_status)
        {
          // Use the previous type and channel, don't advance read!
          eventByte = last_event_byte;
          read_channel_event = true;
        }
        break;
    }

    if (read_channel_event) {
      channelEvent = read_midi_event(eventByte, events, byteIndex);
      if (R_VERBOSE){
        printf("\tCh:%d %d|%d\n", channelEvent.channel, channelEvent.note, channelEvent.velocity);
      }
      int eventSizeBytes = get_midi_event_size(eventByte, running_status);
      byteIndex += eventSizeBytes;

      if (is_supported_event(channelEvent.type)) {
        eventCount++;
      }
    }

    if (VERBOSE) {
      for (int i = byteIndexBefore; i < byteIndex; i++) {
        printf("%X ", events[i]);
      }
      printf("\n");
    }
  }
  return eventCount;
}


DangoFM::ErrorCode DangoFM::MidiToSongConverter::read_midi_events_and_instruments(uint8* events, int eventsSizeBytes, int eventIndex)
{
  if (VERBOSE){
    printf("Reading %d bytes of events\n", eventsSizeBytes);
  }
  byteindex byteIndex = 0;
  byteindex prevByteIndex = 0;

  int32 masterTime = 0;
  int32 timeStamp = 0;
  bool running_status = false;
  uint8 last_event_byte = 0;

  struct ChannelEvent channelEvent;

  if (VERBOSE){
    printf("Byte\tTime\tEvent\n");
  }
  while (byteIndex < eventsSizeBytes) {
    prevByteIndex = byteIndex;

    // Increases byteIndex
    timeStamp = read_time_stamp(events, &byteIndex);
    masterTime += timeStamp;

    uint8 eventByte = events[byteIndex];
    bool read_channel_event = false;

    const MidiByteType type = GetByteType(eventByte);
      switch(type)
      {
        case ChannelEvent:
          running_status = true;
          last_event_byte = eventByte;
          byteIndex++;
          read_channel_event = true;
          break;

        case SystemEvent:
          byteIndex++;
          byteIndex = read_system_event(events, byteIndex);
          running_status = false;
          break;

        case SystemMultiPartEvent:
          byteIndex++;
          byteIndex = read_system_multi_part_event(events, byteIndex);
          running_status = false;
          break;

        case MetaEvent:
          byteIndex++;
          byteIndex = read_meta_event(events, byteIndex);
          running_status = false;
          break;
        case VoiceData:
          if (running_status)
          {
              // Use the previous type and channel, dont advance write
              eventByte = last_event_byte;
              read_channel_event = true;
          }
        break;
      }

    if (read_channel_event) {
      channelEvent = read_midi_event(eventByte, events, byteIndex);
      channelEvent.time = masterTime;

      int eventSizeBytes = get_midi_event_size(eventByte, running_status);
      byteIndex += eventSizeBytes;

      if (is_supported_event(channelEvent.type)) {
        if (eventIndex >= 0 && eventIndex < md.eventAmount)
        {
          md.events[eventIndex] = channelEvent;
        }
        else {
          printf("Index out of bounds: md.events[%d] > %d\n", eventIndex, md.eventAmount);
        }
        eventIndex++;
      }
      else if (channelEvent.type == EVENT_SET_INSTRUMENT)
      {
        // Set instrument event
        uint8 instrument = get_7bits_value(channelEvent.note);
        md.instrumentsArray[channelEvent.channel] = (InstrumentName)instrument;
      }
    }

    if (VERBOSE) {
      printf("-------------------------------------\n");
      printf("%u / %u  BI:%zu \t%X\n"
      , timeStamp
      , masterTime
      , byteIndex
      , eventByte);
    }

    int stampAndEventBytes = byteIndex - prevByteIndex;
    if (stampAndEventBytes == 0) {
      printf("ERROR, read 0 event bytes\n");
      return ZERO_READ;
    }
  }

  return NO_ERROR;
}

// Static helper functions
uint32 DangoFM::MidiToSongConverter::read_time_stamp(uint8 *events, byteindex *byteIndex) {
  uint32 value = read_variable_length(events, byteIndex);
  return value;
}

uint32 DangoFM::MidiToSongConverter::get_stamp_size(uint32 timestamp)
{
  uint32 bytes = 0;
  do {
    // Take 7 bits of value, store and shift left
    bytes++;
    timestamp = timestamp >> 7;
  } while (timestamp > 0);
  return bytes;
}

DangoFM::MidiByteType DangoFM::MidiToSongConverter::GetByteType(uint8 event)
{
  if (event < MULTI_BYTE)
  {
    // First byte is 0 and Not Meta or system event
    return VoiceData;
  }
  else if ((event >= MULTI_BYTE) && (event < 0xFF))
  {
    // A channel voice message and not meta or system event
    // First byte is 1, but the whole value is less than 0xFF
    return ChannelEvent;
  }
  else if (event == EVENT_TYPE_SYSTEM)
  {
    return SystemEvent;
  }
  else if (event == EVENT_TYPE_SYSTEM_MULTI)
  {
    return  SystemMultiPartEvent;
  }
  else if ( event == EVENT_TYPE_META)
  {
    return MetaEvent;
  }

  return MidiByteType::Unknown;
}

byteindex DangoFM::MidiToSongConverter::read_system_event(uint8 *events, byteindex byteIndex) {
  uint32 lengthBytes = read_variable_length(events, &byteIndex);
  // if last byte is not F7, the message continues
  if (VERBOSE) {
    printf("\tSystem event of length %d\n", lengthBytes);
  }
  uint8 lastByte = events[byteIndex + lengthBytes - 1];
  if (lastByte & EVENT_SYSTEM_END) {
    // Does not continue
    readingMultiPartEvent = false;
  }
  else {
    //continues
    readingMultiPartEvent = true;
  }
  byteIndex += lengthBytes;
  return byteIndex;
}

byteindex DangoFM::MidiToSongConverter::read_system_multi_part_event(uint8 *events, byteindex byteIndex) {
  uint32 lengthBytes = read_variable_length(events, &byteIndex);
  if (VERBOSE) {
    printf("\tSystem event part. length %d\n", lengthBytes);
  }
  if (readingMultiPartEvent) {
    // if reading multi part event and
    // last byte is not F7, the message continues
    uint8 lastByte = events[byteIndex + lengthBytes - 1];
    if (lastByte & EVENT_SYSTEM_END) {
      // Does not continue
      readingMultiPartEvent = false;
    }
  }
  byteIndex += lengthBytes;
  return byteIndex;
}

byteindex DangoFM::MidiToSongConverter::read_meta_event(uint8 *events, byteindex byteIndex) {
  uint8 type = (events[byteIndex]);
  byteIndex++;
  uint8 eventLengthBytes = read_variable_length(events, &byteIndex);

  if (VERBOSE) {
    printf("\tMeta event of type %X\n", type);
    if (eventLengthBytes > 0) {
      printf("\tVariable length meta event, length: %d\n", eventLengthBytes);
    }

    switch (type) {

    case META_SEQ_NUMBER:
      printf("Sequence Number");
      print_meta_number(events, byteIndex, eventLengthBytes);
      break;

    case META_TEXT:
      printf("Text: ");
      print_meta_text(events, byteIndex, eventLengthBytes);
      break;

    case META_COPY_NOTE:
      printf("Copyright notice: ");
      print_meta_text(events, byteIndex, eventLengthBytes);
      break;

    case META_TRACK_NAME:
      printf("Track name: ");
      print_meta_text(events, byteIndex, eventLengthBytes);
      break;

    case META_INSTRUMENT_NAME:
      printf("Instrument name: ");
      print_meta_text(events, byteIndex, eventLengthBytes);
      break;

    case META_LYRIC:
      printf("Lyrics: ");
      print_meta_text(events, byteIndex, eventLengthBytes);
      break;

    case META_MARKER:
      printf("Marker: ");
      print_meta_text(events, byteIndex, eventLengthBytes);
      break;

    case META_CUE:
      printf("Cue Point: ");
      print_meta_text(events, byteIndex, eventLengthBytes);
      break;

    case META_CHANNEL:
      printf("Cue Point: ");
      print_meta_number(events, byteIndex, eventLengthBytes);
      break;

    case META_TRACK_END:
      printf("Track ended");
      break;

    case META_DELTA_TIME:
      printf("Delta time: ");
      print_meta_number(events, byteIndex, eventLengthBytes);
      break;

    case META_SMPTE_OFFSET:
      printf("SMPTE Offset: ");
      print_meta_number(events, byteIndex, eventLengthBytes);
      break;

    case META_TIME_SIGN:
      printf("Time signature: ");
      print_meta_number(events, byteIndex, eventLengthBytes);
      break;

    case META_KEY_SIGN:
      printf("Key Signature: ");
      print_meta_number(events, byteIndex, eventLengthBytes);
      break;

    case META_SEQ_SPECIFIC:
      printf("Sequencer specific: ");
      print_meta_number(events, byteIndex, eventLengthBytes);
      break;

    default:
      printf("Unknown meta event");
      break;
    };

    printf("\n");
  }

  byteIndex += eventLengthBytes;
  return byteIndex;
}


void DangoFM::MidiToSongConverter::print_instruments(InstrumentName *instrumentsArray, int activeChannels) {
  printf("Instruments:\n");
  for (int c = 0; c < activeChannels; c++) {
    int inst = instrumentsArray[c];
    printf("%d : %s(%d)\n", c, DangoFM::InstrumentNames::get_instrument_name(inst).c_str(), inst);
  }
  printf("\n");
}

void DangoFM::MidiToSongConverter::print_channel_event(struct DangoFM::ChannelEvent event) {
  printf("%d", event.channel);
  int8 note = event.note;

  // Two bytes of data
  switch(event.type) {
    case EVENT_NOTE_ON: {
    uint8 velocity = event.velocity;
    if (velocity == 0) {
      event.type = EVENT_NOTE_OFF;
      printf("\tNote OFF %d\n", note);
    }
    else {
      printf("\tNote ON %d velocity %u\n", note, velocity);
    }
  }
    break;
    case EVENT_NOTE_OFF:
    printf("\tNote OFF %d\n", note);
    break;

    case EVENT_SET_INSTRUMENT:
    printf("\tSet instrument to %d: %s\n", note, DangoFM::InstrumentNames::get_instrument_name(event.instrument).c_str());
    break;


    case VOICE_KEY_PRESSURE:
    printf("\tKey pressure %d \n", event.velocity);
    break;
    case VOICE_CONTROL_CHANGE:
    printf("\tControl change controller: %d value: %d\n", event.note, event.velocity);
    break;
    case VOICE_CHANNEL_PRESSURE:
    printf("\tChannel pressure %d \n", event.note);
    break;
    case VOICE_PITCH_WHEEL:
    printf("\tPitch wheel %d\n", (event.note + (event.velocity >> 4) - 0x2000));
    break;
  default:
    printf("\tUnused %x %x \n", event.note, event.velocity);
    break;
  };
}

void DangoFM::MidiToSongConverter::print_meta_text(uint8* events, int byteIndex, int eventLengthBytes) {
  for (int b = 0; b < eventLengthBytes; b++) {
      printf("%c", events[byteIndex + b]);
    }
}

void DangoFM::MidiToSongConverter::print_meta_number(uint8* events, int byteIndex, int eventLengthBytes) {
  for (int b = 0; b < eventLengthBytes; b++) {
    printf("%d ", events[byteIndex + b]);
  }
}

struct DangoFM::ChannelEvent DangoFM::MidiToSongConverter::read_midi_event(uint8 eventByte, uint8* events, byteindex byteIndex)
{
  struct ChannelEvent channelEvent;
  // Bit order is:
  // [1eee cccc]
  uint8 channel = (eventByte & 0xF);
  uint8 eventType = eventByte >> 4;

  // Byte 1: Note / Instrument number
  // Byte 2: Velocity
  uint8 note = events[byteIndex];
  byteIndex++;
  uint8 velocity = 0;

  // Two bytes of data
  switch(eventType) {

    case EVENT_NOTE_ON: {
      velocity = events[byteIndex];
      byteIndex++;
      if (velocity == 0) {
        // This is actually a note off event, haha!
        eventType = EVENT_NOTE_OFF;
      }
    }
    break;

    case EVENT_NOTE_OFF: {
      velocity = 0;
      byteIndex++;
    }
    break;

    case EVENT_SET_INSTRUMENT:
      channelEvent.velocity = note; // Instrument value to velocity
      break;

      // These events are not supported but have 2 bytes of data
    case VOICE_KEY_PRESSURE:
    case VOICE_CONTROL_CHANGE:
    case VOICE_PITCH_WHEEL:
      velocity = events[byteIndex];
      byteIndex++;
      break;

      // These events are not supported and only have 1 byte of data
    case VOICE_CHANNEL_PRESSURE:
      break;
  };

  channelEvent.type = eventType;
  channelEvent.channel = channel;
  channelEvent.note = note;
  channelEvent.velocity = velocity;
  channelEvent.time = 0;

  return channelEvent;
}
int DangoFM::MidiToSongConverter::get_midi_event_size(uint8 eventByte, bool running_status) {
  uint8 eventType = eventByte >> 4;
  int size = 0;

  switch (eventType)
    {
    case EVENT_NOTE_ON:
    case EVENT_NOTE_OFF:
    case VOICE_KEY_PRESSURE:
    case VOICE_CONTROL_CHANGE:
    case VOICE_PITCH_WHEEL:
      size = 3;
      break;
    default:
      size = 2;
      break;
    };

  if (running_status) {
    size -= 1;
  }
  return size;
}
bool DangoFM::MidiToSongConverter::is_supported_event(uint8 event_type)
{
  switch(event_type) {
  case EVENT_NOTE_ON:
  case EVENT_NOTE_OFF:
    return true;
    break;
  default:
    return false;
    break;
  };
}

void DangoFM::MidiToSongConverter::sort_by_channel(struct ChannelEvent* events, int eventAmount)
{
  int channel = 0;
  int sortedIndex = 0;
  int sourceIndex = 0;
  while (channel < MIDI_CHANNEL_AMOUNT) {
    while (sortedIndex < eventAmount) {
      struct ChannelEvent *event = &events[sortedIndex];
      if (event->channel <= channel) {
        // This event is in correct place
        sortedIndex++;
        continue;
      }
      else {
        // Find event that has correct channel
        bool found = false;
        sourceIndex = sortedIndex + 1;
        while (sourceIndex < eventAmount) {
          struct ChannelEvent *sourceEvent = &events[sourceIndex];
          if (sourceEvent->channel == channel) {
            found = true;
            // Switch
            struct ChannelEvent move = *event;
            events[sortedIndex] = events[sourceIndex];
            events[sourceIndex] = move;
            break;
          }
          sourceIndex++;
        }
        if (!found) {
          // Event with correct channel was not found
          // change to next channel
          break;
        }
      }
    } // sortedIndex == eventAmount
    channel++;
  } // channel == MIDI_CHANNELS
}

// Should be sorted by channels before calling this
void DangoFM::MidiToSongConverter::renumber_channels(struct ChannelEvent* events, int eventAmount, InstrumentName* instrumentsArray)
{
  int activeChannel = -1;
  int workingChannel = -1;
  for (int eventIndex = 0; eventIndex < eventAmount; eventIndex++) {
    struct ChannelEvent *event = &events[eventIndex];

    // Channel increases
    if (event->channel > workingChannel) {
      workingChannel = event->channel;
      activeChannel++;

      // TODO What happens here?
      // Set instrument of this channel
      // Default instrument is 1 (Grand piano)
      if (workingChannel != activeChannel) {
        instrumentsArray[activeChannel] = instrumentsArray[workingChannel];
        instrumentsArray[workingChannel] = NoInstrument;
      }
      if (instrumentsArray[activeChannel] == NoInstrument) {
        instrumentsArray[activeChannel] = NoInstrument;
      }

    }
    event->channel = activeChannel;
  }
}

void DangoFM::MidiToSongConverter::sort_by_time(struct ChannelEvent* events, int eventAmount)
{
  // Bubble sort inside each channel
  // if time is same, order is OFF, Instrument change, ON
  // the numerical values of these events is not same as
  // the wanted order
  int channel = 0;
  int smallest = 0;
  int sortedIndex = 0;
  int sourceIndex = 0;

  while (channel < MIDI_CHANNEL_AMOUNT) {
    while (sortedIndex < eventAmount) {
      struct ChannelEvent *event = &events[sortedIndex];
      if (event->channel != channel) {
        break;
      }
      //  printf("I:%d : %d <= %d ?\n", sortedIndex, event->time, smallest);
      if (event->time <= smallest) {
        // This event is in correct place
        sortedIndex++;
        continue;
      }
      else {
        // Find event that has lower or same time
        bool found = false;
        int smallestFound = 1000000;
        sourceIndex = sortedIndex + 1;
        while (sourceIndex < eventAmount) {
          struct ChannelEvent *sourceEvent = &events[sourceIndex];
          int sourceTime = sourceEvent->time;
          if (sourceEvent->channel != channel) {
            // No smaller time on this channel
            break;
          }
          else if (sourceTime<= smallest) {
            //     printf("%d: %d < %d\n", sourceIndex, sourceEvent->time, smallest);
            found = true;
            // Switch
            struct ChannelEvent move = *event;
            events[sortedIndex] = events[sourceIndex];
            events[sourceIndex] = move;
            break;
          }
          if (sourceTime < smallestFound) {
            smallestFound = sourceTime;
          }
          sourceIndex++;
        }
        if (!found) {
          // Event with lesser or same time was not found
          // accept event time as the smallest
          // printf("No smaller time than %d, only %d\n", smallest, smallestFound);
          smallest = smallestFound;
        }
      }
    } // sortedIndex == eventAmount
    channel++;
    smallest = 0;
    sourceIndex = sortedIndex;
  } // channel == MIDI_CHANNELS


  // Sort order of events with same time
  sortedIndex = 0;
  while (sortedIndex < eventAmount) {
    struct ChannelEvent *prevEvent = &events[sortedIndex];
    sourceIndex = sortedIndex + 1;
    if (sourceIndex < eventAmount) {
      struct ChannelEvent *nextEvent = &events[sourceIndex];

      bool order = false;
      if (prevEvent->channel != nextEvent->channel) {
        // NOP
      }
      else if (prevEvent->time != nextEvent->time) {
        // NOP
      }
      else if (prevEvent->type == EVENT_NOTE_OFF) {
        // NOP
      }
      else if (prevEvent->type == EVENT_NOTE_ON
               && nextEvent->type == EVENT_NOTE_OFF) {
        order = true;
      }

      if (order) {
        struct ChannelEvent move = *prevEvent;
        events[sortedIndex] = events[sourceIndex];
        events[sourceIndex] = move;
        // sorted index stays same until correct order
        continue;
      }
    }
    sortedIndex++;
  } // sortedIndex == eventAmount
}
void DangoFM::MidiToSongConverter::change_times_to_relative(struct ChannelEvent* events, int eventAmount, int channels)
{
  for (int channel  = 0; channel <= channels; channel++) {
    int time = 0;
    int prevTime = 0;

    for (int index = 0; index < eventAmount; index++) {
      struct ChannelEvent *event = &events[index];
      if (event->channel < channel) {
        continue;
      }
      else if (event->channel > channel) {
        break;
      }
      time = event->time;
      event->time -= prevTime;

      prevTime = time;
    }
  }
}

// All channels start from note 0
// and stored notes are offsets.
void DangoFM::MidiToSongConverter::change_notes_to_relative(struct ChannelEvent* events, int eventAmount, int channels)
{
  printf("Changing notes to relative on %d channels\n", channels);
  for (int channel  = 0; channel <= channels; channel++) {
    int note;
    int prevNote = 0;

    for (int index = 0; index < eventAmount; index++) {
      struct ChannelEvent *event = &events[index];
      if (event->channel < channel) {
        continue;
      }
      else if (event->channel > channel) {
        break;
      }
      note = event->note;
      event->note -= prevNote;

      prevNote = note;
    }
  }
}
