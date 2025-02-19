#include "envelope.h"
#include "synth_definitions.h"

static const real max_value = 127.0f;
DangoFM::Envelope::Envelope()
{
	rates[Attack]=0.9f;
	rates[Decay1]=0.9f;
	rates[Decay2]=0.9f;
	rates[Release]=0.9f;
	sustainLevel = 0.1f;

}

DangoFM::Envelope::Envelope(real attackRate, real decay1Rate, real sustainLevel, real decay2Rate, real releaseRate)
{
	rates[Attack]=attackRate;
	rates[Decay1]=decay1Rate;
	rates[Decay2]=decay2Rate;
	rates[Release]=releaseRate;
	this->sustainLevel = sustainLevel;
}

DangoFM::EnvelopeSlope DangoFM::Envelope::GetSlopeAt(DangoFM::EnvelopeState state)
{
	EnvelopeSlope s;
	s.increment = 0.0f;
	s.targetLevel = 0.0f;
	if (state != EnvelopeState::End)
	{

		real time = (1.0f-rates[state]) * DANGO_SAMPLES_PER_SECOND;
		s.increment = 1.0f/time;
		switch(state)
		{
			case Attack: s.targetLevel = 1.0f; break;
			case Decay1: s.targetLevel = sustainLevel; break;
			case Decay2: s.targetLevel = 0.0f;
			if (rates[Decay2] == 0.0f)
			{
				// Infinite sustain
				s.increment = 0.0f;
			}
			break;
			case Release: s.targetLevel = 0.0f; break;
			case End: s.targetLevel = 0.0f; break;
		}
		if (state != Attack)
		{
			s.increment *= -1.0f;
		}
	}
	//printf("EnvelopeSlope at %d incr: %.4f, target %.4f\n", state, s.increment, s.targetLevel);
	return s;
}


// NOTE: ASD envelope
// This is called on each audio callback
#if 0
DangoFM::EnvelopeStatus DangoFM::Envelope::CalculateStatus(const Voice& v, real previousValue)
{
	EnvelopeStatus val;
	val.releaseRate = releaseRate;
	if (v.noteOn)
	{
		// NOTE! AttackRate is given as [0, 1] where 1 means fastest attack
		// but in this calculation the fasttest attack is 0 -> attack takes no time at all
		val.attackTime = (1.0-attackRate) * DANGO_SAMPLES_PER_SECOND;
		if (val.attackTime == 0)
		{
			val.attackTime = 1;
		}

		// If sustainLevel is 1.0f, decayTime 1 is 0
		// If sustainLevel is 0.0f, decayTime is same as attackTime
		val.decay1Time = (1.0f - sustainLevel) * (1.0f - decay1Rate) * DANGO_SAMPLES_PER_SECOND;
		if (val.decay1Time == 0)
		{
			val.decay1Time = 1;
		}

		val.sustainLevel = sustainLevel;

		// Decay2 is from sustain level
		// to end.
		// If Decay2Rate is 0.0f, sustain is as long
		// as key is pressed
		// If sustain level is 0.0f, decay2 is 0.0
		// If sustain level is 1.0f, decay2 is same as attack time
		if (decay2Rate > 0.0f)
		{
			val.decay2Time = (sustainLevel) * (1.0f - decay2Rate) * DANGO_SAMPLES_PER_SECOND;
			if (val.decay2Time == 0)
			{
				val.decay2Time = 1;
			}
		}
		else
		{
			val.decay2Time = 0;
		}


		//// Advance on the envelope to current sample

		// Start from 0 samples
		real peakAmplitude = 1.0f;
		if (v.elapsed < val.attackTime)
		{
			// in attack phase
			val.increment = peakAmplitude/(float)val.attackTime;
			val.value = val.increment * v.elapsed;
			val.state = EnvelopeState::Attack;
		}
		else
		{
			// Attack is over
			SampleCount decay1End = val.attackTime + val.decay1Time;
			if (v.elapsed >= val.attackTime && v.elapsed < decay1End)
			{
				// In decay 1
				val.increment = peakAmplitude/(float)val.decay1Time;
				//                                          how many samples into decay1
				val.value = peakAmplitude - val.increment * (v.elapsed - val.attackTime);
				val.state = EnvelopeState::Decay1;
			}
			else if (v.elapsed >= decay1End)
			{
				// In sustain or ended
				if (val.sustainLevel == 0.0f)
				{
					// No sustain
					// should have reached zero by now
				}
				else if (val.decay2Time == 0)
				{
					val.value = val.sustainLevel;
					val.state = EnvelopeState::Sustain;
				}
				else if (val.decay2Time > 0)
				{
					SampleCount decay2End = decay1End + val.decay2Time;
					val.increment = val.sustainLevel/(float)val.decay2Time;
					//                                             how many samples into decay2
					val.value = val.sustainLevel - val.increment * (v.elapsed - decay2End);
					val.state = EnvelopeState::Decay2;
				}
			}
			if (val.value <= 0.0f)
			{
				val.value = 0.0f;
				val.state = EnvelopeState::End;
			}
		}
	}

	return val;
}
#endif

