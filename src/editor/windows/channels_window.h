#pragma once

#include "../../core/synth.h"
#include "../../core/driver.h"

namespace DangoFM
{
	class ChannelsWindow
	{
	public:
		void Init();
		void Draw(Synth& synth, Driver& driver);
		bool isOpen = true;
	private :
		void DrawChannel(SynthChannel* ch);
	};
}

