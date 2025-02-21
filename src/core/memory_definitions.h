#pragma once

#ifndef DANGO_MALLOC_FUNCTION
#include <SDL_stdinc.h>
#define DANGO_MALLOC_FUNCTION SDL_malloc
#endif

#ifndef DANGO_FREE_FUNCTION
#include <SDL_stdinc.h>
#define DANGO_FREE_FUNCTION SDL_free
#endif

#ifndef DANGO_MEMSET_FUNCTION
#include <SDL_stdinc.h>
#define DANGO_MEMSET_FUNCTION SDL_memset
#endif
