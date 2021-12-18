//#include <flame/foundation/bitmap.h>
//#include <flame/database/database.h>
//#include <flame/graphics/buffer.h>
//#include <flame/universe/components/text.h>
//#include <flame/universe/components/image.h>
//#include <flame/universe/app.h>
//
//using namespace flame;
//using namespace graphics;
//
//App g_app;
//
//database::Connection* db;
//std::filesystem::path stored_path = "E:/";
//
//Entity* root = nullptr;
//
//const auto ThumbnailSize = 96U;
//
//struct DynamicAtlas
//{
//	uint cx, cy;
//	uint size;
//	Image* image;
//	uint id;
//	std::vector<uint> map;
//
//	CommandBuffer* cb;
//	Buffer* buf;
//
//	void create(uint _cx, uint _cy, uint _size)
//	{
//		cx = _cx;
//		cy = _cy;
//		size = _size;
//		image = Image::create(nullptr, Format_R8G8B8A8_UNORM, uvec2(cx * size, cy * size), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled);
//		id = canvas->set_element_resource(-1, { image->get_view(), nullptr, nullptr });
//		map.resize(cx * cy);
//		for (auto i = 0; i < map.size(); i++)
//			map[i] = 0;
//
//		cb = CommandBuffer::create(CommandPool::get(nullptr));
//		buf = Buffer::create(nullptr, sizeof(cvec4) * size * size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent);
//		buf->map();
//	}
//
//	uint alloc_image(uint w, uint h, uchar* data)
//	{
//		assert(w <= size && h <= size);
//		for (auto i = 0; i < map.size(); i++)
//		{
//			if (map[i] == 0)
//			{
//				memcpy(buf->get_mapped(), data, sizeof(cvec4) * w * h);
//				cb->begin(true);
//				BufferImageCopy cpy;
//				cpy.image_offset.x = size * (i % cx);
//				cpy.image_offset.y = size * (i / cx);
//				cpy.image_extent = uvec2(w, h);
//				cb->copy_buffer_to_image(buf, image, 1, &cpy);
//				cb->end();
//				auto q = Queue::get(nullptr);
//				q->submit(1, &cb, nullptr, nullptr, nullptr);
//				q->wait_idle();
//				map[i] = 1;
//				return i;
//			}
//		}
//		return -1;
//	}
//
//	void free_image(uint id)
//	{
//		map[id] = 0;
//	}
//}thumbnails_atlas;
//
//struct cThumbnail : Component
//{
//	cElement* element;
//	uint w, h;
//	uchar* data;
//	uint id = -1;
//
//	cThumbnail() :
//		Component("cThumbnail", S<"cThumbnail"_h>)
//	{
//	}
//
//	~cThumbnail()
//	{
//		id = thumbnails_atlas.alloc_image(w, h, data);
//	}
//
//	void toggle()
//	{
//		if (element->get_culled())
//		{
//			thumbnails_atlas.free_image(id);
//			id = -1;
//		}
//		else
//			id = thumbnails_atlas.alloc_image(w, h, data);
//		looper().add_event([](Capture& c) {
//			c.thiz<cElement>()->mark_drawing_dirty();
//		}, Capture().set_thiz(element));
//	}
//};
//
//struct Item
//{
//	std::filesystem::path filename;
//
//	void get_filename(const std::filesystem::path& fn, const std::filesystem::path& ext)
//	{
//		filename = stored_path;
//		filename /= fn;
//		filename += ext;
//	}
//};
//std::vector<Item> items;
//
//struct Tag
//{
//	std::wstring name;
//};
//std::vector<Tag> tags;
//
//struct ImageView
//{
//	Image* image = nullptr;
//	Entity* e = nullptr;
//	cImage* c_image = nullptr;
//	uint res_id = -1;
//	uint id;
//
//	void init(Entity* e)
//	{
//		res_id = canvas->set_element_resource(-1, { nullptr, nullptr, nullptr });
//	}
//
//	void show(uint _id)
//	{
//		id = _id;
//		if (image)
//			image->release();
//		image = Image::create(nullptr, items[id].filename.c_str(), true);
//		canvas->set_element_resource(res_id, { image->get_view(), nullptr, nullptr });
//
//		if (!e)
//		{
//			e = Entity::create();
//			e->load(L"image_view");
//			c_image = e->find_child("image")->get_component_t<cImage>();
//			c_image->set_res_id(res_id);
//			e->get_component_t<cReceiver>()->add_key_down_listener([](Capture& c, KeyboardKey key) {
//				auto thiz = c.thiz<ImageView>();
//				switch (key)
//				{
//				case Keyboard_Esc:
//					looper().add_event([](Capture& c) {
//						auto thiz = c.thiz<ImageView>();
//						thiz->close();
//					}, Capture().set_thiz(thiz));
//					break;
//				case Keyboard_Left:
//					if (thiz->id > 0)
//						thiz->show(thiz->id - 1);
//					break;
//				case Keyboard_Right:
//					if (thiz->id < items.size() - 1)
//						thiz->show(thiz->id + 1);
//					break;
//				}
//			}, Capture().set_thiz(this));
//			root->add_child(e);
//		}
//		c_image->refres_res();
//	}
//
//	void close()
//	{
//		if (image)
//		{
//			image->release();
//			image = nullptr;
//		}
//
//		if (e)
//		{
//			e->get_parent()->remove_child(e);
//			e = nullptr;
//		}
//	}
//}image_view;
//
//struct TagView
//{
//	cText* c_search = nullptr;
//	Entity* e_list = nullptr;
//
//	void init(Entity* e)
//	{
//		auto e_search_bar = e->find_child("tag_search_bar");
//		c_search = e_search_bar->get_component_t<cText>();
//		e_search_bar->add_data_listener(c_search, [](Capture& c, uint h) {
//			auto thiz = c.thiz<TagView>();
//			if (h == S<"text"_h>)
//				thiz->search(thiz->c_search->get_text());
//		}, Capture().set_thiz(this));
//
//		e_list = e->find_child("tag_list");
//
//		search(L"");
//	}
//
//	void search(std::wstring_view name)
//	{
//		tags.clear();
//		e_list->remove_all_children();
//
//		auto str = std::string();
//		if (!name.empty())
//			str = sfmt("WHERE name LIKE '%%%s%%'", w2s(name).c_str());
//		auto res = db->query_fmt([](Capture& c, database::Res* res) {
//			auto thiz = c.thiz<TagView>();
//			tags.resize(res->row_count);
//			for (auto i = 0; i < tags.size(); i++)
//			{
//				res->fetch_row();
//
//				tags[i].name = s2w(res->row[1]);
//
//				auto e = Entity::create();
//				e->load(L"tag_item");
//				e->find_child("name")->get_component_t<cText>()->set_text(tags[i].name.c_str());
//
//				thiz->e_list->add_child(e);
//			}
//		}, Capture().set_thiz(this), "SELECT * FROM tk.tags %s;", str.c_str());
//		assert(res == database::NoError);
//	}
//}tag_view;
//
//struct MainView
//{
//	cText* c_search = nullptr;
//	Entity* e_list = nullptr;
//
//	void init(Entity* e)
//	{
//		c_search = e->find_child("search_bar")->get_component_t<cText>();
//
//		e->find_child("search_btn")->get_component_t<cReceiver>()->add_mouse_click_listener([](Capture& c) {
//			c.thiz<MainView>()->search();
//		}, Capture().set_thiz(this));
//
//		e_list = e->find_child("list");
//	}
//
//	void search()
//	{
//		items.clear();
//		e_list->remove_all_children();
//
//		auto sp = SUS::split(w2s(c_search->get_text()));
//		auto tag_str = std::string("(");
//		for (auto i = 0; i < sp.size(); i++)
//		{
//			tag_str += "'" + sp[i] + "'";
//			if (i < sp.size() - 1)
//				tag_str += ", ";
//		}
//		if (sp.empty())
//			tag_str += "''";
//		tag_str += ")";
//		auto res = db->query_fmt([](Capture& c, database::Res* res) {
//			auto thiz = c.thiz<MainView>();
//			items.resize(res->row_count);
//			for (auto i = 0; i < items.size(); i++)
//			{
//				res->fetch_row();
//
//				items[i].get_filename(res->row[0], res->row[1]);
//
//				uint w, h;
//				uchar* data;
//				get_thumbnail(ThumbnailSize, items[i].filename.c_str(), &w, &h, &data);
//				auto id = thumbnails_atlas.alloc_image(w, h, data);
//
//				auto e = Entity::create();
//				e->load(L"item");
//				auto element = e->get_component_t<cElement>();
//				element->set_width(ThumbnailSize);
//				element->set_height(ThumbnailSize);
//				auto padding_v = (ThumbnailSize - h) * 0.5f;
//				element->set_padding(vec4(0, padding_v, 0, padding_v));
//				auto image = e->get_component_t<cImage>();
//				image->set_res_id(thumbnails_atlas.id);
//				auto atlas_size = vec2(thumbnails_atlas.image->get_size());
//				auto uv0 = vec2((id % thumbnails_atlas.cx) * ThumbnailSize, (id / thumbnails_atlas.cx) * ThumbnailSize);
//				auto uv1 = uv0 + vec2(w, h);
//				image->set_uv(vec4(uv0 / atlas_size, uv1 / atlas_size));
//				auto thumbnail = new cThumbnail;
//				thumbnail->element = element;
//				thumbnail->w = w;
//				thumbnail->h = h;
//				thumbnail->data = data;
//				thumbnail->id = id;
//				e->add_component(thumbnail);
//				e->add_data_listener(element, [](Capture& c, uint h) {
//					auto thiz = c.thiz<cThumbnail>();
//					if (h == S<"culled"_h>)
//						thiz->toggle();
//				}, Capture().set_thiz(thumbnail));
//				auto receiver = cReceiver::create();
//				receiver->add_mouse_click_listener([](Capture& c) {
//					image_view.show(c.data<int>());
//				}, Capture().set_data(&i));
//				e->add_component(receiver);
//				thiz->e_list->add_child(e);
//			}
//		}, Capture().set_thiz(this), "SELECT * FROM tk.ssss WHERE id in (SELECT ssss_id FROM tk.ssss_tags WHERE tag_id in (SELECT id FROM tk.tags WHERE name in (%s))) ORDER BY 'order';", tag_str.c_str());
//		assert(res == database::NoError);
//	}
//}main_view;
//
//void add_tag(const char* name)
//{
//	auto res = db->query_fmt("INSERT INTO `tk`.`tags` (`id`, `name`) VALUES ('%s', '%s');", std::to_string(ch(name)).c_str(), name);
//	assert(res == database::NoError || res == database::ErrorDuplicated);
//}
//
//void collect_files(const std::filesystem::path& dir, const std::vector<const char*>& tags)
//{
//	std::vector<std::pair<std::string, std::filesystem::path>> e_list;
//	for (auto& it : std::filesystem::directory_iterator(dir))
//	{
//		if (!std::filesystem::is_directory(it.status()))
//		{
//			auto& path = it.path();
//			auto fn = std::to_string(ch(w2s(path.wstring()).c_str()));
//			e_list.emplace_back(fn, path);
//		}
//	}
//	char time_str[100];
//	{
//		time_t t;
//		time(&t);
//		auto ti = localtime(&t);
//		strftime(time_str, sizeof(time_str), "%Y-%m-%d", ti);
//	}
//	for (auto i = 0; i < e_list.size(); i++)
//	{
//		auto& item = e_list[i];
//		auto ext = item.second.extension().string();
//		auto res = db->query_fmt("INSERT INTO `tk`.`ssss` (`id`, `ext`, `time`, `order`) VALUES ('%s', '%s', '%s', '%s');", item.first.c_str(), ext.c_str(), time_str, std::to_string(i).c_str());
//		assert(res == database::NoError || res == database::ErrorDuplicated);
//		std::filesystem::copy_file(item.second, stored_path.string() + item.first + ext, std::filesystem::copy_options::skip_existing);
//		for (auto& t : tags)
//		{
//			auto tag_id = std::to_string(ch(t));
//			{
//				uint row_count;
//				auto res = db->query_fmt(&row_count, "SELECT * FROM `tk`.`ssss_tags` WHERE ssss_id='%s' AND tag_id='%s';", item.first.c_str(), tag_id.c_str());
//				assert(res == database::NoError);
//				if (row_count > 0)
//					continue;
//			}
//			auto res = db->query_fmt("INSERT INTO `tk`.`ssss_tags` (`ssss_id`, `tag_id`) VALUES ('%s', '%s')", item.first.c_str(), tag_id.c_str());
//			assert(res == database::NoError);
//		}
//	}
//}
//
//int main(int argc, char** args)
//{
//	db = database::Connection::create("tk");
//
//	g_app.create();
//
//	auto w = new GraphicsWindow(&g_app, L"Media Browser", uvec2(1280, 720), WindowFrame | WindowResizable, true, true);
//	root = w->root;
//	canvas = w->canvas;
//
//	auto screen_size = get_screen_size();
//	thumbnails_atlas.create(ceil((float)screen_size.x / (float)ThumbnailSize), ceil((float)screen_size.y / (float)ThumbnailSize), ThumbnailSize);
//
//	auto e = Entity::create();
//	e->load(L"main");
//	root->add_child(e);
//
//	main_view.init(e);
//	image_view.init(e);
//	tag_view.init(e);
//
//	looper().add_event([](Capture& c) {
//		auto dispatcher = g_app.main_window->s_dispatcher;
//		printf("------------------------\n");
//		auto hovering = dispatcher->get_hovering();
//		auto focusing = dispatcher->get_focusing();
//		auto active = dispatcher->get_active();
//		printf("hovering: %s, focusing: %s, active: %s\n", 
//			hovering ? hovering->entity->get_name() : "",
//			focusing ? focusing->entity->get_name() : "",
//			active ? active->entity->get_name() : "");
//		printf("fps: %d\n", looper().get_fps());
//		c._current = nullptr;
//	}, Capture(), 1.f);
//
//	g_app.run();
//
//	return 0;
//}
