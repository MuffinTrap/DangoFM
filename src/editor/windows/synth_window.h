


#pragma once

#include "../../core/synth.h"
#include "../../core/driver.h"

namespace DangoFM
{
	class SynthWindow
	{
	public:
		void Init();
		void Draw(Synth& synth, Driver& driver);
		bool isOpen = true;

	private:
	};

}
