#pragma once
#include "../../core/instrument.h"
#include "../../core/synth.h"
#include "../../core/driver.h"
namespace DangoFM
{
	struct OperatorSettings
	{
		real levelValue = 1.0f;
		real feedbackValue = 1.0f;
		int waveFormValue = 0;
	};
	class InstrumentWindow
	{
	public:
		void Init();
		void Draw(Synth& synth, Instrument& instrument, bool instrumentChanged = false);
		bool DrawOperator(OperatorRole role, std::string name, Operator& O);
	private:
		std::string SemitonesToIntervalDescription(int16 semitones);
		void UpdateGraph(real* graph, Envelope& envelope);
		void UpdatePreview(Synth& synth, Instrument& instrument);
		std::string intervalTexts[2];
		static const size_t graphSize = DANGO_SAMPLES_PER_SECOND/8;
		real carrierGraphPoints[graphSize];
		real modulatorGraphPoints[graphSize];
		static const size_t previewSize = DANGO_SAMPLES_PER_CALLBACK;
		real previewGraphPoints[previewSize];
		bool firstUpdate = true;

		OperatorSettings operators[2];
	};
}
