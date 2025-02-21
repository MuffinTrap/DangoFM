#pragma once

namespace DangoFM
{
	class DataChannel
	{
	public:
		int index = 0;
		uint8* stampsStart = nullptr;
		uint8* eventsStart = nullptr;
		int stampsSize = 0;
		int eventsSize = 0 ;
		byteindex stampIndex = 0;
		byteindex eventIndex = 0;
		uint32 TickClock = 0;
		uint32 NextEventTimeTicks = 0;
		uint32 SamplesDone = 0;

		DataChannel()
		{

		}


		void ResetClock()
		{
			TickClock = 0;
			NextEventTimeTicks = 0;
		}

		void SetIndex(int index)
		{
			this->index = index;
		}

		void SetByteIndices(uint8* stampStart, int stampSize, uint8* eventStart, int eventSize)
		{
			this->stampsStart = stampStart;
			this->eventsStart = eventStart;
			this->stampsSize = stampSize;
			this->eventsSize = eventSize;
			stampIndex = 0;
			eventIndex = 0;
		}

		void ResetSamplesDone()
		{
			SamplesDone = 0;
		}
	};
}
