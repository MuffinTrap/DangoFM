#include "keyboard_window.h"

#include "../imgui.h"
#include "../../core/type_definitions.h"


DangoFM::KeyResult DangoFM::PianoKey::Key(std::string text)
{
	ImGui::Button(text.c_str());
	bool press = ImGui::IsItemActivated();
	bool release = ImGui::IsItemDeactivated();
	bool keyPress = ImGui::IsKeyPressed(hotkey);
	bool keyRelease = ImGui::IsKeyReleased(hotkey);
	if ((press||keyPress) && status == KeyStatus::Up)
	{
		status = KeyStatus::Down;
		return KeyResult::Pressed;
	}
	else if ((release||keyRelease) && status == KeyStatus::Down)
	{
		status = KeyStatus::Up;
		return KeyResult::Released;
	}
	else
	{
		return KeyResult::None;
	}
}

void DangoFM::KeyboardWindow::HandleKey(std::string text, DangoFM::PianoKey& key, Synth& synth)
{
	KeyResult r = key.Key(text);
	if (r == KeyResult::Pressed)
	{
		synth.NoteOn(0, baseNote + key.semitoneOffset, velocity);
	}
	else if ( r == KeyResult::Released)
	{
		// Note off
		synth.NoteOff(0, baseNote + key.semitoneOffset);
	}
}

DangoFM::KeyboardWindow::KeyboardWindow()
{
	pianoKeyBinds = new ImGuiKey[12] {
		ImGuiKey_Z, ImGuiKey_S,
		ImGuiKey_X, ImGuiKey_D,
		ImGuiKey_C,

		ImGuiKey_V, ImGuiKey_G,
		ImGuiKey_B, ImGuiKey_H,
		ImGuiKey_N, ImGuiKey_J,
		ImGuiKey_M
	};
	for (int i  = 0; i < 12; i++)
	{
		keys[i].semitoneOffset = i;
		keys[i].hotkey = pianoKeyBinds[i];
	}
	baseNote = octave * Interval::Octave;
}

void DangoFM::KeyboardWindow::Draw(DangoFM::Synth& synth)
{
	ImGui::Begin("Keyboard", NULL);

	ImColor black = ImColor::HSV(172.0f/255.0f, 57.0f/255.0f, 73.0f/255.0f);
	ImColor blue = ImColor::HSV(146.0f/255.0f, 249.0f/255.0f, 196.0f/255.0f);
	ImColor white = ImColor::HSV(246.0f/255.0f, 5.0f/255.0f, 216.0f/255.0f);

	if (ImGui::SliderInt("Octave", &octave, 1, 7, "%d"))
	{
		baseNote = octave * 12;
	}
	ImGui::SliderInt("Velocity", &velocity, 1, 127, "%d");

	// Black Keys
	ImGui::PushID(0);
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)blue);
	ImGui::Indent(30.0f);
	HandleKey("C#", keys[1], synth);
	ImGui::SameLine();
	HandleKey("D#", keys[3], synth);
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(10.0f, 0.0f));
	ImGui::SameLine();
	HandleKey("F#", keys[6], synth);
	ImGui::SameLine();
	HandleKey("G#", keys[8], synth);
	ImGui::SameLine();
	HandleKey("A#", keys[10], synth);
	ImGui::PopStyleColor();
	ImGui::Unindent();
	ImGui::PopID();

	ImGui::PushID(1);
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)white);
	ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)black);
	HandleKey("C ", keys[0], synth);
	ImGui::SameLine();
	HandleKey("D ", keys[2], synth);
	ImGui::SameLine();
	HandleKey("E ", keys[4], synth);
	ImGui::SameLine();
	HandleKey("F ", keys[5], synth);
	ImGui::SameLine();
	HandleKey("G ", keys[7], synth);
	ImGui::SameLine();
	HandleKey("A ", keys[9], synth);
	ImGui::SameLine();
	HandleKey("B ", keys[11], synth);
	ImGui::PopStyleColor(2);
	ImGui::PopID();

	ImGui::End();
}

