#include "synth_window.h"
#include "../imgui.h"



void DangoFM::SynthWindow::Init()
{
}




void DangoFM::SynthWindow::Draw(DangoFM::Synth& synth, Driver& driver)
{
	ImGui::Begin("Synthesizer", &isOpen);
	ImGui::Text("Beep boop!");
	ImGui::PlotLines("Audio", driver.GetWorkBuffer(), DANGO_SAMPLES_PER_CALLBACK, 0.0f, NULL, -1.0f, 1.0f, ImVec2(0.0f, 80.0f));
	ImGui::SliderInt("Modulation Index", &synth.modulationIndex, 1, 100);
	if (ImGui::Button("Stop all voices"))
	{
		synth.StopAllVoices();

	}

	ImGui::End();
}


