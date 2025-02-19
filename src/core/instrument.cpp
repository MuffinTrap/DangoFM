#include "instrument.h"
#include "envelope.h"
#include <SDL_stdinc.h>
#include "synth_definitions.h"
#include <cmath>

DangoFM::Instrument::Instrument()
{

}

DangoFM::Instrument::Instrument(DangoFM::Operator C, DangoFM::Operator M)
{
  Carrier = C;
  Modulator = M;
}

void DangoFM::Instrument::TuneToNote(uint8 note)
{
  Modulator.TuneToNote(note);
  Carrier.TuneToNote(note);
}
