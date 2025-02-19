#include "instrument_window.h"
#include "../imgui.h"
#include <SDL_stdinc.h>

void DangoFM::InstrumentWindow::Init()
{
	intervalTexts[0] = SemitonesToIntervalDescription(0);
	intervalTexts[1] = SemitonesToIntervalDescription(0);
}


void DangoFM::InstrumentWindow::Draw(Synth& synth, DangoFM::Instrument& instrument, bool instrumentChanged)
{
	ImGui::Begin("Instrument");

	ImGui::PlotLines("Preview", previewGraphPoints, previewSize, 0.0f, NULL, -1.0f, 1.0f, ImVec2(0.0f, 80.0f));

	Operator& C = instrument.Carrier;
	bool Cchanged = DrawOperator(Carrier, "Carrier", C);
	Operator& M = instrument.Modulator;
	bool Mchanged = DrawOperator(Modulator, "Modulator", M);

	if (Cchanged || Mchanged || firstUpdate || instrumentChanged)
	{
		operators[Carrier].levelValue = C.GetLevelEditorValue();
		operators[Carrier].feedbackValue = C.GetFeedBackEditorValue();
		operators[Modulator].levelValue = M.GetLevelEditorValue();
		operators[Modulator].feedbackValue = M.GetFeedBackEditorValue();
		UpdatePreview(synth, instrument);
	}

	ImGui::End();
	firstUpdate = false;
}


bool DangoFM::InstrumentWindow::DrawOperator(OperatorRole role, std::string name, DangoFM::Operator& O)
{
	// Draw Modulator
	bool change = false;
	ImGui::PushID(role + 1);
	ImGui::Text("%s", name.c_str());

	OperatorSettings& OS = operators[role];

	if (ImGui::SliderFloat("Level", &OS.levelValue, 0.0f, 1.0f, "Level %.2f"))
	{
		change = true;
		O.SetLevel(OS.levelValue);
	}

	if (ImGui::SliderInt("Multiplier", &O.freqMultiplier, MinFreqMultiplier, MaxFreqMultiplier, "%d"))
	{
		change = true;
		intervalTexts[role] = SemitonesToIntervalDescription(O.ModulatorFreqMultiplierToSemitones());
	}
	ImGui::Text("%s", intervalTexts[role].c_str());

	if (role == OperatorRole::Modulator)
	{
		if (ImGui::SliderFloat("Feedback", &OS.feedbackValue, 0.0f, 1.0f, "%.2f"))
		{
			change = true;
			O.SetFeedback(OS.feedbackValue);
		}
	}

	// Draw Envelope
	Envelope& ampl = O.GetVolumeEnvelope();
	ImGui::Text("Volume envelope");

	real* graphPoints;
	if (role==Carrier)
	{
		graphPoints = carrierGraphPoints;
	}
	else
	{
		graphPoints = modulatorGraphPoints;
	}
	ImGui::PlotLines("Amplitude", graphPoints, graphSize, 0, NULL, 0.0f, 1.0f, ImVec2(0.0f, 80.0f));
	bool aChanged = ImGui::SliderFloat("Attack Rate", &ampl.rates[Attack], 0.0f, 1.0f, "A %.2f");
	bool d1Changed = ImGui::SliderFloat("Decay 1 Rate", &ampl.rates[Decay1], 0.0f, 1.0f, "D1 %.2f");
	bool sustainChanged = ImGui::SliderFloat("Sustain level", &ampl.sustainLevel, 0.0f, 1.0f, "S %.2f");
	bool d2Changed = ImGui::SliderFloat("Decay 2 Rate", &ampl.rates[Decay2], 0.0f, 0.5f, "D2 %.4f");
	bool rChanged = ImGui::SliderFloat("Release Rate", &ampl.rates[Release], 0.0f, 1.0f, "R %.2f");

	bool wChanged = false;
	const char* wavenames[] = {"Sin", "Square", "Saw", "Triangle"};
	if (ImGui::Combo("Waveform", &OS.waveFormValue, wavenames, 4))
	{
		Waveform form = Waveform::Sin;
		switch(OS.waveFormValue)
		{
			case 0: form = Waveform::Sin; break;
			case 1: form = Waveform::Square; break;
			case 2: form = Waveform::Saw; break;
			case 3: form = Waveform::Triangle; break;
		}

		O.SetWaveform(form);
		wChanged = true;
	}

	if (firstUpdate || sustainChanged || aChanged || d1Changed || d2Changed || rChanged || wChanged)

	{
		change = true;
		UpdateGraph(graphPoints, ampl);
	}
	ImGui::PopID();

	return (firstUpdate || change);
}

std::string DangoFM::InstrumentWindow::SemitonesToIntervalDescription(int16 semitones)
{
	std::string text = "";
	if (semitones == -Interval::Octave)
	{
		text += "Octave below";
	}
	else
	{
		uint8 octaves = 0;
		while(semitones >= Interval::Octave)
		{
			octaves++;
			semitones -= Interval::Octave;
		}
		if (octaves > 0)
		{
			text += std::to_string(octaves);
			text += " octave";
			if (octaves > 1)
			{
				text += "s";
			}
			if (semitones > 0)
			{
				text += " and ";
				text += IntervalNames[semitones];
			}
		}
		else
		{
			text += IntervalNames[semitones];
		}
	}
	return text;
}


void DangoFM::InstrumentWindow::UpdatePreview(Synth& synth, DangoFM::Instrument& instrument)
{
	instrument.TuneToNote(12 * 4);
	Operator C = instrument.Carrier;
	Operator M = instrument.Modulator;
    real ModAmpl = Freq2Rad * (M.GetLevel() * (float)synth.modulationIndex) * M.frequenzyHz;
	real feedback = 0.0f;
	real sample = 0.0f;
	for (SampleCount i = 0; i < previewSize; i++)
	{
		sample = C.GetWaveFunction()(C.phase);
		real modulationAmount = M.GetWaveFunction()(M.phase) + feedback * M.GetFeedBack();
		feedback = modulationAmount;

		C.phase += C.phaseIncrement + modulationAmount * ModAmpl;
		M.phase += M.phaseIncrement;
		if (C.phase >= TAU)
		{
			C.phase -= TAU;
		}
		if (M.phase >= TAU)
		{
			M.phase -= TAU;
		}

		previewGraphPoints[i] = sample;
	}
}


void DangoFM::InstrumentWindow::UpdateGraph(real* graph, DangoFM::Envelope& envelope)
{
	int step = 10;//DANGO_SAMPLES_PER_SECOND;
	OperatorState state;
	state.state = Attack;
	state.slope = envelope.GetSlopeAt(state.state);

	for (SampleCount i = 0; i < graphSize; i++)
	{
		real sample = state.level;
		graph[i] = sample;
		for (int s = 0; s < step; s++)
		{
			if (state.Advance())
			{
				state.slope = envelope.GetSlopeAt(state.state);
			}
		}
	}
	state.state = Release;
	state.slope = envelope.GetSlopeAt(state.state);
	state.slope.increment *= -1.0f;
	state.level = 0.0f;
	for (SampleCount i = graphSize-1; i >= 0; i--)
	{
		if (graph[i] > state.level)
		{
			graph[i] = state.level;
		}
		else
		{
			break;
		}
		for (int s = 0; s < step; s++)
		{
			state.level += state.slope.increment;
		}
	}
}
