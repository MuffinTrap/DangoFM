#pragma once
#include "operator.h"
#include "envelope.h"
#include "instrument_name.h"
#include <string>

namespace DangoFM
{
	class Instrument
	{
	public:
		Instrument();
		Instrument(Operator C, Operator M);
		void LoadFromBinary(uint8* dataPointer);
		void TuneToNote(uint8 note);

		Operator Carrier;
		Operator Modulator;

		std::string name;
		InstrumentName enumName;

		// TODO more settings
		// L.Key scale in editor
		real levelKeyScaling_Lo = 0.0f;
		real levelKeyScaling_Hi = 0.0f;
		/*
		bits 7-6 - 	causes output levels to decrease as the frequency rises/lowers from C3
		00	-	no change
		10	-	1.5 dB/8ve
		01	-	3 dB/8ve
		11	-	6 dB/8ve
		*/

		// envelope progresses faster when frequency rises
		// R.Key scale in editor
		real envelopeSpeedOffset = 0.0f;

		bool sustainOn; // When on, uses sustained release rate

		// Amplitude modulation changes volume
		real AMstrength; // How much AM changes the volume
		bool CarrierAMOn;
		bool ModulatorAMon;


		// Vibrato changes pitch
		bool vibratoOn; // When on,
		real vibratoDelay; //
		uint16 vibratoDepthCents; // Width of vibrato 14, 7 or 0  cents   PMSens
	};
}
