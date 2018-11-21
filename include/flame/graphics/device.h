// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Renderpass;
		struct Pipeline;
		struct Descriptorpool;
		struct Sampler;
		struct Commandpool;
		struct Queue;

		enum Feature
		{
			FeatureTextureCompressionBC,
			FeatureTextureCompressionASTC_LDR,
			FeatureTextureCompressionETC2,

			FeatureCount
		};

		struct Device
		{
			Renderpass *rp_one_rgba32;
			Pipeline *pl_trans;
			Descriptorpool *dp;
			Sampler *sp_bi_linear;
			Commandpool *gcp; // graphics command pool
			Commandpool *tcp; // transfer command pool
			Queue *gq; // graphics queue
			Queue *tq; // transfer queue

			FLAME_GRAPHICS_EXPORTS bool has_feature(Feature f);

			FLAME_GRAPHICS_EXPORTS void set_shared();

			FLAME_GRAPHICS_EXPORTS static Device *create(bool debug);
			FLAME_GRAPHICS_EXPORTS static Device *get_shared();
			FLAME_GRAPHICS_EXPORTS static void destroy(Device *d);
		};
	}
}

