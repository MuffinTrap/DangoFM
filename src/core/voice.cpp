#include "voice.h"

void DangoFM::Voice::On(uint8 note, real volume, Envelope& carrier, Envelope& modulator)
{
	this->note = note;
	this->volume = volume;


	carrierState.phase = 0.0f;
	carrierState.feedback = 0.0f;
	carrierState.state = EnvelopeState::Attack;
	carrierState.slope = carrier.GetSlopeAt(carrierState.state);
	carrierState.level = 0.0f;

	modulatorState.phase = 0.0f;
	modulatorState.feedback = 0.0f;
	modulatorState.state = EnvelopeState::Attack;
	modulatorState.slope = modulator.GetSlopeAt(modulatorState.state);
	modulatorState.level = 0.0f;

	noteOffTime = 0;
	elapsed = 0;

	noteOn = true;
}

void DangoFM::Voice::Off()
{
	noteOffTime = elapsed;
	noteOn = false;

}
DangoFM::OperatorState::OperatorState()
{
	phase = 0.0f;
	feedback = 0.0f;
	level = 0.0f;
	slope.targetLevel = 0.0f;
	slope.increment = 0.0f;
	state = EnvelopeState::End;
}


// returns true on state change
bool DangoFM::OperatorState::Advance(bool debug)
{
  real lb = level;
  level += slope.increment * slope.advanceSpeed;
  real la = level;

  //printf("%.2f -> %.2f s(%.2f)\n", lb, la, slope.advanceSpeed);
  bool reachedTarget = false;
  if (state == Attack)
  {
	  if (level >= slope.targetLevel)
	  {
		reachedTarget = true;
	  }
  }
  else
  {
	  if (level <= slope.targetLevel)
	  {
		reachedTarget = true;
	  }
  }

  if (reachedTarget)
  {
    // Advance to next state
    level = slope.targetLevel;
    switch(state)
    {
      case EnvelopeState::Attack:
        state = Decay1;
        break;
      case EnvelopeState::Decay1:
        state = Decay2;
        break;
      case EnvelopeState::Decay2:
        state = Release;
        break;
      case EnvelopeState::Release:
        state = End;
        break;
      case EnvelopeState::End:
        // Early return
        return false;
        break;
    }
  }
  return reachedTarget;
}

