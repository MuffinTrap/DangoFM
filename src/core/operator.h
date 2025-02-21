#pragma once
#include "type_definitions.h"
#include "wave_enum.h"
#include "envelope.h"
#include <functional>

namespace DangoFM
{
	class Operator
	{
	public:

		Operator();
		Operator(Waveform wave, real power, int16 tuningSemitones, Envelope e);
		void TuneToNote(uint8 note);
		real GetPhase();
		real GetPhaseIncrement();

		Envelope& GetVolumeEnvelope();
		std::function<real(real)> GetWaveFunction();

		int16 ModulatorFreqMultiplierToSemitones();

		void SetLevel(float editorValue);
		real GetLevelEditorValue();
		real GetLevel();

		void SetFeedback(float editorValue);
		real GetFeedBack();
		real GetFeedBackEditorValue();

		void SetWaveform(Waveform form);

		static void PrecalculateSines();
		static const size_t SineResolution = 512;
		static real PC_Sin(real wavePeriod);

		real phase = 0.0f;
		real phaseIncrement = 0.0f;
		real frequenzyHz = 0.0f;
		int freqMultiplier = 1;
	private:
		real NoteToHz(int note);
		static real* SinTable;
		real level = 0.0f;
		real feedBack = 0.0f;

		int16 tuningSemitones = 0.0f;
		// TODO detuning the operator
		int16 detuneCents;

		Waveform wave = Waveform::Sin;
		real generate_wave();

		Envelope volumeEnvelope;
	};
}
