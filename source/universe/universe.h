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
	typedef FlameUniverseTypeSelector<name, name##Private>::result name##T; \
	typedef FlameUniverseTypeSelector<name*, name##Private*>::result name##Ptr;

#include "../foundation/foundation.h"
#include "../graphics/graphics.h"
#ifdef USE_SOUND_MODULE
#include "../sound/sound.h"
#endif
#ifdef USE_PHYSICS_MODULE
#include "../physics/physics.h"
#endif

namespace flame
{
	FLAME_UNIVERSE_TYPE(Entity);
	FLAME_UNIVERSE_TYPE(Component);
	FLAME_UNIVERSE_TYPE(System);
	FLAME_UNIVERSE_TYPE(World);

	FLAME_UNIVERSE_TYPE(cElement);
	FLAME_UNIVERSE_TYPE(cText);
	FLAME_UNIVERSE_TYPE(cImage);
	FLAME_UNIVERSE_TYPE(cReceiver);
	FLAME_UNIVERSE_TYPE(cToggle);
	FLAME_UNIVERSE_TYPE(cEdit);
	FLAME_UNIVERSE_TYPE(cList);
	FLAME_UNIVERSE_TYPE(cListItem);
	FLAME_UNIVERSE_TYPE(cTree);
	FLAME_UNIVERSE_TYPE(cTreeNode);
	FLAME_UNIVERSE_TYPE(cTreeLeaf);
	FLAME_UNIVERSE_TYPE(cMenu);
	FLAME_UNIVERSE_TYPE(cMenuItem);
	FLAME_UNIVERSE_TYPE(cMenuBar);
	FLAME_UNIVERSE_TYPE(cDropDown);
	FLAME_UNIVERSE_TYPE(cDragEdit);
	FLAME_UNIVERSE_TYPE(cScroller);
	FLAME_UNIVERSE_TYPE(cSplitter);
	FLAME_UNIVERSE_TYPE(cGrid);
	FLAME_UNIVERSE_TYPE(cWindow);
	FLAME_UNIVERSE_TYPE(cInputDialog);
	FLAME_UNIVERSE_TYPE(cFileSelector);
	FLAME_UNIVERSE_TYPE(cNode);
	FLAME_UNIVERSE_TYPE(cOctree);
	FLAME_UNIVERSE_TYPE(cMesh);
	FLAME_UNIVERSE_TYPE(cArmature);
	FLAME_UNIVERSE_TYPE(cTerrain);
	FLAME_UNIVERSE_TYPE(cRigid);
	FLAME_UNIVERSE_TYPE(cShape);
	FLAME_UNIVERSE_TYPE(cCharacterController);
	FLAME_UNIVERSE_TYPE(cLight);
	FLAME_UNIVERSE_TYPE(cCamera);
	FLAME_UNIVERSE_TYPE(cSky);
	FLAME_UNIVERSE_TYPE(cImgui);

	FLAME_UNIVERSE_TYPE(sRenderer);
	FLAME_UNIVERSE_TYPE(sElementRenderer);
	FLAME_UNIVERSE_TYPE(sNodeRenderer);
	FLAME_UNIVERSE_TYPE(sScene);
	FLAME_UNIVERSE_TYPE(sDispatcher);
	FLAME_UNIVERSE_TYPE(sPhysics);
	FLAME_UNIVERSE_TYPE(sImgui);

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

	enum LightType
	{
		LightDirectional,
		LightPoint,
		LightSpot
	};

	enum RenderType
	{
		RenderWireframe,
		RenderShaded,
		RenderNormalData
	};

	enum ShadingFlags
	{
		ShadingMaterial = 1 << 0,
		ShadingShadow = 1 << 1,
		ShadingWireframe = 1 << 2,
		ShadingOutline = 1 << 3
	};

	inline ShadingFlags operator| (ShadingFlags a, ShadingFlags b) { return (ShadingFlags)((int)a | (int)b); }

	inline constexpr uint MaxLod = 6;
}
