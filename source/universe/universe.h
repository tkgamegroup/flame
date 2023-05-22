#pragma once

#include "../foundation/foundation.h"
#include "../graphics/graphics.h"
#ifdef USE_AUDIO_MODULE
#include "../audio/audio.h"
#endif
#ifdef USE_PHYSICS_MODULE
#include "../physics/physics.h"
#endif

#ifdef FLAME_UNIVERSE_MODULE

#define FLAME_UNIVERSE_API __declspec(dllexport)

#define FLAME_UNIVERSE_TYPE(name) FLAME_TYPE_PRIVATE(name)

#else

#define FLAME_UNIVERSE_API __declspec(dllimport)

#define FLAME_UNIVERSE_TYPE(name) FLAME_TYPE(name)

#endif

namespace flame
{
	struct DrawData;

	FLAME_UNIVERSE_TYPE(Entity);
	FLAME_UNIVERSE_TYPE(Component);
	FLAME_UNIVERSE_TYPE(System);
	FLAME_UNIVERSE_TYPE(World);

	FLAME_UNIVERSE_TYPE(cElement);
	FLAME_UNIVERSE_TYPE(cText);
	FLAME_UNIVERSE_TYPE(cImage);
	FLAME_UNIVERSE_TYPE(cReceiver);
	FLAME_UNIVERSE_TYPE(cLayout);
	FLAME_UNIVERSE_TYPE(cDataUpdater);
	FLAME_UNIVERSE_TYPE(cList);
	FLAME_UNIVERSE_TYPE(cNode);
	FLAME_UNIVERSE_TYPE(cMesh);
	FLAME_UNIVERSE_TYPE(cArmature);
	FLAME_UNIVERSE_TYPE(cTerrain);
	FLAME_UNIVERSE_TYPE(cProcedureTerrain);
	FLAME_UNIVERSE_TYPE(cSdf);
	FLAME_UNIVERSE_TYPE(cVolume);
	FLAME_UNIVERSE_TYPE(cProcedureVolume);
	FLAME_UNIVERSE_TYPE(cTileMap);
	FLAME_UNIVERSE_TYPE(cParticleSystem);
	FLAME_UNIVERSE_TYPE(cCurve);
	FLAME_UNIVERSE_TYPE(cPhysicsRigid);
	FLAME_UNIVERSE_TYPE(cPhysicsShape);
	FLAME_UNIVERSE_TYPE(cPhysicsCController);
	FLAME_UNIVERSE_TYPE(cNavAgent);
	FLAME_UNIVERSE_TYPE(cNavObstacle);
	FLAME_UNIVERSE_TYPE(cNavScene);
	FLAME_UNIVERSE_TYPE(cAudioSource);
	FLAME_UNIVERSE_TYPE(cAudioListener);
	FLAME_UNIVERSE_TYPE(cDirLight);
	FLAME_UNIVERSE_TYPE(cPtLight);
	FLAME_UNIVERSE_TYPE(cCamera);
	FLAME_UNIVERSE_TYPE(cRendererSettings);

	FLAME_UNIVERSE_TYPE(sInput);
	FLAME_UNIVERSE_TYPE(sScene);
	FLAME_UNIVERSE_TYPE(sRenderer);
	FLAME_UNIVERSE_TYPE(sAudio);

	enum StaticState
	{
		NotStatic,
		StaticButDirty,
		Static
	};

	enum TagFlags
	{
		TagNone = 0,
		TagGeneral = 1 << 0,
		TagNotSerialized,
		TagMarkNavMesh = 1 << 2,
		TagUser = 1 << 3
	};

	inline TagFlags operator| (TagFlags a, TagFlags b) { return (TagFlags)((int)a | (int)b); }

	enum ElementAlignment
	{
		ElementAlignNone,
		ElementAlignCenter,
		ElementAlignEnd0,
		ElementAlignEnd1,
		ElementAlignFill
	};

	enum ElementLayoutType
	{
		ElementLayoutFree,
		ElementLayoutVertical,
		ElementLayoutHorizontal,
		ElementLayoutGrid,
		ElementLayoutCircle
	};

	enum ElementStateFlags
	{
		ElementStateNone = 0,
		ElementStateHovering = 1 << 0,
		ElementStateFocusing = 1 << 1,
		ElementStateActive = 1 << 2,
		ElementStateSelected = 1 << 3
	};

	inline ElementStateFlags operator| (ElementStateFlags a, ElementStateFlags b) { return (ElementStateFlags)((int)a | (int)b); }

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

	struct OctNode;

	FLAME_UNIVERSE_API void* universe_info();
}
