#pragma once

#include "../foundation/foundation.h"
#include "../foundation/typeinfo.h"
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
	FLAME_UNIVERSE_TYPE(Entity);
	FLAME_UNIVERSE_TYPE(Component);
	FLAME_UNIVERSE_TYPE(System);
	FLAME_UNIVERSE_TYPE(World);
	FLAME_UNIVERSE_TYPE(Keyframe);
	FLAME_UNIVERSE_TYPE(Track);
	FLAME_UNIVERSE_TYPE(Timeline);
	FLAME_UNIVERSE_TYPE(TimelineInstance);
	FLAME_UNIVERSE_TYPE(FloatingText);

	FLAME_UNIVERSE_TYPE(cBpInstance);
	FLAME_UNIVERSE_TYPE(cElement);
	FLAME_UNIVERSE_TYPE(cText);
	FLAME_UNIVERSE_TYPE(cInputField);
	FLAME_UNIVERSE_TYPE(cImage);
	FLAME_UNIVERSE_TYPE(cStretchedImage);
	FLAME_UNIVERSE_TYPE(cReceiver);
	FLAME_UNIVERSE_TYPE(cLayout);
	FLAME_UNIVERSE_TYPE(cScrollView);
	FLAME_UNIVERSE_TYPE(cList);
	FLAME_UNIVERSE_TYPE(cNode);
	FLAME_UNIVERSE_TYPE(cMesh);
	FLAME_UNIVERSE_TYPE(cSkinnedMesh);
	FLAME_UNIVERSE_TYPE(cAnimator);
	FLAME_UNIVERSE_TYPE(cProcedureMesh);
	FLAME_UNIVERSE_TYPE(cTerrain);
	FLAME_UNIVERSE_TYPE(cProcedureTerrain);
	FLAME_UNIVERSE_TYPE(cSdf);
	FLAME_UNIVERSE_TYPE(cVolume);
	FLAME_UNIVERSE_TYPE(cProcedureVolume);
	FLAME_UNIVERSE_TYPE(cTileMap);
	FLAME_UNIVERSE_TYPE(cParticleSystem);
	FLAME_UNIVERSE_TYPE(cCurve);
	FLAME_UNIVERSE_TYPE(cBody2d);
	FLAME_UNIVERSE_TYPE(cBody3d);
	FLAME_UNIVERSE_TYPE(cNavAgent);
	FLAME_UNIVERSE_TYPE(cNavObstacle);
	FLAME_UNIVERSE_TYPE(cNavMesh);
	FLAME_UNIVERSE_TYPE(cCollider);
	FLAME_UNIVERSE_TYPE(cAudioSource);
	FLAME_UNIVERSE_TYPE(cAudioListener);
	FLAME_UNIVERSE_TYPE(cResourcesHolder);
	FLAME_UNIVERSE_TYPE(cDirectionalLight);
	FLAME_UNIVERSE_TYPE(cPointLight);
	FLAME_UNIVERSE_TYPE(cCamera);
	FLAME_UNIVERSE_TYPE(cWorldSettings);

	FLAME_UNIVERSE_TYPE(sInput);
	FLAME_UNIVERSE_TYPE(sScene);
	FLAME_UNIVERSE_TYPE(RenderTask);
	FLAME_UNIVERSE_TYPE(sRenderer);
	FLAME_UNIVERSE_TYPE(sHud);
	FLAME_UNIVERSE_TYPE(sAudio);
	FLAME_UNIVERSE_TYPE(sGraveyard);
	FLAME_UNIVERSE_TYPE(sTween);

	enum TagFlags
	{
		TagNone = 0,
		TagGeneral = 1 << 0,
		TagNotSerialized = 1 << 1,
		TagNotPickable = 1 << 2,
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

	enum ModifierType
	{
		ModifierLinear,
		ModifierDataBinding,
		ModifierExpression
	};

	struct Modifier
	{
		ModifierType type;
		std::string address;
		std::string data;
	};

	inline bool operator==(const Modifier& a, const Modifier& b)
	{
		return a.type == b.type && a.address == b.address && a.data == b.data;
	}

	struct DrawData;

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
}
