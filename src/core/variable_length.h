#pragma once

#include <stdio.h>
#include "type_definitions.h"

namespace DangoFM
{

	uint8 get_7bits_value(uint8 byte);
	uint32 read_variable_length(uint8 *events, byteindex *byteIndex);

	uint32 read_variable_length_file(FILE *file);
	byteindex write_variable_length(uint32 value, uint8 *data, byteindex byteIndex);
	void write_variable_length_file(uint32 value, FILE *file);
	void write_variable_length_file_text(uint32 value, FILE *file);

};
