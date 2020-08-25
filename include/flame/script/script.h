#pragma once

#ifdef FLAME_SCRIPT_MODULE
#define FLAME_SCRIPT_EXPORTS __declspec(dllexport)
#else
#define FLAME_SCRIPT_EXPORTS __declspec(dllimport)
#endif

namespace flame
{

}
