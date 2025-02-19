
#include "synth.h"
#include "synth_definitions.h"
#include <cmath>


void DangoFM::Synth::Init()
{
  channels = new SynthChannel[DANGO_CHANNEL_AMOUNT];
  for (int c = 0; c < DANGO_CHANNEL_AMOUNT; c++)
  {
    channels[c].Reset();
  }
  InitDefaultInstruments();
}

void DangoFM::Synth::InitDefaultInstruments()
{
	Instrument default_instrument = CreateDefaultInstrument();
	for (int c = 0; c < DANGO_CHANNEL_AMOUNT; c++)
	{
		channels[c].instrument = default_instrument;
	}
}

void DangoFM::Synth::InitSongInstruments(Instrument* instruments, uint8 amount)
{
	// TODO create Instruments from SongInstruments
}

DangoFM::Instrument DangoFM::Synth::CreateDefaultInstrument()
{
  Instrument i;
    Envelope defEnv = Envelope(0.9f, 0.9f, 0.3f, 0.0f, 0.9f);
  Operator defCarrier = Operator(Sin, 0.5f, 0, defEnv);
  Operator defMod = Operator(Sin, 0.0f, 0, defEnv);
  Instrument defaultInst = Instrument(defCarrier, defMod);

  defaultInst.name = "<Default>";
  defaultInst.enumName = AcousticPiano;

  return defaultInst;
}

void DangoFM::Synth::GenerateAudio_V3(uint8 channelNumber, AudioBuffer buffer, uint16 sampleAmount)
{
  SynthChannel& ch = channels[channelNumber];
  Instrument& instrument = ch.instrument;
  for(int i = 0; i < ch.activeVoices; i++)
  {
    Voice& V = ch.voices[i];
    instrument.TuneToNote(V.note);
    Operator& Carrier = instrument.Carrier;
    Envelope& C_Envelope = Carrier.GetVolumeEnvelope();

    Operator& Modulator = instrument.Modulator;
    real ModAmpl = Freq2Rad * (Modulator.GetLevel() * (float)modulationIndex) * Modulator.frequenzyHz;
    Envelope& M_Envelope = Modulator.GetVolumeEnvelope();

    std::function<real(real)> C_func = Carrier.GetWaveFunction();
    std::function<real(real)> M_func = Modulator.GetWaveFunction();

    real sample = 0.0f;
    ch.peakValue = 0.0f;

    // NOTE: n is used as index to the buffer!
    for(SampleCount n = 0; n < sampleAmount; n++)
    {
      sample = V.volume * V.carrierState.level * C_func(V.carrierState.phase) * Carrier.GetLevel();
      if( SDL_fabsf(sample) > ch.peakValue)
      {
        ch.peakValue = SDL_fabsf(sample);
      }
      buffer[n] = sample;

      real modulationAmount = V.modulatorState.level * M_func(V.modulatorState.phase) + V.modulatorState.feedback * Modulator.GetFeedBack();
      V.modulatorState.feedback = modulationAmount;

      V.carrierState.phase += Carrier.phaseIncrement + modulationAmount * ModAmpl;
      V.modulatorState.phase += Modulator.phaseIncrement;

      UpdateOperatorState(V.carrierState, C_Envelope);
      UpdateOperatorState(V.modulatorState, M_Envelope);
    }

    // NOTE is this happens inside the sample loop, will swap to invalid voice
    if (V.carrierState.state == EnvelopeState::End)
    {
      ch.StopVoice(i);
    }
    else if (V.noteOn == false)
    {
      V.carrierState.state = Release;
      V.modulatorState.state = Release;
      V.carrierState.slope = C_Envelope.GetSlopeAt(Release);
      V.modulatorState.slope = M_Envelope.GetSlopeAt(Release);
    }
  }
}

void DangoFM::Synth::UpdateOperatorState(DangoFM::OperatorState& state, DangoFM::Envelope& E)
{
  if (state.phase >= TAU)
  {
    state.phase -= TAU;
  }

  if (state.Advance())
  {
    state.slope = E.GetSlopeAt(state.state);
  }
}

// From BasicSynth Book
void DangoFM::Synth::GenerateSamples_V1(uint8 channelNumber, AudioBuffer buffer, uint16 sampleAmount)
{
#if 0
  // 1. Initialize for sound generation
  // Get instrument
  SynthChannel& ch = channels[channelNumber];
  Instrument& instrument = ch.instrument;

  // 2. For each voice
  for(int i = 0; i < ch.activeVoices; i++)
  {
    Voice& V = ch.voices[i];
    // Set phase of oscillators using elapsedSamples
    // Set envelope states using elapsedSamples
    SampleCount duration = V.elapsed;

    instrument.TuneToNote(V.note);

    Operator& Carrier = instrument.Carrier;
    Envelope& Cenv = Carrier.GetVolumeEnvelope();

    Operator& Modulator = instrument.Modulator;
    real ModAmpl = Freq2Rad * (Modulator.level * (float)modulationIndex) * Modulator.frequenzyHz;
    Envelope& Menv = Modulator.GetVolumeEnvelope();
    EnvelopeStatus Mev = Menv.CalculateStatus(V, V.volume);

  // TODO Check from Envelope Cutoff
    real Mvalue = Mev.value;

    // Maximum possible volume
    const real voiceVolume = V.volume * Carrier.level * ch.volume;
    EnvelopeStatus Cev = Cenv.CalculateStatus(V, V.volume);
    real volume = Cev.value;

    // Continue modulation from previous frame
    real Cphase = V.carrierPhase;
    real Cincr = Carrier.GetPhaseIncrement();

    real Mphase = V.modulatorPhase;
    real Mincr = Modulator.GetPhaseIncrement();
    real Mfeedback = V.modulatorFeedback;


    // NOTE: n is used as index to the buffer!
    for(SampleCount n = 0; n < sampleAmount; n++)
    {
      // 3. For each sample
      // Advance envelopes
      // Get oscillator value
      // Multiply by envelope value
      // Store sample to buffer
      // TODO use std::function to abstract wave generation
      volume = GetEnvelopeValue_V2(Cev, V);
      if (Cev.state == EnvelopeState::End)
      {
        ch.StopVoice(i);
      }
      buffer[n] = volume * SDL_sin(Cphase);

      Mvalue = GetEnvelopeValue_V2(Mev, V);
      real modulationAmount = Mvalue * SDL_sin(Mphase) + Mfeedback * Modulator.feedBack;
      Mfeedback = modulationAmount;

      // Advance oscillators

      Cphase += Cincr + modulationAmount * ModAmpl;
      Mphase += Mincr;
      if (Cphase >= TAU) { Cphase -= TAU; }
      if (Mphase >= TAU) { Mphase -= TAU; }

      // Increase duration
      duration++;
    }

    // 4. After generation
    // Update elapsed samples of voice
    V.elapsed += sampleAmount;
    // Store operator phases
    V.carrierPhase = Cphase;
    V.modulatorPhase = Mphase;
    V.modulatorFeedback = Mfeedback;
  }
#endif
}
real DangoFM::Synth::GetEnvelopeValue_V2(EnvelopeStatus& E, Voice& V)
{
#if 0
  // Noteoff can happen at any time
  if (V.noteOn == false)
  {
    E.state = EnvelopeState::Release;
    E.value -= E.releaseRate;
    if (E.value < 0.0f)
    {
      E.value = 0.0f;
    }
    return E.value;
  }

  switch(E.state)
  {
    case EnvelopeState::Attack:
      if (V.elapsed < E.attackTime)
      {
        E.value += E.increment;
      }
      else
      {
        E.value = 1.0f;
        E.increment = 1.0f/(float)E.decay1Time;
        E.state = EnvelopeState::Decay1;
      }
      break;
    case EnvelopeState::Decay1:
      if (V.elapsed < E.attackTime + E.decay1Time)
      {
        E.value -= E.increment;
      }
      else
      {
        if (E.sustainLevel > 0.0f)
        {
          E.value = E.sustainLevel;
          E.increment = E.sustainLevel/(float)E.decay2Time;
          if (E.decay2Time == 0)
          {
            E.state = EnvelopeState::Sustain;
          }
          else
          {
            E.state = EnvelopeState::Decay2;
          }
        }
        else
        {
          E.state = EnvelopeState::End;
          E.value = 0.0f;
        }
      }
      break;
    case EnvelopeState::Sustain:
      // NOP, wait till note off
      break;
    case EnvelopeState::Decay2:
    {
      E.value -= E.increment;
    }
    break;
    case EnvelopeState::Release:
    case EnvelopeState::End:
      // Nop, handled in the beginning
      break;
  };

  return E.value;
#endif
  return 0.0f;
}



void DangoFM::Synth::FillBufferFromChannel_V2(uint8 channelNumber, AudioBuffer buffer, uint16 sampleAmount)
{
#if 0
  #ifdef DANGO_EDITOR
  if (channelNumber >= DANGO_CHANNEL_AMOUNT) {
    printf("Cannot generate for channel %d !\n", channel);
    return;
  }
  printf("Fillbuffer\n");
  #endif
  SynthChannel& ch = channels[channelNumber];
  Instrument& instrument = ch.instrument;
  // Generate samples
  for(int v = 0; v < DANGO_VOICE_AMOUNT; v++)
  {
    const int note = ch.notes[v];
    if (note == 0) {
      continue;
    }

    // All voices start writing from the same point in the output buffer
    real *sampleOut = buffer;
    float peakVolume = VelocityToVolume(ch.velocities[v]);
    uint32 noteDuration = ch.durationsSamples[v];
    int32 noteOffTime = ch.noteOffTimes[v];

    instrument.TuneToNote(note, noteDuration);
    for (int s = 0; s < sampleAmount; ++s) { // Must be FOR()

      // Update operators
      real amplitude = instrument.UpdateOperators(noteDuration + s, noteOffTime);
      if (std::isnan(amplitude)) printf("Amp NaN\n");

      // Check if combined amplitude output is under amplitude cutoff
      if (noteOffTime > 0 && amplitude < DANGO_SYNTH_AMPLITUDE_CUTOFF) {
        ch.StopVoice(v);
        break;
      }

      // Do the FM synthesis using the operators
      // real sampleValue = run_patch_function_BS(instrument[PatchFunction], operators);
      // sampleValue *= peakVolume;


      if (std::isnan(peakVolume)) printf("peakVolume NaN\n");
      if (std::isnan(sampleValue)) printf("SampleValue NaN\n");

      // Stereo or mono
      *sampleOut += sampleValue;
      ++sampleOut;
      #if DANGO_OUT_CHANNELS == 2
      *sampleOut += sampleValue;
      ++sampleOut;
      #endif

      if (sampleValue > ch.lastValue)
      {
        ch.lastValue = sampleValue;
      }


      #ifdef DANGO_EDITOR
      ch.volumes[v] = (sampleValue < 0.0f ? (sampleValue * -1.0f) : sampleValue);
      if (s % DEBUG_INTERVAL == 0 && SMOL_DEBUG_ENV) {
        //printf("waveValue: %.2f -> out: %f\n" , waveValue,  sampleValue);
      }
      #endif
    } // End sample generation for a voice

    ch.durationsSamples[v] += sampleAmount;
  }
#endif
}




void DangoFM::Synth::SetNoteRelative(uint8 channelNumber, int8 noteRelative, uint8 velocity)
{
	#ifdef DANGO_EDITOR
	if (channelNumber >= DANGO_CHANNEL_AMOUNT) {
		return;
	}
	// TODO GLissando effect
	#endif
	SynthChannel& ch = channels[channelNumber];
	ch.SetNoteRelative(noteRelative, velocity);
}

void DangoFM::Synth::NoteOn(uint8 channelNumber, uint8 note, uint8 velocity)
{
	#ifdef DANGO_EDITOR
	if (channelNumber >= DANGO_CHANNEL_AMOUNT) {
		return;
	}
	#endif

	SynthChannel& ch = channels[channelNumber];
	ch.NoteOn(note, velocity);
}

void DangoFM::Synth::NoteOff(uint8 channelNumber, uint8 note)
{
	#ifdef DANGO_EDITOR
	if (channel >= DANGO_CHANNEL_AMOUNT) {
		return;
	}
	#endif

	SynthChannel& ch = channels[channelNumber];
	ch.NoteOff(note);
}

void DangoFM::Synth::StopVoice(uint8 channelNumber, uint8 voice)
{
	#ifdef DANGO_EDITOR
	if (channelNumber >= DANGO_CHANNEL_AMOUNT) {
		printf("invalid channel %d\n", channelNumber);
		return;
	}
	#endif
	SynthChannel& ch = channels[channelNumber];
	ch.StopVoice(voice);
}

void DangoFM::Synth::StopAllVoices()
{
  for (int c = 0; c < DANGO_CHANNEL_AMOUNT; c++)
  {
    SynthChannel& ch = channels[c];
    for (int v = 0; v < ch.activeVoices; v++)
    {
      ch.StopVoice(v);
    }
  }
}


float DangoFM::Synth::VelocityToVolume(uint8 velocity)
{
  return (float)velocity/127.0f;
}
 DangoFM::SynthChannel * DangoFM::Synth::GetChannel(uint8 channelNumber)
{
  if (channelNumber < DANGO_CHANNEL_AMOUNT)
  {
  return &channels[channelNumber];
  }
  return nullptr;
}





