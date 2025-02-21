#include "channels_window.h"

#include "../imgui.h"

void DangoFM::ChannelsWindow::Init()
{
}
void DangoFM::ChannelsWindow::Draw(DangoFM::Synth& synth, DangoFM::Driver& driver)
{
	ImGui::Begin("Channels");
	ImGui::Text("Channel");
	for (int c = 0; c < DANGO_CHANNEL_AMOUNT; c++)
	{
		SynthChannel* ch0 = synth.GetChannel(c);
		DangoFM::DataChannel& d0 = driver.GetDataChannel(c);
		ImGui::PushID(c);
		DrawChannel(ch0, d0);
		ImGui::PopID();
	}

	ImGui::End();
}

void DangoFM::ChannelsWindow::DrawChannel(DangoFM::SynthChannel* ch, DangoFM::DataChannel& dchannel)
{
	ImVec4 grey = ImVec4(0.5f,0,0.5f,1);
	for (int v = 0; v < ch->activeVoices; v++)
	{
		uint8 n = ch->voices[v].note;
		std::string number = std::to_string(n);
		ImVec4 yellow = ImVec4(1,1,0,1);
		ImGui::TextColored(yellow, number.c_str());
		ImGui::SameLine();
	}
	for (int v = ch->activeVoices; v < DANGO_VOICE_AMOUNT; v++)
	{
		// Inactive voices
		ImGui::TextColored(grey, "--");
		if (v<DANGO_VOICE_AMOUNT-1)
		{
		ImGui::SameLine();
		}
	}


	// Draw Channel
	ImGui::SliderFloat("Volume", &ch->volume, 0.0f, 1.0f, "Vol %.2f");
	ImGui::PlotLines("Audio", ch->GetWorkBuffer(), DANGO_SAMPLES_PER_CALLBACK, 0.0f, NULL, -ch->volume, ch->volume
	, ImVec2(0.0f, 30.0f));
	ImGui::ProgressBar(ch->peakValue, ImVec2(0.0f, 0.0f));
	ImGui::SameLine();
	ImGui::Text("%.4f", ch->peakValue);
	ImGui::Text("Tick Clock: %d < %d Stamps: %zu/%d Events: %zu/%d", dchannel.TickClock, dchannel.NextEventTimeTicks, dchannel.stampIndex, dchannel.stampsSize, dchannel.eventIndex, dchannel.eventsSize);
}



