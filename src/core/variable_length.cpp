#include "variable_length.h"
#include "../file/midi_defines.h"

uint8 DangoFM::get_7bits_value(uint8 byte) {
  return (byte & BYTE_VALUE);
}

uint32 DangoFM::read_variable_length(uint8 *events, byteindex *byteIndex)
{
  // if bytes matches with 128, it means most significant
  // byte is 1 and this is a multi byte value
  // Only read the 7 bytes
  uint32 value = 0;
  int byteLocal = *byteIndex;
  uint8 byte = 0;

  /* Midi standard code */
  if ((value = events[byteLocal++]) & 0x80) {// If first byte masked with multi byte is > 0
    value &= 0x7F; //  remove continuation bit == mask with 127
    do {
      value = (value << 7) + ((byte = events[byteLocal++]) & 0x7F); // Shift left and add next byte masked with 127
    } while (byte & 0x80);// as long as last read byte has the continuation bit
  }

  *byteIndex = byteLocal;
  return value;
}

uint32 DangoFM::read_variable_length_file(FILE *file)
{
  uint32 value = 0;
  uint8 byte = 0;

  /* Midi standard code */
  if ((value = getc(file)) & 0x80) {// If first byte masked with multi byte is > 0
    value &= 0x7F; //  remove continuation bit == mask with 127
    do {
      value = (value << 7) + ((byte = getc(file)) & 0x7F); // Shift left and add next byte masked with 127
    } while (byte & 0x80);// as long as last read byte has the continuation bit
  }
  return value;
}

void DangoFM::write_variable_length_file_text(uint32 value, FILE *file) {
  uint32 buffer;
  buffer = value & 0x7F;  // First 7 bits
  while ((value >>= 7) > 0) { // While value has more bits
    buffer <<= 8; // Make room for more bits, using buffer like stack
    buffer |= 0x80; // Set continuation bit
    buffer += (value & 0x7F); // Add 7 bits
  }

  while (true) {
    uint8 hex = buffer & 0xFF;
    fprintf(file, "0x%x, ", hex); // Put one byte to file
    if (buffer & 0x80) { // If continuation bit set
      buffer >>= 8; // Next 8 bits
    }
    else {
      break;
    }
  }
}

void DangoFM::write_variable_length_file(uint32 value, FILE *file) {
  uint32 buffer;
  buffer = value & 0x7F;  // First 7 bits
  while ((value >>= 7) > 0) { // While value has more bits
    buffer <<= 8; // Make room for more bits, using buffer like stack
    buffer |= 0x80; // Set continuation bit
    buffer += (value & 0x7F); // Add 7 bits
  }

  while (true) {
    putc(buffer, file); // Put one byte to file
    if (buffer & 0x80) { // If continuation bit set
      buffer >>= 8; // Next 8 bits
    }
    else {
      break;
    }
  }
}

byteindex DangoFM::write_variable_length(uint32 value, uint8 *data, byteindex byteIndex) {
  // printf("Write variable length value: %d ->", value);
  int stackPointer = 4;
  uint8 stack[4] = {0, 0, 0, 0};
  uint8 byte = 0;
  do {
    // Take 7 bits of value, store and shift left
    byte = (value & 0xFF);
    uint8 byteValue = get_7bits_value(byte);
    stackPointer--;
    stack[stackPointer] = byteValue;
    value = value >> 7;
  } while (value > 0);

  //printf("Value takes %d bytes\n",  4 - stackPointer);

  uint8 writeValue = 0;
  for(int s = stackPointer; s < 4; s++) {
    writeValue = stack[s];
    if (s < 3) {
      writeValue |= MULTI_BYTE; // Continuation bit
    }
    data[byteIndex++] = writeValue;
    // printf(" %X", writeValue);
  }
  // printf("Conversion done\n\n");
  // printf("\n");
  return byteIndex;
}
