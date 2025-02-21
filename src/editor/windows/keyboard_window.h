#pragma once
#include "../../core/synth.h"
#include "../imgui.h"

namespace DangoFM
{
	enum KeyResult
	{
		None,
		Pressed,
		Released
	};
	enum KeyStatus
	{
		Up,
		Down
	};
	struct PianoKey
	{
	public:
		KeyResult Key(std::string text);
		KeyStatus GetStatus();
		uint8 semitoneOffset = 0;
		ImGuiKey hotkey;

	private:
		KeyStatus status = KeyStatus::Up;
	};

	class KeyboardWindow
	{
	public:
		KeyboardWindow();
		void Draw(Synth& synth);

		int octave = 2;
		uint8 baseNote = 24;
		int velocity = 68;
		PianoKey keys[12];
		void HandleKey(std::string text, PianoKey& key, Synth& synth);
		ImGuiKey* pianoKeyBinds = nullptr;
	};
}
