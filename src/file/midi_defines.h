#pragma once
// Note ON and OFF are in core/defines.h
// because driver uses them
#define EVENT_SET_INSTRUMENT 0xC

// System event. Variable length, bytes, ends with 0xF7
#define EVENT_TYPE_SYSTEM 0xF0

// Continue events can only happend
// directly after System event which did
// not end in F7 itself
// An independent event can also start with F7, followed by variable length and bytes, ends with 0xF7
#define EVENT_TYPE_SYSTEM_MULTI 0xF7
#define EVENT_SYSTEM_END 0xF7

#define EVENT_TYPE_META 0xFF

#define META_SEQ_NUMBER 0x0
#define META_TEXT 0x1
#define META_COPY_NOTE 0x2
#define META_TRACK_NAME 0x3
#define META_INSTRUMENT_NAME 0x4
#define META_LYRIC 0x5
#define META_MARKER 0x6
#define META_CUE 0x7

#define META_CHANNEL 0x20
#define META_TRACK_END 0x2F // < Important

#define META_DELTA_TIME 0x51 // Important, gives beats per minute
#define META_SMPTE_OFFSET 0x54
#define META_TIME_SIGN 0x58
#define META_KEY_SIGN 0x59

#define META_SEQ_SPECIFIC 0x7F

#define VOICE_KEY_PRESSURE 0xA
#define VOICE_CONTROL_CHANGE 0xB
#define VOICE_PROGRAM_CHANGE 0xC
#define VOICE_CHANNEL_PRESSURE 0xD
#define VOICE_PITCH_WHEEL 0xE

#define EVENT_NOTE_OFF 0x8
#define EVENT_NOTE_ON 0x9

#define MULTI_BYTE 0x80
#define BYTE_VALUE 127

#define MIDI_CHANNEL_AMOUNT 16

// Channel event size in bytes
#define CHANNEL_EVENT_SIZE 2

