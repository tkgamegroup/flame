#pragma once

#ifdef FLAME_UNIVERSE_MODULE

#define FLAME_UNIVERSE_EXPORTS __declspec(dllexport)

#define FLAME_UNIVERSE_TYPE(name) struct name; struct name##Private; \
	using name##T = name##Private; \
	using name##Ptr = name##Private*;

#else

#define FLAME_UNIVERSE_EXPORTS __declspec(dllimport)

#define FLAME_UNIVERSE_TYPE(name) struct name; struct name##Private; \
	using name##T = name##Private; \
	using name##Ptr = name##Private*;

#endif

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
	FLAME_UNIVERSE_TYPE(cWindow);
	FLAME_UNIVERSE_TYPE(cNode);
	FLAME_UNIVERSE_TYPE(cMesh);
	FLAME_UNIVERSE_TYPE(cArmature);
	FLAME_UNIVERSE_TYPE(cTerrain);
	FLAME_UNIVERSE_TYPE(cRigid);
	FLAME_UNIVERSE_TYPE(cShape);
	FLAME_UNIVERSE_TYPE(cCharacterController);
	FLAME_UNIVERSE_TYPE(cLight);
	FLAME_UNIVERSE_TYPE(cCamera);
	FLAME_UNIVERSE_TYPE(cImgui);

	FLAME_UNIVERSE_TYPE(sScene);
	FLAME_UNIVERSE_TYPE(sElementRenderer);
	FLAME_UNIVERSE_TYPE(sNodeRenderer);
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
