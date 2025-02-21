#pragma once
#include "../../core/driver.h"
#include <SDL.h>

namespace DangoFM
{
	class MainWindow
	{
	public:
		void Init(SDL_AudioDeviceID audio_device);
		void Draw(Driver& driver);
		// Different windows and controls
		void SetAudioPaused(bool paused);
	private:

		bool alwaysOpen = true;
		bool soundOn = true;
		float masterLevel = 1.0f;
		SDL_AudioDeviceID audio_device;
	};
}
