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

	enum Alignx
	{
		AlignxFree,
		AlignxLeft,
		AlignxMiddle,
		AlignxRight
	};

	enum Aligny
	{
		AlignyFree,
		AlignyTop,
		AlignyMiddle,
		AlignyBottom
	};

	enum SizePolicy
	{
		SizeFixed,
		SizeFitParent,
		SizeGreedy
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

	struct World;

	// objects in universe module can have serialization through an reflected type, the type name is 'Serializer_' + object's name
}
