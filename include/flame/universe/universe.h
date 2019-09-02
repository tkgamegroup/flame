#pragma once

#ifdef FLAME_UNIVERSE_MODULE
#define FLAME_UNIVERSE_EXPORTS __declspec(dllexport)
#else
#define FLAME_UNIVERSE_EXPORTS __declspec(dllimport)
#endif

namespace flame
{
	enum Alignx$
	{
		AlignxFree,
		AlignxLeft,
		AlignxMiddle,
		AlignxRight
	};

	enum Aligny$
	{
		AlignyFree,
		AlignyTop,
		AlignyMiddle,
		AlignyBottom
	};

	enum SizePolicy$
	{
		SizeFixed,
		SizeFitLayout,
		SizeGreedy
	};

	enum LayoutType$
	{
		LayoutFree,
		LayoutVertical,
		LayoutHorizontal
	};

	enum ScrollbarType
	{
		ScrollbarVertical,
		ScrollbarHorizontal
	};

	enum SplitterType
	{
		SplitterHorizontal,
		SplitterVertical
	};
}
