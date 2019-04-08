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

#ifdef FLAME_WINDOWS
#ifdef FLAME_GRAPHICS_MODULE
#define FLAME_GRAPHICS_EXPORTS __declspec(dllexport)
#else
#define FLAME_GRAPHICS_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_GRAPHICS_EXPORTS
#endif

#include <flame/math.h>

namespace flame
{
	namespace graphics
	{
		enum Format
		{
			Format_Undefined,

			Format_R8_UNORM,
			Format_R16_UNORM,
			Format_R32_SFLOAT,
			Format_R32G32_SFLOAT,
			Format_R32G32B32_SFLOAT,
			Format_R8G8B8A8_UNORM,
			Format_R8G8B8A8_SRGB,
			Format_B8G8R8A8_UNORM,
			Format_B8G8R8A8_SRGB,
			Format_Swapchain_B8G8R8A8_UNORM,
			Format_Swapchain_B8G8R8A8_SRGB,
			Format_Swapchain_Begin = Format_Swapchain_B8G8R8A8_UNORM,
			Format_Swapchain_End = Format_Swapchain_B8G8R8A8_SRGB,
			Format_R16G16B16A16_UNORM,
			Format_R16G16B16A16_SFLOAT,
			Format_R32G32B32A32_SFLOAT,
			Format_RGBA_BC3,
			Format_RGBA_ETC2,
			Format_Color_Begin = Format_R8_UNORM,
			Format_Color_End = Format_RGBA_ETC2,

			Format_Depth16,
			Format_Depth32,
			Format_Depth24Stencil8,
			Format_DepthStencil_Begin = Format_Depth24Stencil8,
			Format_DepthStencil_End = Format_Depth24Stencil8,
			Format_Depth_Begin = Format_Depth16,
			Format_Depth_End = Format_Depth24Stencil8,
		};

		enum MemProp
		{
			MemPropDevice = 1 << 0,
			MemPropHost = 1 << 1,
			MemPropHostCoherent = 1 << 2
		};

		enum DescriptorSetBindings
		{
			MainDescriptorSetBinding,
			MaterialDescriptorSetBinding,
			BoneSetDescriptorBinding
		};

		enum SampleCount
		{
			SampleCount_1,
			SampleCount_2,
			SampleCount_4,
			SampleCount_8,
			SampleCount_16,
			SampleCount_32
		};

		enum ShaderType
		{
			ShaderNone,
			ShaderVert = 1 << 0,
			ShaderTesc = 1 << 1,
			ShaderTese = 1 << 2,
			ShaderGeom = 1 << 3,
			ShaderFrag = 1 << 4,
			ShaderComp = 1 << 5,
			ShaderAll = ShaderVert | ShaderTesc | ShaderTese | ShaderGeom | ShaderFrag
		};

		enum ShaderResourceType
		{
			ShaderResourceUniformBuffer,
			ShaderResourceStorageBuffer,
			ShaderResourceSampledImage,
			ShaderResourceStorageImage,
			ShaderResourcePushConstant
		};

		enum PipelineType
		{
			PipelineNone,
			PipelineGraphics,
			PipelineCompute
		};

		enum BufferUsage
		{
			BufferUsageTransferSrc = 1 << 0,
			BufferUsageTransferDst = 1 << 1,
			BufferUsageUniform = 1 << 2,
			BufferUsageStorage = 1 << 3,
			BufferUsageIndex = 1 << 4,
			BufferUsageVertex = 1 << 5,
			BufferUsageIndirect = 1 << 6
		};

		enum ImageUsage
		{
			ImageUsageTransferSrc = 1 << 0,
			ImageUsageTransferDst = 1 << 1,
			ImageUsageSampled = 1 << 2,
			ImageUsageStorage = 1 << 3,
			ImageUsageAttachment = 1 << 4
		};

		enum ImageLayout
		{
			ImageLayoutUndefined,
			ImageLayoutAttachment,
			ImageLayoutShaderReadOnly,
			ImageLayoutShaderStorage,
			ImageLayoutTransferSrc,
			ImageLayoutTransferDst,
			ImageLayoutPresent
		};

		enum ImageAspect
		{
			ImageAspectColor = 1 << 0,
			ImageAspectDepth = 1 << 1,
			ImageAspectStencil = 1 << 2
		};

		enum ImageviewType
		{
			Imageview1D,
			Imageview2D,
			Imageview3D,
			ImageviewCube,
			Imageview1DArray,
			ImageView2DArray,
			ImageViewCubeArray
		};

		enum Swizzle
		{
			SwizzleIdentity,
			SwizzleZero,
			SwizzleOne,
			SwizzleR,
			SwizzleG,
			SwizzleB,
			SwizzleA
		};

		enum IndiceType
		{
			IndiceTypeUint,
			IndiceTypeUshort
		};

		enum Filter
		{
			FilterNearest,
			FilterLinear
		};
	}
}
