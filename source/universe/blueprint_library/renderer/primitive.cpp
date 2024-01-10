#include "../../../foundation/blueprint.h"
#include "../../systems/renderer_private.h"

namespace flame
{
	void add_primitive_node_templates(BlueprintNodeLibraryPtr library)
	{
		constexpr auto SEPARATOR = std::numeric_limits<float>::quiet_NaN();

		library->add_template("Make Line Strips", "", BlueprintNodeFlagNone,
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
						auto& strips = *(std::vector<vec3>*)inputs[2].data;
						auto pt0 = *(vec3*)inputs[0].data;
						auto pt1 = *(vec3*)inputs[1].data;
						auto it0 = std::find_if(strips.begin(), strips.end(), [&](const auto& i) {
							return distance(i, pt0) < 0.01f;
						});
						auto it1 = std::find_if(strips.begin(), strips.end(), [&](const auto& i) {
							return distance(i, pt1) < 0.01f;
						});

						if (it0 != strips.end() && it1 != strips.end())
						{
							if (it1 == strips.begin())
								strips.push_back(strips[0]);
							else
							{
								auto n = 0;
								for (auto it = it1; it != strips.end(); it++)
								{
									if (isnan(it->x))
										break;
									n++;
								}

								auto copied = std::vector<vec3>(it1, it1 + n);
								{
									auto b = it1;
									auto e = it1 + n;
									b = b == strips.begin() ? b : b - 1;
									e = e == strips.end() ? e : e + 1;
									strips.erase(b, e);
								}
								it0 = std::find_if(strips.begin(), strips.end(), [&](const auto& i) {
									return distance(i, pt0) < 0.01f;
								});
								strips.insert(it0 + 1, copied.begin(), copied.end());
							}
						}
						else if (it0 != strips.end())
							strips.emplace(it0 + 1, pt1);
						else if (it1 != strips.end())
							strips.emplace(it1, pt0);
						else
						{
							if (!strips.empty())
								strips.push_back(vec3(SEPARATOR));
							strips.push_back(pt0);
							strips.push_back(pt1);
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

		library->add_template("Draw Line Strips", "", BlueprintNodeFlagNone,
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

		library->add_template("Draw Line Strips Advance", "", BlueprintNodeFlagNone,
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

								auto off = 0;
								int n_pts = 0;
								for (auto i = 0; ; i++)
								{
									if (i < points.size() && !isnan(points[i].x))
										n_pts++;
									else if (n_pts >= 2)
									{
										auto first_bitangent = get_bitangent(points[off], points[off + 1]);
										vec3 last_bitangent = first_bitangent;

										std::vector<ParticleDrawData::Ptc> ptcs;
										auto p0 = points[off] - first_bitangent * thickness;
										auto p3 = points[off] + first_bitangent * thickness;
										for (auto j = 1; j < n_pts - 1; j++)
										{
											auto n = last_bitangent;
											last_bitangent = get_bitangent(points[off + j], points[off + j + 1]);
											n = normalize(n + last_bitangent);
											auto t = thickness / dot(n, last_bitangent);

											auto& ptc = ptcs.emplace_back();
											ptc.pos0 = p0;
											ptc.pos1 = points[off + j] - n * t;
											ptc.pos2 = points[off + j] + n * t;
											ptc.pos3 = p3;
											ptc.col = color;
											ptc.uv = vec4(0.f, 0.f, 1.f, 1.f);

											p0 = ptc.pos1;
											p3 = ptc.pos2;
										}
										{
											auto& ptc = ptcs.emplace_back();
											ptc.pos0 = p0;
											ptc.pos1 = points[off + n_pts - 1] - last_bitangent * thickness;
											ptc.pos2 = points[off + n_pts - 1] + last_bitangent * thickness;
											ptc.pos3 = p3;
											ptc.col = color;
											ptc.uv = vec4(0.f, 0.f, 1.f, 1.f);
										}
										if (closed)
										{
											if (distance(points[off + n_pts - 1], points[off]) > 0.01f)
											{
												auto n = get_bitangent(points[off + n_pts - 1], points[off]);
												auto n1 = normalize(n + last_bitangent);
												auto t1 = thickness / dot(n1, last_bitangent);
												ptcs.back().pos1 = points[off + n_pts - 1] - n1 * t1;
												ptcs.back().pos2 = points[off + n_pts - 1] + n1 * t1;
												auto n2 = normalize(n + first_bitangent);
												auto t2 = thickness / dot(n2, first_bitangent);
												ptcs.front().pos0 = points[off] - n2 * t2;
												ptcs.front().pos3 = points[off] + n2 * t2;

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
												ptcs.front().pos0 = ptcs.back().pos1 = points[off] - n * t;
												ptcs.front().pos3 = ptcs.back().pos2 = points[off] + n * t;
											}
										}

										sRenderer::instance()->draw_particles(mat_id, ptcs);
										n_pts = 0;
										off = i + 1;
									}
									if (i >= points.size())
										break;
								}
							}
						}
					}
				}
			}
		);
	}
}
