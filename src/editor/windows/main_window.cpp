#include "main_window.h"
#include "../imgui.h"
#include "../../file/midi_to_song.h"
void DangoFM::MainWindow::Init(SDL_AudioDeviceID audio_device)
{
	this->audio_device = audio_device;
}

void DangoFM::MainWindow::Draw(DangoFM::Driver& driver)
{
	ImGui::Begin("Dango FM", &alwaysOpen, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Load MIDI"))
			{
				// NOP
				MidiToSongConverter ms;
				Song* s = ms.Convert("TitleTheme.mid");
				driver.LoadSong(s);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (ImGui::Checkbox("Sound On", &soundOn))
	{
		SetAudioPaused(!soundOn);
	}
	if (ImGui::SliderFloat("Master volume", &masterLevel, 0.0f, 1.0f))
	{
		driver.SetMasterVolume(masterLevel);
	}

	if (driver.HasLoadedSong())
	{
		if (ImGui::Button("Play"))
		{
			driver.ResetSongPlayback();
			driver.activeMode = DriverMode::PlayLoadedSong;
		}
		if (ImGui::Button("Stop"))
		{
			driver.activeMode = DriverMode::PlaySynth;
			driver.GetSynth()->StopAllVoices();
		}
		Song* s = driver.GetLoadedSong();
		ImGui::Text("Song ch: %d", s->channelAmount);
	}
	switch(driver.activeMode)
	{
		case PlayLoadedSong:
		ImGui::Text("Playing song");
		break;
		case PlaySynth:
		ImGui::Text("Playing synth");
		break;
		case PlayHeaderSong:
		ImGui::Text("Playing header song");
		break;
	}
	ImGui::Text("Ticks %d / Target %d", driver.GetTicksClock(), driver.GetTicksTarget());
	ImGui::End();
}


void DangoFM::MainWindow::SetAudioPaused(bool paused)
{
  SDL_PauseAudioDevice(audio_device, paused ? 1 : 0);
}
