#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cAudioSource : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		/// Reflect
		std::filesystem::path buffer_name;
		/// Reflect
		virtual void set_buffer_name(const std::filesystem::path& buffer_name) = 0;

		int source_res_id = -1;

		struct Create
		{
			virtual cAudioSourcePtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
