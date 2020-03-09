#pragma once

#ifdef FLAME_UNIVERSE_MODULE
#define FLAME_UNIVERSE_EXPORTS __declspec(dllexport)
#else
#define FLAME_UNIVERSE_EXPORTS __declspec(dllimport)
#endif

#include <flame/foundation/foundation.h>

namespace flame
{
	enum FocusingState
	{
		FocusingNormal,
		FocusingAndActive,
		FocusingAndDragging
	};

	enum EventReceiverState
	{
		EventReceiverNormal,
		EventReceiverHovering,
		EventReceiverActive
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

	template <class T>
	T* new_u_object()
	{
		auto c = (T*)f_malloc(sizeof(T));
		new (c) T;
		return c;
	}

	// objects in universe module can have serialization through an reflected type, the type name is 'Serializer_' + object's name
}
