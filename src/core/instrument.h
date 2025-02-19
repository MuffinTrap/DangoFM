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
	};
}
