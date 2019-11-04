#pragma once

#ifdef FLAME_UNIVERSE_MODULE
#define FLAME_UNIVERSE_EXPORTS __declspec(dllexport)
#else
#define FLAME_UNIVERSE_EXPORTS __declspec(dllimport)
#endif

#include <flame/foundation/foundation.h>

namespace flame
{
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

	FLAME_UNIVERSE_EXPORTS void* universe_alloc(uint size);

	template<class T>
	T* new_u_object()
	{
		auto c = (T*)universe_alloc(sizeof(T));
		new (c) T;
		return c;
	}

	FLAME_UNIVERSE_EXPORTS void* add_listener_plain(void* hub, void(*pf)(void* c), const Mail<>& capture);
	FLAME_UNIVERSE_EXPORTS void remove_listener_plain(void* hub, void* c);

	template<class F>
	struct Listeners
	{
		void* hub;

		void* add(F* pf, const Mail<>& capture)
		{
			return add_listener_plain(hub, (void(*)(void* c))pf, capture);
		}

		void remove(void* c)
		{
			remove_listener_plain(hub, c);
		}
	};

	struct Universe
	{
		FLAME_UNIVERSE_EXPORTS void add_object(Object* o);
		FLAME_UNIVERSE_EXPORTS Object* find_object(uint name_hash, uint id);
		FLAME_UNIVERSE_EXPORTS uint find_id(Object* o);

		FLAME_UNIVERSE_EXPORTS void add_world(World* w);
		FLAME_UNIVERSE_EXPORTS void update();

		FLAME_UNIVERSE_EXPORTS static Universe* create();
		FLAME_UNIVERSE_EXPORTS static void destroy(Universe* u);
	};

	// objects in universe module can have serialization through an reflected type, the type name is 'Serializer_' + object's name + '$'
}
