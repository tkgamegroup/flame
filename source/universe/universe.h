#pragma once

#ifdef FLAME_UNIVERSE_MODULE
#define FLAME_UNIVERSE_EXPORTS __declspec(dllexport)
template<class T, class U>
struct FlameUniverseTypeSelector
{
	typedef U result;
};
#else
#define FLAME_UNIVERSE_EXPORTS __declspec(dllimport)
template<class T, class U>
struct FlameUniverseTypeSelector
{
	typedef T result;
};
#endif

#define FLAME_UNIVERSE_TYPE(name) struct name; struct name##Private; \
	typedef FlameUniverseTypeSelector<name*, name##Private*>::result name##Ptr;

#include "../foundation/foundation.h"
#include "../network/network.h"
#include "../database/database.h"
#include "../graphics/graphics.h"
#include "../sound/sound.h"
#include "../physics/physics.h"
#include "../script/script.h"

namespace flame
{
	FLAME_UNIVERSE_TYPE(Entity);
	FLAME_UNIVERSE_TYPE(Component);
	FLAME_UNIVERSE_TYPE(Driver);
	FLAME_UNIVERSE_TYPE(System);
	FLAME_UNIVERSE_TYPE(World);

	FLAME_UNIVERSE_TYPE(cElement);
	FLAME_UNIVERSE_TYPE(cText);
	FLAME_UNIVERSE_TYPE(cImage);
	FLAME_UNIVERSE_TYPE(cReceiver);
	FLAME_UNIVERSE_TYPE(cNode);
	FLAME_UNIVERSE_TYPE(cMesh);
	FLAME_UNIVERSE_TYPE(cTerrain);
	FLAME_UNIVERSE_TYPE(cRigid);
	FLAME_UNIVERSE_TYPE(cShape);
	FLAME_UNIVERSE_TYPE(cController);
	FLAME_UNIVERSE_TYPE(cLight);
	FLAME_UNIVERSE_TYPE(cCamera);
	FLAME_UNIVERSE_TYPE(cSky);
	FLAME_UNIVERSE_TYPE(cScript);

	FLAME_UNIVERSE_TYPE(dEdit);
	FLAME_UNIVERSE_TYPE(dCheckbox);
	FLAME_UNIVERSE_TYPE(dSlider);
	FLAME_UNIVERSE_TYPE(dMenu);
	FLAME_UNIVERSE_TYPE(dCombobox);
	FLAME_UNIVERSE_TYPE(dList);
	FLAME_UNIVERSE_TYPE(dTree);
	FLAME_UNIVERSE_TYPE(dScroller);
	FLAME_UNIVERSE_TYPE(dSplitter);
	FLAME_UNIVERSE_TYPE(dDragEdit);
	FLAME_UNIVERSE_TYPE(dWindow);
	FLAME_UNIVERSE_TYPE(dInputDialog);
	FLAME_UNIVERSE_TYPE(dGrid);

	FLAME_UNIVERSE_TYPE(sRenderer);
	FLAME_UNIVERSE_TYPE(sScene);
	FLAME_UNIVERSE_TYPE(sDispatcher);
	FLAME_UNIVERSE_TYPE(sPhysics);

	enum StateFlags
	{
		StateNone = 0,
		StateHovering = 1 << 0,
		StateFocusing = 1 << 1,
		StateActive = 1 << 2,
		StateSelected = 1 << 3
	};

	inline StateFlags operator| (StateFlags a, StateFlags b) { return (StateFlags)((int)a | (int)b); }

	enum Align
	{
		AlignNone,
		AlignMin,
		AlignMax,
		AlignMiddle,
		AlignMinMax
	};

	enum LayoutType
	{
		LayoutFree,
		LayoutHorizontal,
		LayoutVertical,
		LayoutTile
	};

	enum ScrollType
	{
		ScrollHorizontal,
		ScrollVertical,
		ScrollBoth
	};

	enum SplitterType
	{
		SplitterHorizontal,
		SplitterVertical
	};

	enum MenuType
	{
		MenuTop,
		MenuSub
	};

	enum ExtraDrawFlags
	{
		ExtraDrawFilledCornerSE = 1 << 0,
		ExtraDrawHorizontalLine = 1 << 1,
		ExtraDrawVerticalLine = 1 << 2
	};

	inline ExtraDrawFlags operator| (ExtraDrawFlags a, ExtraDrawFlags b) { return (ExtraDrawFlags)((int)a | (int)b); }

	enum LightType
	{
		LightDirectional,
		LightPoint,
		LightSpot
	};
}
