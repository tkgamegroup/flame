#pragma once

#ifdef FLAME_SOUND_MODULE
#define FLAME_SOUND_EXPORTS __declspec(dllexport)
#else
#define FLAME_SOUND_EXPORTS __declspec(dllimport)
#endif
