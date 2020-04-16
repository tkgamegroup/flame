#pragma once

#ifdef FLAME_UNIVERSE_MODULE
#define FLAME_UNIVERSE_EXPORTS __declspec(dllexport)
#else
#define FLAME_UNIVERSE_EXPORTS __declspec(dllimport)
#endif

#include <flame/foundation/foundation.h>

namespace flame
{
	enum ClipFlag
	{
		ClipSelf = 1 << 0,
		ClipChildren = 1 << 1
	};

	enum FocusType
	{
		FocusByLeftButton,
		FocusByRightButton,
		FocusByLeftOrRightButton
	};

	enum FocusingState
	{
		FocusingNormal,
		FocusingAndActive,
		FocusingAndDragging
	};

	enum EventReceiverState
	{
		EventReceiverNormal = 0,
		EventReceiverHovering = 1 << 0,
		EventReceiverActive = 1 << 1
	};

	enum AlignFlag
	{
		AlignMin = 1 << 0,
		AlignMax = 1 << 1,
		AlignMiddle = 1 << 2,
		AlignMinMax = 1 << 3,
		AlignAbsolute = 1 << 4, // no padding
		AlignGreedy = 1 << 5
	};

	enum LayoutType
	{
		LayoutFree,
		LayoutVertical,
		LayoutHorizontal,
		LayoutGrid
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

	enum ExtraDrawFlag
	{
		ExtraDrawFilledCornerSE = 1 << 0,
		ExtraDrawHorizontalLine = 1 << 1,
		ExtraDrawVerticalLine = 1 << 2
	};
}
