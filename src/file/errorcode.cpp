#include "errorcode.h"

#include <stdio.h>
#include <errno.h>

void DangoFM::ErrorHandler::print_dango_error(DangoFM::ErrorCode code)
{
  switch(code) {
  case NO_ERROR:
    break;
  case TOO_MANY_CHANNELS:
    printf("Too many channels in MIDI file\n");
    break;
  case FORMAT_NOT_SUPPORTED:
    printf("Unsupported MIDI format\n");
    break;
  case FILE_NOT_FOUND:
    printf("File not found\n");
    break;
  case COULD_NOT_ALLOCATE:
    printf("Could not allocate memory\n");
    break;
  case FILE_CANNOT_CREATE:
    printf("Cannot create file\n");
    break;
  case FILE_NO_INSTRUMENTS:
    printf("No instruments\n");
    break;
  case ZERO_READ:
    printf("Read nothing\n");
    break;
  };
}

void DangoFM::ErrorHandler::clear_system_error()
{
#ifdef DANGO_WINDOWS
  _set_errno(0);
#endif
#ifdef DANGO_LINUX
  errno = 0;
#endif
}

bool DangoFM::ErrorHandler::has_system_error(const char* tag)
{
  if (errno != 0) {
    perror(tag);
    printf("Error number is %d.\n",  errno);
#ifdef DANGO_WINDOWS
    printf("Windows error is %lu\n", _doserrno);
#endif
    clear_system_error();
    return false;
  }
  return false;
}
