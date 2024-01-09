#include "../../../foundation/blueprint.h"
#include "../../systems/renderer_private.h"

namespace flame
{
	void add_primitive_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Make Line Strip", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Point 0",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Point 1",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Strip",
					.allowed_types = { TypeInfo::get<voidptr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto array_type = inputs[2].type;
				if (array_type && is_vector(array_type->tag))
				{
					if (array_type->get_wrapped() == TypeInfo::get<vec3>())
					{
						auto& strip = *(std::vector<vec3>*)inputs[2].data;
						auto pt0 = *(vec3*)inputs[0].data;
						auto pt1 = *(vec3*)inputs[1].data;

						if (strip.size() >= 2)
						{
							if (strip.back() == pt0)
								strip.push_back(pt1);
							else if (strip.front() == pt1)
								strip.insert(strip.begin(), pt0);
						}
						else
						{
							strip.push_back(pt0);
							strip.push_back(pt1);
						}
					}
				}
			}
		);

		library->add_template("Draw Line", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Pos0",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Pos1",
					.allowed_types = { TypeInfo::get<vec3>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				},
				{
					.name = "Depth Test",
					.allowed_types = { TypeInfo::get<bool>() },
					.default_value = "false"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				vec3 pts[2];
				pts[0] = *(vec3*)inputs[0].data;
				pts[1] = *(vec3*)inputs[1].data;
				sRenderer::instance()->draw_primitives(PrimitiveLineList, pts, 2, *(cvec4*)inputs[2].data, *(bool*)inputs[3].data);
			}
		);

		library->add_template("Draw Lines", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Points",
					.allowed_types = { TypeInfo::get<voidptr>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				},
				{
					.name = "Depth Test",
					.allowed_types = { TypeInfo::get<bool>() },
					.default_value = "false"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto array_type = inputs[0].type;
				if (array_type && is_vector(array_type->tag))
				{
					if (array_type->get_wrapped() == TypeInfo::get<vec3>())
					{
						auto& points = *(std::vector<vec3>*)inputs[0].data;
						sRenderer::instance()->draw_primitives(PrimitiveLineList, points.data(), points.size(), *(cvec4*)inputs[2].data, *(bool*)inputs[3].data);
					}
				}
			}
		);

		library->add_template("Draw Line Strip", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Points",
					.allowed_types = { TypeInfo::get<voidptr>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				},
				{
					.name = "Depth Test",
					.allowed_types = { TypeInfo::get<bool>() },
					.default_value = "false"
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto array_type = inputs[0].type;
				if (array_type && is_vector(array_type->tag))
				{
					if (array_type->get_wrapped() == TypeInfo::get<vec3>())
					{
						auto& points = *(std::vector<vec3>*)inputs[0].data;
						sRenderer::instance()->draw_primitives(PrimitiveLineStrip, points.data(), points.size(), *(cvec4*)inputs[2].data, *(bool*)inputs[3].data);
					}
				}
			}
		);

		library->add_template("Draw Line Strip Advance", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Points",
					.allowed_types = { TypeInfo::get<voidptr>() }
				},
				{
					.name = "Thickness",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Normal",
					.allowed_types = { TypeInfo::get<vec3>() },
					.default_value = "0,1,0"
				},
				{
					.name = "Material ID",
					.allowed_types = { TypeInfo::get<int>() }
				},
				{
					.name = "Col",
					.allowed_types = { TypeInfo::get<cvec4>() },
					.default_value = "255,255,255,255"
				},
				{
					.name = "Closed",
					.allowed_types = { TypeInfo::get<bool>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto array_type = inputs[0].type;
				if (array_type && is_vector(array_type->tag))
				{
					if (array_type->get_wrapped() == TypeInfo::get<vec3>())
					{
						auto& points = *(std::vector<vec3>*)inputs[0].data;
						if (points.size() >= 2)
						{
							auto thickness = *(float*)inputs[1].data;
							auto normal = *(vec3*)inputs[2].data;
							auto mat_id = *(int*)inputs[3].data;
							if (mat_id != -1)
							{
								auto color = *(cvec4*)inputs[4].data;
								auto closed = *(bool*)inputs[5].data;

								auto get_bitangent = [&](const vec3& p1, const vec3& p2) {
									return normalize(cross(p2 - p1, normal));
								};

								int n_pts = points.size();
								auto first_bitangent = get_bitangent(points[0], points[1]);
								vec3 last_bitangent = first_bitangent;

								std::vector<ParticleDrawData::Ptc> ptcs;
								auto p0 = points[0] - first_bitangent * thickness;
								auto p3 = points[0] + first_bitangent * thickness;
								for (auto i = 1; i < n_pts - 1; i++)
								{
									auto n = last_bitangent;
									last_bitangent = get_bitangent(points[i], points[i + 1]);
									n = normalize(n + last_bitangent);
									auto t = thickness / dot(n, last_bitangent);

									auto& ptc = ptcs.emplace_back();
									ptc.pos0 = p0;
									ptc.pos1 = points[i] - n * t;
									ptc.pos2 = points[i] + n * t;
									ptc.pos3 = p3;
									ptc.col = color;
									ptc.uv = vec4(0.f, 0.f, 1.f, 1.f);

									p0 = ptc.pos1;
									p3 = ptc.pos2;
								}
								{
									auto& ptc = ptcs.emplace_back();
									ptc.pos0 = p0;
									ptc.pos1 = points.back() - last_bitangent * thickness;
									ptc.pos2 = points.back() + last_bitangent * thickness;
									ptc.pos3 = p3;
									ptc.col = color;
									ptc.uv = vec4(0.f, 0.f, 1.f, 1.f);
								}
								if (closed)
								{
									if (points[n_pts - 1] != points[0])
									{
										auto n = get_bitangent(points[n_pts - 1], points[0]);
										auto n1 = normalize(n + last_bitangent);
										auto t1 = thickness / dot(n1, last_bitangent);
										ptcs.back().pos1 = points[n_pts - 1] - n1 * t1;
										ptcs.back().pos2 = points[n_pts - 1] + n1 * t1;
										auto n2 = normalize(n + first_bitangent);
										auto t2 = thickness / dot(n2, first_bitangent);
										ptcs.front().pos0 = points[0] - n2 * t2;
										ptcs.front().pos3 = points[0] + n2 * t2;

										{
											auto& ptc = ptcs.emplace_back();
											ptc.pos0 = ptcs.back().pos1;
											ptc.pos1 = ptcs.front().pos0;
											ptc.pos2 = ptcs.front().pos3;
											ptc.pos3 = ptcs.back().pos2;
											ptc.col = color;
											ptc.uv = vec4(0.f, 0.f, 1.f, 1.f);
										}
									}
									else
									{
										auto n = normalize(first_bitangent + last_bitangent);
										auto t = thickness / dot(n, last_bitangent);
										ptcs.front().pos0 = ptcs.back().pos1 = points[0] - n * t;
										ptcs.front().pos3 = ptcs.back().pos2 = points[0] + n * t;
									}
								}
								sRenderer::instance()->draw_particles(mat_id, ptcs);
							}
						}
					}
				}
			}
		);
	}
}
