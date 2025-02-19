#pragma once
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "../core/synth.h"
#include "../core/driver.h"

#include "windows/synth_window.h"
#include "windows/keyboard_window.h"
#include "windows/channels_window.h"
#include "windows/instrument_window.h"

namespace DangoFM
{

class Editor
{
public:
	Editor();
	int Init(); // Returns error codes: 0 is ok
	void Run();
	void Quit();

private:
	void InitAudio();
	void SetAudioPaused(bool paused);

	// Different windows and controls
	bool soundOn = false;
	float masterLevel = 1.0f;

	SDL_Window* window;
	ImGuiIO io;
	SDL_GLContext gl_context;
	SDL_AudioDeviceID audio_device;
	bool window_open;

	Synth synthesizer;
	Driver driver;

	uint8 selectedChannel = 0;

	SynthWindow synthW;
	KeyboardWindow keyW;
	ChannelsWindow channelsW;
	InstrumentWindow instrumentW;
};
}
