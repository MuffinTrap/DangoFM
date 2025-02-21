
#include "operator.h"
#include <SDL_stdinc.h>
#include "synth_definitions.h"

real* DangoFM::Operator::SinTable;

static real generate_sin(real wavePeriod);
static real generate_square(real wavePeriod);
static real generate_saw(real wavePeriod);
static real generate_triangle(real wavePeriod);
DangoFM::Operator::Operator()
{
  this->wave = Waveform::Sin;
  this->level = 1.0f;
  this->tuningSemitones = 1;
  phase = 0;
  phaseIncrement = 0.0f;
}


DangoFM::Operator::Operator(DangoFM::Waveform wave, real power, int16 tuningSemitones, Envelope e)
{
  this->wave = wave;
  this->level = power;
  this->tuningSemitones = tuningSemitones;
  volumeEnvelope = e;
  phase = 0.0f;
  phaseIncrement = 0.0f;
}

 int16 DangoFM::Operator::ModulatorFreqMultiplierToSemitones()
{
  switch(freqMultiplier)
  {
    case 0:
      return -Octave;
      break;
    case 1:
      return Unison;
      break;
    case 2:
      return Octave;
      break;
    case 3:
      return Octave + P5th;
      break;
    case 4:
      return Octave * 2;
      break;
    case 5:
      return Octave * 2 + M3erd;
      break;
    case 6:
      return Octave * 2 + P5th;
      break;
    case 7:
      return Octave * 2 + min7th;
      break;
    case 8:
      return Octave * 3;
      break;
    case 9:
      return Octave * 3 + M2nd;
      break;
    case 10:
      return Octave * 3 + M3erd;
      break;
    case 11:
      return Octave * 3 + P5th;
    case 12:
      return Octave * 3 + M7th;
      break;
  };
  return Unison;
}


real DangoFM::Operator::NoteToHz(int note)
{
  /* Input note: 0-127
 * Output: C0 - C9
 */

  // TODO adjust by tuning
  real pitch_in_hz = 440.0f * SDL_pow(2.0f, ( float(note) - 45.0f ) / 12.0f);
  return pitch_in_hz;
}

DangoFM::Envelope & DangoFM::Operator::GetVolumeEnvelope()
{
  return volumeEnvelope;
}


void DangoFM::Operator::TuneToNote(uint8 note)
{
  int adjusted = note + ModulatorFreqMultiplierToSemitones();
  frequenzyHz = NoteToHz(adjusted);
  // Calculate offset from C0
  float offsetOctaves = (float)note/12.0f;

  // This must always be positive
  volumeEnvelope.advanceSpeed = 1.0f + offsetOctaves * volumeEnvelope.advanceSpeedOffset;

  phaseIncrement = Freq2Rad * frequenzyHz;
}

void DangoFM::Operator::SetLevel(float editorValue)
{
  level = editorValue * editorValue;
}

real DangoFM::Operator::GetLevel()
{
  return level;
}

real DangoFM::Operator::GetLevelEditorValue()
{
  return SDL_sqrt(level);
}
real DangoFM::Operator::GetFeedBackEditorValue()
{
  return SDL_sqrt(feedBack);
}

void DangoFM::Operator::SetWaveform(DangoFM::Waveform form)
{
  wave = form;
}


void DangoFM::Operator::SetFeedback(float editorValue)
{
  feedBack = editorValue * editorValue;
}

real DangoFM::Operator::GetFeedBack()
{
  return feedBack;
}


real DangoFM::Operator::GetPhase()
{
  return phase;
}

real DangoFM::Operator::GetPhaseIncrement()
{
  return phaseIncrement;
}

std::function<real (real)> DangoFM::Operator::GetWaveFunction()
{
  switch(wave) {
  case Square:
    return generate_square;
    break;
  case Saw:
    return generate_saw;
    break;
  case Triangle:
    return generate_triangle;
    break;
  case Sin:
  default:
    return generate_sin;
    break;
  };
}

real DangoFM::Operator::PC_Sin(real wavePeriod)
{
  const real waveTau = (wavePeriod/TAU);
  const size_t i = (size_t)SDL_floorf( (waveTau)*SineResolution );
  const real sinvalue = SinTable[i];
  /*
  if (sinvalue < -1.0f || sinvalue > 1.0f)
  {
    printf("Clipping Sin sample: %zu: %f from wave %f\n", i, sinvalue, wavePeriod);
  }
  */
  return SinTable[i];
}

#define SIN_FUNC DangoFM::Operator::PC_Sin

real generate_sin(real wavePeriod)
{
  return SIN_FUNC(wavePeriod);
}

// TODO: Compiler define to use Fourier versions
real generate_triangle(real wavePeriod)
{
  return SDL_asin(SIN_FUNC(wavePeriod)) * (2.0 / M_PI);
}

real generate_saw(real wavePeriod)
{
  return (wavePeriod / M_PI) - 1.0f;
}

real generate_square(real wavePeriod)
{
  return (SIN_FUNC(wavePeriod) >= 0.0f ? 1.0f : -1.0f);
}

void DangoFM::Operator::PrecalculateSines()
{
  SinTable = new real[SineResolution];
  real step = TAU / (float)SineResolution;

  size_t i = 0;
  for(real a = 0.0f; a < TAU && i < SineResolution; a += step)
  {

    SinTable[i] = SDL_sin(a);
    i++;
  }
}

#undef SIN_FUNC
