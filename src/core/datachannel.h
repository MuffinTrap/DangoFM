#pragma once

namespace DangoFM
{
	class DataChannel
	{
	public:
		int index;
		int stampsStart;
		int eventsStart;
		int stampsSize;
		int eventsSize;
		int stampsEnd;
		int eventsEnd;
		uint32 TickClock;
		uint32 NextEventTimeTicks;
		uint32 SamplesDone;

		int stampIndex;
		int eventIndex;

		void ResetClock()
		{
			TickClock = 0;
			NextEventTimeTicks = 0;
		}

		void SetIndex(int index)
		{
			this->index = index;
		}

		void SetByteIndices(int stampStart, int stampSize, int eventStart, int eventSize)
		{
			this->stampsStart = stampStart;
			this->eventsStart = eventStart;
			this->stampsSize = stampSize;
			this->eventsSize = eventSize;
			stampsEnd = stampsStart + stampSize;
			eventsEnd = eventStart + eventSize;
			stampIndex = 0;
			eventIndex = 0;
		}

		void ResetSamplesDone()
		{
			SamplesDone = 0;
		}
	};
}
