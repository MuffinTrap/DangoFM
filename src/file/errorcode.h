#pragma once

#include "midi_defines.h"
#include "../core/type_definitions.h"

namespace DangoFM
{
	enum ErrorCode : code
	{
		NO_ERROR,
		TOO_MANY_CHANNELS,
		FORMAT_NOT_SUPPORTED,
		FILE_NOT_FOUND,
		COULD_NOT_ALLOCATE,
		FILE_CANNOT_CREATE,
		FILE_NO_INSTRUMENTS,
		ZERO_READ,
	};

	class ErrorHandler
	{
	public:
		static void print_dango_error(ErrorCode code);

		static void clear_system_error();
		static bool has_system_error(const char* tag);

	};
}
