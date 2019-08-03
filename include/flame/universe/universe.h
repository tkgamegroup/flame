#pragma once

#ifdef FLAME_UNIVERSE_MODULE
#define FLAME_UNIVERSE_EXPORTS __declspec(dllexport)
#else
#define FLAME_UNIVERSE_EXPORTS __declspec(dllimport)
#endif

