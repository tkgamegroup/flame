#include <flame/surface.h>
#include <flame/system.h>
#include <flame/file.h>
#include <flame/math.h>
#include <flame/blueprint.h>
#include <flame/3DWorld/object.h>
#include <flame/3DWorld/light.h>
#include <flame/3DWorld/cell.h>
#include <flame/graphics/camera.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/sampler.h>
#include <flame/graphics/renderpath.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/graphics/UBO.h>
#include <flame/model/model.h>
#include <flame/UI/instance.h>
#include <flame/UI/widget.h>
#include <flame/UI/style.h>
#include <flame/UI/blueprint_drawing.h>
#include <flame/UI/gizmo.h>

#include <tuple>
#include <queue>
#include <process.h>
#include <Windows.h>

using namespace flame;

graphics::Device *d;
UI::Instance *ui;

Model *m;

graphics::Renderpass *rp;
graphics::Framebuffer *fb;
graphics::Image *img_dst;
graphics::Imageview *img_v_dst;
graphics::Image *img_dep;
graphics::Imageview *img_v_dep;

graphics::Renderpass *rp_sky;
graphics::Framebuffer *fb_sky;
graphics::Image *img_sky;
graphics::Imageview *img_v_sky;

graphics::Renderpass *rp_godray;
graphics::Framebuffer *fb_godray;

graphics::Renderpass *rp_combine;

graphics::Renderpass *rp_sel;
graphics::Framebuffer *fb_sel;
graphics::Image *img_sel;
graphics::Imageview *img_v_sel;

struct U_constant
{
	float zNear;
	float zFar;
	float cx;
	float cy;
	float aspect;
	float fovy;
	float tanHfFovy;
	float envrCx;
	float envrCy;
	float d0;
	float d1;
	float d2;
};
graphics::UBO<1> *ubo_constant;
U_constant *u_constant;

graphics::UBO<1> *ubo_matrix;
graphics::U_matrix *u_matrix;

graphics::UBO<2> *ubo_ins;
graphics::U_matrix *u_ins;

struct U_sky
{
	Vec4 sun_dir;
	Vec4 sun_color;
	Vec4 sun_size;
	Vec4 sky_color;
	Mat4 view_mat;
};
graphics::UBO<1> *ubo_sky;
U_sky *u_sky;

struct Us_light
{
	Vec4 pos;
	Vec4 color;
};

struct U_light
{
	Us_light points[64];
};
graphics::UBO<2> *ubo_light;
U_light *u_light;

struct U_godrayparm
{
	Vec2 pos;
	float intensity;
};
graphics::UBO<1> *ubo_godrayparm;
U_godrayparm *u_godrayparm;

graphics::Buffer *vb, *ib;

graphics::Descriptorset *ds_matrix[2];
graphics::Pipeline *p_forward_ins;
graphics::Descriptorset *ds_forward_ins[2];
graphics::Pipeline *p_sky;
graphics::Descriptorset *ds_sky;
graphics::Pipeline *p_godray;
graphics::Descriptorset *ds_godray;
graphics::Pipeline *p_frame;
graphics::Pipeline *p_sel;
graphics::Pipeline *p_combine;
graphics::Descriptorset *ds_combine;

std::vector<ThreeDWorld::Object*> objs;
std::vector<ThreeDWorld::PointLight*> lits;

std::vector<ThreeDWorld::Object*> screen_objs[2];
std::vector<ThreeDWorld::PointLight*> screen_lits[2];
ThreeDWorld::Object* sel_obj = nullptr;

ThreeDWorld::Cell3DArray *cell_array;
int ins_idx = 0;

Vec3 sun_dir;
Vec3 sun_color;
float sun_size;
Vec3 sky_color;

auto curr_tool = 0;

graphics::Commandbuffer *cb[2], *cb_sel[2];

void make_cbs(int idx) 
{
	cb[idx]->begin();

	cb[idx]->begin_renderpass(rp, fb);

	if (!screen_objs[idx].empty())
	{
		cb[idx]->bind_pipeline(p_forward_ins);
		cb[idx]->bind_descriptorset(ds_forward_ins[idx], 0);
		cb[idx]->bind_descriptorset(ds_matrix[idx], 1);
		int lit_cnt = screen_lits[idx].size();
		cb[idx]->push_constant(graphics::ShaderFrag, 0, 4, &lit_cnt);
		cb[idx]->bind_vertexbuffer(vb);
		cb[idx]->bind_indexbuffer(ib, graphics::IndiceTypeUint);
		cb[idx]->draw_indexed(m->indice_count, 0, 0, screen_objs[idx].size(), 0);

		if (sel_obj && sel_obj->show_idx != -1)
		{
			cb[idx]->bind_pipeline(p_frame);
			cb[idx]->bind_descriptorset(ds_matrix[idx], 1);
			cb[idx]->bind_vertexbuffer(vb);
			cb[idx]->draw_indexed(m->indice_count, 0, 0, 1, sel_obj->show_idx);
		}

		//cb->bind_pipeline(p_line);
		//cb->bind_descriptorset(ds_frame);
		//cb->bind_vertexbuffer(vb_lines.v);
		//cb->draw(objects.size() * 3 * 4 * 2, 1, 0);
	}

	cb[idx]->end_renderpass();

	cb[idx]->begin_renderpass(rp_sky, fb_sky);
	cb[idx]->bind_pipeline(p_sky);
	cb[idx]->bind_descriptorset(ds_sky, 0);
	cb[idx]->draw(3, 1, 0);
	cb[idx]->end_renderpass();

	cb[idx]->begin_renderpass(rp_godray, fb_godray);
	cb[idx]->bind_pipeline(p_godray);
	cb[idx]->bind_descriptorset(ds_godray, 0);
	cb[idx]->draw(3, 1, 0);
	cb[idx]->end_renderpass();

	cb[idx]->end();

	cb_sel[idx]->begin();
	cb_sel[idx]->begin_renderpass(rp_sel, fb_sel);
	if (!screen_objs[idx].empty())
	{
		cb_sel[idx]->bind_pipeline(p_sel);
		cb_sel[idx]->bind_descriptorset(ds_matrix[idx], 1);
		cb_sel[idx]->bind_vertexbuffer(vb);
		cb_sel[idx]->bind_indexbuffer(ib, graphics::IndiceTypeUint);
		cb_sel[idx]->draw_indexed(m->indice_count, 0, 0, screen_objs[idx].size(), 0);
	}
	cb_sel[idx]->end_renderpass();
	cb_sel[idx]->end();
};

long long curr_frame = 0;
long long ins_frame = 0;
long long thread_ins_frame = 0;

bool need_update_ins = true;
float ins_update_t = 0.f;

int MaxCellCollectCount = 100;
int MaxObjCount = 150;
int MaxLitCount = 8;
void collect_ins(int idx)
{
	screen_objs[idx].clear();
	screen_lits[idx].clear();
	if (sel_obj)
		sel_obj->show_idx = -1;

	cell_array->init_visit();
	std::queue<int> rest_cell;
	rest_cell.push(cell_array->cell_idx(camera.pos));
	auto cell_cnt = 0;
	while (!rest_cell.empty())
	{
		if (cell_cnt > MaxCellCollectCount)
			return;
		cell_cnt++;

		if (screen_objs[idx].size() == objs.size() &&
			screen_lits[idx].size() == lits.size())
			return;

		auto cell_idx = rest_cell.front();
		rest_cell.pop();
		if (cell_array->cells[cell_idx].visited)
			continue;
		cell_array->cells[cell_idx].visited = true;

		if ((cell_array->cell_pos(cell_idx) - camera.pos).length() > camera.zFar)
			continue;

		for (auto &obj : cell_array->cells[cell_idx].objs)
		{
			if (obj->last_show_frame[idx] != curr_frame && camera.frustum_check(obj->aabb_c, obj->aabb_r))
			{
				obj->last_show_frame[idx] = curr_frame;
				obj->show_idx = screen_objs[idx].size();
				ubo_ins->update(sizeof(Mat4) * obj->show_idx, sizeof(Mat4), &obj->mat);
				screen_objs[idx].push_back(obj);
				if (screen_objs[idx].size() >= MaxObjCount)
					break;
			}
		}

		for (auto &lit : cell_array->cells[cell_idx].lits)
		{
			if (lit->last_show_frame[idx] != curr_frame)
			{
				lit->last_show_frame[idx] = curr_frame;
				lit->show_idx = screen_lits[idx].size();
				Us_light us_light;
				us_light.pos = Vec4(lit->pos, 0.f);
				us_light.color = Vec4(lit->color, 0.f);
				ubo_light->update(sizeof(Us_light) * lit->show_idx, sizeof(Us_light), &us_light);
				screen_lits[idx].push_back(lit);
				if (screen_lits[idx].size() >= MaxLitCount)
					break;
			}
		}

		if (screen_objs[idx].size() >= MaxObjCount &&
			screen_lits[idx].size() >= MaxLitCount)
			return;

		int id;
		id = cell_array->front(cell_idx);
		if (id != -1)
			rest_cell.push(id);
		id = cell_array->back(cell_idx);
		if (id != -1)
			rest_cell.push(id);
		id = cell_array->left(cell_idx);
		if (id != -1)
			rest_cell.push(id);
		id = cell_array->right(cell_idx);
		if (id != -1)
			rest_cell.push(id);
		id = cell_array->up(cell_idx);
		if (id != -1)
			rest_cell.push(id);
		id = cell_array->down(cell_idx);
		if (id != -1)
			rest_cell.push(id);
	}
}

graphics::Commandbuffer *cb_update_thread;
void thread_ins_collect(void *)
{
	while (true)
	{
		if (thread_ins_frame <= ins_frame &&
			need_update_ins && ins_update_t > 0.2f)
		{
			auto idx = 1 - ins_idx;
			cell_array->mtx.lock();
			collect_ins(idx);
			cell_array->mtx.unlock();

			cb_update_thread->begin(true);
			ubo_ins->flush(cb_update_thread, idx);
			ubo_light->flush(cb_update_thread, idx);
			cb_update_thread->end();
			d->tq->submit(cb_update_thread, nullptr, nullptr);
			d->tq->wait_idle();

			make_cbs(idx);

			thread_ins_frame = curr_frame;
			need_update_ins = false;
			ins_update_t = 0.f;
		}
		else
			Sleep(16);
	}
}

void create_obj(const Vec3 &pos)
{
	auto idx = objs.size();

	auto obj = new ThreeDWorld::Object;
	obj->pos = pos;
	obj->euler = EulerYawPitchRoll(0.f, 0.f, 0.f);
	obj->scale = Vec3(1.f);
	obj->m = m;
	obj->aabb = m->aabb + obj->pos;
	obj->aabb_c = obj->aabb.center();
	obj->aabb_r = max(max(obj->aabb.width(), obj->aabb.height()), obj->aabb.depth()) / 2.f;
	obj->update_mat();
	obj->last_show_frame[0] = 0;
	obj->last_show_frame[1] = 0;
	obj->show_idx = -1;
	cell_array->mtx.lock();
	objs.push_back(obj);
	cell_array->add_obj(obj);
	cell_array->mtx.unlock();

	need_update_ins = true;

	//vb_lines.set(idx, *obj);
}

//struct VbLines
//{
//	graphics::Buffer *v;
//	Vec3 *pp;
//
//	void init()
//	{
//		v = graphics::create_buffer(d, sizeof(Vec3) * 65536 * 24,  graphics::BufferUsageVertexBuffer,
//			graphics::MemPropHost | graphics::MemPropHostCoherent);
//		v->map();
//		pp = (Vec3*)v->mapped;
//	}
//
//	void set(int index, const ThreeDWorld::Object &o)
//	{
//		Vec3 c[8];
//		o.aabb.get_points(c);
//
//		auto p = pp + 24 * index;
//
//		*p = c[0]; p++;
//		*p = c[1]; p++;
//		*p = c[1]; p++;
//		*p = c[2]; p++;
//		*p = c[2]; p++;
//		*p = c[3]; p++;
//		*p = c[3]; p++;
//		*p = c[0]; p++;
//
//		*p = c[4]; p++;
//		*p = c[5]; p++;
//		*p = c[5]; p++;
//		*p = c[6]; p++;
//		*p = c[6]; p++;
//		*p = c[7]; p++;
//		*p = c[7]; p++;
//		*p = c[4]; p++;
//
//		*p = c[0]; p++;
//		*p = c[4]; p++;
//		*p = c[1]; p++;
//		*p = c[5]; p++;
//		*p = c[2]; p++;
//		*p = c[6]; p++;
//		*p = c[3]; p++;
//		*p = c[7]; p++;
//	}
//};

auto day = true;

int main(int argc, char **args)
{
	{
		std::ifstream file("performance_spec.txt");
		if (file.good())
		{
			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);
				if (line == "MaxCellCollectCount")
				{
					std::getline(file, line);
					MaxCellCollectCount = std::stoi(line);
				}
				else if (line == "MaxObjCount")
				{
					std::getline(file, line);
					MaxObjCount = std::stoi(line);
				}
				else if (line == "MaxLitCount")
				{
					std::getline(file, line);
					MaxLitCount = std::stoi(line);
				}
			}
			file.close();
		}
	}

	Ivec2 res(1280, 720);

	camera.calc_dir();
	camera.update_view();
	camera.update_frustum_plane();

	cell_array = new ThreeDWorld::Cell3DArray(40, 100.f);

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res, SurfaceStyleFrame, "Scene Editor");

	d = graphics::create_device(true);

	auto sc = graphics::create_swapchain(d, s);

	auto cb_update = d->gcp->create_commandbuffer();
	cb_update_thread = d->tcp->create_commandbuffer();

	ubo_constant = new graphics::UBO<1>(d, sizeof(U_constant));
	ubo_matrix = new graphics::UBO<1>(d, sizeof(graphics::U_matrix));
	ubo_ins = new graphics::UBO<2>(d, sizeof(graphics::U_ins));
	ubo_sky = new graphics::UBO<1>(d, sizeof(U_sky));
	ubo_light = new graphics::UBO<2>(d, sizeof(U_light));
	ubo_godrayparm = new graphics::UBO<1>(d, sizeof(U_godrayparm));

	{
		u_constant = (U_constant*)ubo_constant->stag->mapped;
		u_constant->aspect = camera.aspect;
		u_constant->tanHfFovy = tan(camera.fovy / 2.f * ANG_RAD);
		ubo_constant->update();

		ubo_matrix->update(0, sizeof(Mat4), &camera.proj);
		ubo_matrix->update(sizeof(Mat4), sizeof(Mat4), &camera.view);

		sun_dir = Vec3(0.f, 1.f, 0.f);
		sun_color = Vec3(1.f);
		sun_size = 120.f;
		sky_color = Vec3(0.3f, 0.3f, 0.8f);
		u_sky = (U_sky*)ubo_sky->stag->mapped;
		u_sky->sun_dir = Vec4(sun_dir, 0.f);
		u_sky->sun_color = Vec4(sun_color, 0.f);
		u_sky->sun_size = Vec4(sun_size, 0.f, 0.f, 0.f);
		u_sky->sky_color = Vec4(sky_color, 0.f);
		ubo_sky->update();

		u_godrayparm = (U_godrayparm*)ubo_godrayparm->stag->mapped;
		u_godrayparm->pos = Vec2(0.5f);
		ubo_godrayparm->update();

		cb_update->begin(true);
		ubo_constant->flush(cb_update, 0);
		ubo_matrix->flush(cb_update, 0);
		ubo_sky->flush(cb_update, 0);
		ubo_godrayparm->flush(cb_update, 0);
		cb_update->end();
		d->gq->submit(cb_update, nullptr, nullptr);
		d->gq->wait_idle();
	}

	{
		std::ifstream file("where_is_your_model.txt");
		if (!file.good())
			return 0;
		std::string line;
		std::getline(file, line);

		ModelDescription mod_desc;
		mod_desc.set_to_default();
		mod_desc.need_AABB = true;
		m = load_model(&mod_desc, line.c_str());

		file.close();
	}

	graphics::Renderpath *render_path;
	{
		auto fn = "1col_dep.rp";
		if (!std::filesystem::exists(fn))
			return 0;

		auto bps = blueprint::create_scene();
		bps->register_from_path("blueprint/nodes/math");
		bps->register_from_path("blueprint/nodes/graphics");
		bps->load(fn);
		render_path = graphics::create_renderpath(d, bps);
	}

	rp = render_path->get_renderpass_by_tag("rp");
	fb = render_path->get_framebuffer_by_tag("rp");
	img_dst = render_path->get_image_by_tag("dst");
	img_v_dst = img_dst->get_view();
	img_dep = render_path->get_image_by_tag("dep");
	img_v_dep = img_dep->get_view();

	img_sky = graphics::create_image(d, res, 1, 1, graphics::Format_R8G8B8A8_UNORM,
		graphics::ImageUsageAttachment | graphics::ImageUsageShaderSampled,
		graphics::MemPropDevice);
	img_v_sky = img_sky->get_view();
	graphics::RenderpassAttachmentInfo rp_sky_att_info[] = {
		{ graphics::Format_R8G8B8A8_UNORM, false },
		{ graphics::Format_R8G8B8A8_UNORM, true },
		{ graphics::Format_Depth16, false }
	};
	int rp_sky_col_att[] = {
		0, 1
	};
	graphics::RenderpassSubpassInfo rp_sky_sp_info[] = {
		{ rp_sky_col_att, 2, 2 }
	};
	graphics::RenderpassInfo rp_sky_info;
	rp_sky_info.attachment_count = 3;
	rp_sky_info.attachments = rp_sky_att_info;
	rp_sky_info.subpass_count = 1;
	rp_sky_info.subpasses = rp_sky_sp_info;
	rp_sky = graphics::get_renderpass(d, &rp_sky_info);
	graphics::Imageview *fb_sky_vs[] = {
		img_v_dst,
		img_v_sky,
		img_v_dep
	};
	fb_sky = graphics::get_framebuffer(d, res, rp_sky, 3, fb_sky_vs);

	graphics::RenderpassAttachmentInfo rp_godray_att_info[] = {
		{ graphics::Format_R8G8B8A8_UNORM, false }
	};
	int rp_godray_col_att = 0;
	graphics::RenderpassSubpassInfo rp_godray_sp_info[] = {
		{ &rp_godray_col_att, 1, -1 }
	};
	graphics::RenderpassInfo rp_godray_info;
	rp_godray_info.attachment_count = 1;
	rp_godray_info.attachments = rp_godray_att_info;
	rp_godray_info.subpass_count = 1;
	rp_godray_info.subpasses = rp_godray_sp_info;
	rp_godray = graphics::get_renderpass(d, &rp_godray_info);
	graphics::Imageview *fb_godray_vs[] = {
		img_v_dst
	};
	fb_godray = graphics::get_framebuffer(d, res, rp_godray, 1, fb_godray_vs);

	graphics::RenderpassAttachmentInfo rp_combine_att_info[] = {
		{ graphics::Format_R8G8B8A8_UNORM, false }
	};
	int rp_combine_col_att = 0;
	graphics::RenderpassSubpassInfo rp_combine_sp_info[] = {
		{ &rp_combine_col_att, 1, -1 }
	};
	graphics::RenderpassInfo rp_combine_info;
	rp_combine_info.attachment_count = 1;
	rp_combine_info.attachments = rp_combine_att_info;
	rp_combine_info.subpass_count = 1;
	rp_combine_info.subpasses = rp_combine_sp_info;
	rp_combine = graphics::get_renderpass(d, &rp_combine_info);

	img_sel = graphics::create_image(d, res, 1, 1, graphics::Format_R8G8B8A8_UNORM,
		graphics::ImageUsageAttachment | graphics::ImageUsageShaderSampled | graphics::ImageUsageTransferSrc,
		graphics::MemPropDevice);
	img_v_sel = img_sel->get_view();
	graphics::RenderpassAttachmentInfo rp_sel_att_info[] = {
		{ graphics::Format_R8G8B8A8_UNORM, true },
		{ graphics::Format_Depth16, false }
	};
	int rp_sel_col_att[] = {
		0
	};
	graphics::RenderpassSubpassInfo rp_sel_sp_info[] = {
		{ rp_sel_col_att, 1, 1 }
	};
	graphics::RenderpassInfo rp_sel_info;
	rp_sel_info.attachment_count = 2;
	rp_sel_info.attachments = rp_sel_att_info;
	rp_sel_info.subpass_count = 1;
	rp_sel_info.subpasses = rp_sel_sp_info;
	rp_sel = graphics::get_renderpass(d, &rp_sel_info);
	graphics::Imageview *fb_sel_vs[] = {
		img_v_sel,
		img_v_dep
	};
	fb_sel = graphics::get_framebuffer(d, res, rp_sel, 2, fb_sel_vs);

	auto vert = graphics::create_shader(d, "forward_ins.vert");
	vert->build();
	auto frag = graphics::create_shader(d, "forward_ins.frag");
	frag->build();

	p_forward_ins = create_pipeline(d);
	p_forward_ins->set_renderpass(rp, 0);
	p_forward_ins->set_vertex_attributes({ {
		graphics::VertexAttributeFloat3,
		graphics::VertexAttributeFloat2,
		graphics::VertexAttributeFloat3
	} });
	p_forward_ins->set_size(res);
	p_forward_ins->set_depth_test(true);
	p_forward_ins->set_depth_write(true);
	p_forward_ins->add_shader(vert);
	p_forward_ins->add_shader(frag);
	p_forward_ins->build_graphics();

	for (auto i = 0; i < 2; i++)
	{
		ds_matrix[i] = d->dp->create_descriptorset(p_forward_ins, 1);
		ds_matrix[i]->set_uniformbuffer(0, 0, ubo_matrix->v[0]);
		ds_matrix[i]->set_uniformbuffer(1, 0, ubo_ins->v[i]);

		ds_forward_ins[i] = d->dp->create_descriptorset(p_forward_ins, 0);
		ds_forward_ins[i]->set_uniformbuffer(0, 0, ubo_sky->v[0]);
		ds_forward_ins[i]->set_uniformbuffer(1, 0, ubo_light->v[i]);
	}

	auto vert_sky = graphics::create_shader(d, "fullscreen.vert");
	vert_sky->add_define("DEPTH_1");
	vert_sky->add_define("USE_VIEW");
	vert_sky->build();
	auto frag_sky = graphics::create_shader(d, "simple_sky.frag");
	frag_sky->build();

	p_sky = create_pipeline(d);
	p_sky->set_renderpass(rp_sky, 0);
	p_sky->set_size(res);
	p_sky->set_cull_mode(graphics::CullModeNone);
	p_sky->set_depth_compare_op(graphics::CompareOpLessOrEqual);
	p_sky->set_depth_test(true);
	p_sky->set_depth_write(false);
	p_sky->add_shader(vert_sky);
	p_sky->add_shader(frag_sky);
	p_sky->set_output_attachment_count(2);
	p_sky->build_graphics();

	ds_sky = d->dp->create_descriptorset(p_sky, 0);
	ds_sky->set_uniformbuffer(0, 0, ubo_constant->v[0]);
	ds_sky->set_uniformbuffer(1, 0, ubo_sky->v[0]);

	auto vert_godray = graphics::create_shader(d, "fullscreen.vert");
	vert_godray->add_define("USE_UV");
	vert_godray->build();
	auto frag_godray = graphics::create_shader(d, "godray.frag");
	frag_godray->build();

	p_godray = create_pipeline(d);
	p_godray->set_renderpass(rp_godray, 0);
	p_godray->set_size(res);
	p_godray->set_cull_mode(graphics::CullModeNone);
	p_godray->set_depth_test(false);
	p_godray->set_depth_write(false);
	p_godray->add_shader(vert_godray);
	p_godray->add_shader(frag_godray);
	p_godray->set_blend_state(0, true, graphics::BlendFactorOne, graphics::BlendFactorOne,
		graphics::BlendFactorOne, graphics::BlendFactorOne);
	p_godray->build_graphics();

	ds_godray = d->dp->create_descriptorset(p_godray, 0);
	ds_godray->set_uniformbuffer(0, 0, ubo_godrayparm->v[0]);
	auto sampler = graphics::create_sampler(d, graphics::FilterLinear, graphics::FilterLinear,
		false);
	ds_godray->set_imageview(1, 0, img_v_sky, sampler);

	vb = create_buffer(d, m->vertex_count * m->vertex_buffers[0].size * sizeof(float), graphics::BufferUsageVertexBuffer |
		graphics::BufferUsageTransferDst, graphics::MemPropDevice);
	ib = create_buffer(d, m->indice_count * sizeof(int), graphics::BufferUsageIndexBuffer |
		graphics::BufferUsageTransferDst, graphics::MemPropDevice);
	auto sb = create_buffer(d, vb->size + ib->size, graphics::BufferUsageTransferSrc,
		graphics::MemPropHost | graphics::MemPropHostCoherent);
	sb->map();
	{
		auto c = d->gcp->create_commandbuffer();
		c->begin(true);
		memcpy(sb->mapped, m->vertex_buffers[0].pVertex, vb->size);
		graphics::BufferCopy r1 = { 0, 0, vb->size };
		c->copy_buffer(sb, vb, 1, &r1);
		memcpy((unsigned char*)sb->mapped + vb->size, m->pIndices, ib->size);
		graphics::BufferCopy r2 = { vb->size, 0, ib->size };
		c->copy_buffer(sb, ib, 1, &r2);
		c->end();
		d->gq->submit(c, nullptr, nullptr);
		d->gq->wait_idle();
		d->gcp->destroy_commandbuffer(c);
	}
	sb->unmap();
	destroy_buffer(sb);

	for (auto i = 0; i < 2; i++)
	{
		cb[i] = d->gcp->create_commandbuffer();
		cb[i]->begin();
		cb[i]->end();
	}

	auto vert_frame = graphics::create_shader(d, "frame.vert");
	vert_frame->build();
	auto frag_frame = graphics::create_shader(d, "frame.frag");
	frag_frame->build();
	p_frame = create_pipeline(d);
	p_frame->set_renderpass(rp, 0);
	p_frame->set_vertex_attributes({ {
			graphics::VertexAttributeFloat3,
			graphics::VertexAttributeFloat2,
			graphics::VertexAttributeFloat3
		} });
	p_frame->set_size(res);
	p_frame->set_depth_test(true);
	p_frame->set_depth_write(true);
	p_frame->set_cull_mode(graphics::CullModeFront);
	p_frame->add_shader(vert_frame);
	p_frame->add_shader(frag_frame);
	p_frame->build_graphics();

	auto vert_line = graphics::create_shader(d, "test/line3d.vert");
	vert_line->build();
	auto frag_line = graphics::create_shader(d, "test/line3d.frag");
	frag_line->build();
	auto p_line = create_pipeline(d);
	p_line->set_renderpass(rp, 0);
	p_line->set_vertex_attributes({ {
		graphics::VertexAttributeFloat3
	} });
	p_line->set_size(res);
	p_line->set_primitive_topology(graphics::PrimitiveTopologyLineList);
	p_line->set_polygon_mode(graphics::PolygonModeLine);
	p_line->set_depth_test(false);
	p_line->set_depth_write(false);
	p_line->set_cull_mode(graphics::CullModeNone);
	p_line->add_shader(vert_line);
	p_line->add_shader(frag_line);
	p_line->build_graphics();

	auto ds_line = d->dp->create_descriptorset(p_line, 0);
	ds_line->set_uniformbuffer(0, 0, ubo_matrix->v[0]);

	//VbLines vb_lines;
	//vb_lines.init();

	auto vert_sel = graphics::create_shader(d, "select.vert");
	vert_sel->build();
	auto frag_sel = graphics::create_shader(d, "select.frag");
	frag_sel->build();
	p_sel = create_pipeline(d);
	p_sel->set_renderpass(rp_sel, 0);
	p_sel->set_vertex_attributes({ {
		graphics::VertexAttributeFloat3,
		graphics::VertexAttributeFloat2,
		graphics::VertexAttributeFloat3
	} });
	p_sel->set_size(res);
	p_sel->set_depth_test(true);
	p_sel->set_depth_write(false);
	p_sel->set_depth_compare_op(graphics::CompareOpEqual);
	p_sel->add_shader(vert_sel);
	p_sel->add_shader(frag_sel);
	p_sel->build_graphics();

	for (auto i = 0; i < 2; i++)
	{
		cb_sel[i] = d->gcp->create_commandbuffer();
		cb_sel[i]->begin();
		cb_sel[i]->end();
	}
	auto sel_buf = create_buffer(d, 4, graphics::BufferUsageTransferDst,
		graphics::MemPropHost | graphics::MemPropHostCoherent);
	sel_buf->map();
	auto cb_get = d->gcp->create_commandbuffer();

	auto cb_ui = d->gcp->create_commandbuffer();
	cb_ui->begin();
	cb_ui->end();

	auto image_avalible = graphics::create_semaphore(d);
	auto render_finished = graphics::create_semaphore(d);
	auto ui_finished = graphics::create_semaphore(d);

	ui = UI::create_instance(d, sc->get_rp(true), s);
	ui->build();
	auto dst_img_ui_idx = ui->find_and_set_img_view(img_dst->get_view());

	auto gizmo = UI::create_gizmo();

	make_cbs(ins_idx);
	_beginthread(thread_ins_collect, 0, nullptr);

	sm->run([&](){
		curr_frame = sm->total_frame;

		auto view_changed = false;

		if (s->pressing_M(1))
		{
			x_ang -= (s->mouse_disp.x) * 180.f / s->size.x;
			y_ang -= (s->mouse_disp.y) * 180.f / s->size.y;
			view_changed = true;
		}

		if (view_changed)
		{
			camera.x_ang += x_ang;
			camera.y_ang += y_ang;
			camera.calc_dir();
			camera.pos += z_sp * sp * camera.view_dir + x_sp * sp * camera.x_dir + y_sp * sp * camera.up_dir;
			camera.update_view();
			camera.update_frustum_plane();
			ubo_matrix->update(sizeof(Mat4), sizeof(Mat4), &camera.view);

			need_update_ins = true;
		}

		ins_update_t += sm->elapsed_time;

		{
			if (dot(camera.view_dir, sun_dir) > 0.f)
			{
				auto p = camera.proj_view * Vec4(sun_dir, 0.f);
				if (p.w != 0.f)
					p /= p.w;
				u_godrayparm->pos = Vec2(p) * 0.5f + 0.5f;
				u_godrayparm->intensity = 1.f;
			}
			else
				u_godrayparm->intensity = 0.f;
			ubo_godrayparm->update();
		}

		if (thread_ins_frame > ins_frame)
		{
			ins_idx = 1 - ins_idx;
			ins_frame = thread_ins_frame;
		}

		ui->begin(s->size.x, s->size.y, sm->elapsed_time);

		ui->begin_mainmenu();
		if (ui->begin_menu("Demo"))
		{
			if (ui->menuitem("Create Object"))
				create_obj(camera.pos);
			if (ui->menuitem("Create 100w Objects"))
			{
				for (auto i = 0; i < 1000000; i++)
				{
					auto x = rand() % 3900 - 1950;
					auto y = rand() % 3900 - 1950;
					auto z = rand() % 3900 - 1950;
					create_obj(Vec3(x, y, z));
				}
			}
			if (ui->menuitem("Center Camera"))
			{
				AABB aabb;
				aabb.reset();
				for (auto &o : objs)
					aabb.merge(o->aabb);
				auto c = aabb.center();
				Vec3 p[8];
				aabb.get_points(p);

				if (c == camera.pos)
					camera.pos += Vec3(1.f, 0.f, 0.f);

				auto v = c - camera.pos;
				v.normalize();
				auto s = cross(v, Vec3(0.f, 1.f, 0.f));
				s.normalize();
				auto u = cross(s, v);
				u.normalize();
				auto m = Mat3(s, u, -v);
				m.transpose();
				aabb.reset();
				for (auto i = 0; i < 8; i++)
				{
					p[i] = m * p[i];
					aabb.merge(p[i]);
				}

				auto z = aabb.depth();
				auto tanHfFovy = tan(camera.fovy * ANG_RAD / 2.f);
				z = max(z, aabb.height() / tanHfFovy);
				z = max(z, aabb.width() / camera.aspect / tanHfFovy);
				if (z > camera.zFar)
					z = camera.zFar;
				camera.view_dir = v;
				camera.x_dir = s;
				camera.up_dir = u;
				camera.calc_ang();
				camera.pos = c + -v * (z / 2.f);
				camera.update_view();
				camera.update_frustum_plane();
				ubo_matrix->update(sizeof(Mat4), sizeof(Mat4), &camera.view);
			}
			if (ui->menuitem("Day", nullptr, day))
			{
				if (!day)
				{
					sun_color = Vec3(1.f);
					sky_color = Vec3(0.3f, 0.3f, 0.8f);
					u_sky->sun_color = Vec4(sun_color, 0.f);
					u_sky->sky_color = Vec4(sky_color, 0.f);
					ubo_sky->update();

					cell_array->mtx.lock();
					cell_array->clear_lits();
					for (auto &l : lits)
						delete l;
					lits.clear();
					cell_array->mtx.unlock();

					need_update_ins = true;
					day = true;
				}
			}
			if (ui->menuitem("Night", nullptr, !day))
			{
				if (day)
				{
					sun_color = Vec3(0.f);
					sky_color = Vec3(0.f);
					u_sky = (U_sky*)ubo_sky->stag->mapped;
					u_sky->sun_color = Vec4(sun_color, 0.f);
					u_sky->sky_color = Vec4(sky_color, 0.f);
					ubo_sky->update();

					cell_array->mtx.lock();
					for (auto &o : objs)
					{
						auto l = new ThreeDWorld::PointLight;
						l->pos = Vec3(
							(rand() % 20) - 10,
							(rand() % 20) - 10,
							(rand() % 20) - 10) + o->pos;
						l->color = Vec3(
							(rand() % 256) / 255.f,
							(rand() % 256) / 255.f,
							(rand() % 256) / 255.f);
						lits.push_back(l);
						cell_array->add_lit(l);
					}
					cell_array->mtx.unlock();

					need_update_ins = true;
					day = false;
				}
			}
			if (ui->menuitem("Information"))
			{
				ui->add_message_dialog("Info", "Press 'W' 'A' 'S' 'D' and right click to move camera\n"
					"Press '1' '2' '3' '4' to change tools");
			}
			ui->end_menu();
		}
		auto menu_rect = ui->get_curr_window_rect();
		ui->end_mainmenu();

		ui->begin_plain_window("bg", Vec2(0.f, menu_rect.max.y), Vec2(res), true);
		ui->image(dst_img_ui_idx, Vec2(res));
		auto img_rect = ui->get_last_item_rect();

		if (!gizmo->is_using() && ui->is_last_item_hovered()
			&& s->just_down_M(0))
		{
			d->gq->submit(cb_sel[ins_idx], nullptr, nullptr);
			d->gq->wait_idle();

			cb_get->begin();
			cb_get->change_image_layout(img_sel,
				graphics::ImageLayoutShaderReadOnly, graphics::ImageLayoutTransferSrc);
			graphics::BufferTextureCopy cpy;
			cpy.buffer_offset = 0;
			cpy.image_width = 1;
			cpy.image_height = 1;
			cpy.image_x = s->mouse_pos.x - img_rect.min.x;
			cpy.image_y = s->mouse_pos.y - img_rect.min.y;
			cpy.image_level = 0;
			cb_get->copy_image_to_buffer(img_sel, sel_buf, 1, &cpy);
			cb_get->change_image_layout(img_sel,
				graphics::ImageLayoutTransferSrc, graphics::ImageLayoutAttachment);
			cb_get->end();

			d->gq->submit(cb_get, nullptr, nullptr);
			d->gq->wait_idle();

			uchar bytes[4];
			memcpy(bytes, sel_buf->mapped, 4);
			auto id = bytes[0] +
				(bytes[1] << 8) +
				(bytes[2] << 16) +
				(bytes[3] << 24);
			if (id == 0)
				sel_obj = nullptr;
			else
				sel_obj = screen_objs[ins_idx][id - 1];

			need_update_ins = true;
		}

		if (s->just_down_K(Key_1))
			curr_tool = 0;
		if (s->just_down_K(Key_2))
			curr_tool = 1;
		if (s->just_down_K(Key_3))
			curr_tool = 2;
		if (s->just_down_K(Key_4))
			curr_tool = 3;

		if (curr_tool > 0 && sel_obj && sel_obj->show_idx != -1)
		{
			UI::Gizmo::TransType type;
			switch (curr_tool)
			{
			case 1:
				type = UI::Gizmo::TransMove;
				break;
			case 2:
				type = UI::Gizmo::TransRotate;
				break;
			case 3:
				type = UI::Gizmo::TransScale;
				break;

			}
			if (gizmo->show(ui, type, &camera, sel_obj))
			{
				sel_obj->update_mat();
				ubo_ins->update(sizeof(Mat4) * sel_obj->show_idx, sizeof(Mat4), &sel_obj->mat);
			}
		}

		ui->end_window();

		auto dl_ol = ui->get_overlap_drawlist();
		dl_ol.add_text(Vec2(0.f, s->size.y - 20.f), Vec4(1.f), "%5.2f %5.2f %5.2f",
			camera.pos.x, camera.pos.y, camera.pos.z);

		ui->end();

		ubo_sky->update(sizeof(Vec4) * 4, sizeof(Mat4), &camera.view);

		cb_update->begin(true);
		ubo_matrix->flush(cb_update, 0);
		ubo_ins->flush(cb_update, ins_idx);
		ubo_sky->flush(cb_update, 0);
		ubo_godrayparm->flush(cb_update, 0);
		cb_update->end();
		d->gq->submit(cb_update, nullptr, nullptr);
		d->gq->wait_idle();

		auto index = sc->acquire_image(image_avalible);

		cb_ui->begin();
		ui->record_commandbuffer(cb_ui, sc->get_rp(true), sc->get_fb(index));
		cb_ui->end();

		d->gq->submit(cb[ins_idx], image_avalible, render_finished);
		d->gq->submit(cb_ui, render_finished, ui_finished);
		d->gq->wait_idle();
		d->gq->present(index, sc, ui_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}

