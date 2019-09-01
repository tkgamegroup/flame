namespace flame
{
	namespace ui
	{
		struct Layout
		{
			Layout* parent;
			int idx;

			LayoutType type;

			float width;
			float height;
			float size_radio;
			ImGui::Splitter splitter;

			std::unique_ptr<Layout> children[2];
			std::list<Window*> windows[2];
			Window* curr_tab[2];
		};

		float get_layout_padding(bool horizontal)
		{
			if (horizontal)
				return ImGui::GetStyle().WindowPadding.x * 2.f - ImGui::GetStyle().ItemSpacing.x;
			else
				return ImGui::GetStyle().WindowPadding.y * 2.f - ImGui::GetStyle().ItemSpacing.y;
		}

		const char *get_dock_dir_name(DockDirection dir)
		{
			switch (dir)
			{
				case DockCenter:
					return "center";
				case DockLeft:
					return "left";
				case DockRight:
					return "right";
				case DockTop:
					return "top";
				case DockBottom:
					return "bottom";
			}
			return "null";
		}

		static Window *last_dragging_window;
		static Window *dragging_window;
		static Layout *dock_target_layout;
		static int dock_target_idx;
		static DockDirection dock_dir;

		Layout::Layout() :
			parent(nullptr),
			idx(-1),
			type(LayoutCenter),
			size_radio(0.5f),
			splitter(true)
		{
			splitter.size[0] = splitter.size[1] = -1.f;
			curr_tab[0] = curr_tab[1] = nullptr;
		}

		bool Layout::is_empty(int idx) const
		{
			if (children[idx])
				return false;
			if (windows[idx].size() > 0)
				return false;
			return true;
		}

		bool Layout::is_empty() const
		{
			for (int i = 0; i < 2; i++)
			{
				if (!is_empty(i))
					return false;
			}
			return true;
		}

		void Layout::set_size()
		{
			if (parent)
			{
				if (parent->type == LayoutHorizontal)
				{
					width = parent->splitter.size[idx];
					height = parent->height - get_layout_padding(false);
				}
				else
				{
					width = parent->width - get_layout_padding(true);
					height = parent->splitter.size[idx];
				}
			}
			float s;
			switch (type)
			{
				case LayoutHorizontal:
					s = width - get_layout_padding(true);
					break;
				case LayoutVertical:
					s = height - get_layout_padding(false);
					break;
				default:
					return;
			}
			splitter.size[0] = s * size_radio;
			splitter.size[1] = s * (1.f - size_radio);
		}

		void Layout::set_layout(int idx, Layout *l)
		{
			l->parent = this;
			l->idx = idx;
			children[idx] = std::unique_ptr<Layout>(l);
		}

		void Layout::set_layout(int idx, std::unique_ptr<Layout> &&l)
		{
			if (l)
			{
				l->parent = this;
				l->idx = idx;
			}
			children[idx] = std::move(l);
		}

		void Layout::add_window(int idx, Window *w)
		{
			if (w)
			{
				w->layout = this;
				w->idx = idx;
			}
			windows[idx].push_back(w);
		}

		void Layout::remove_window(int idx, Window *w)
		{
			for (auto it = windows[idx].begin(); it != windows[idx].end(); it++)
			{
				if (*it == w)
				{
					windows[idx].erase(it);
					return;
				}
			}
		}

		void Layout::clear_window(int idx)
		{
			windows[idx].clear();
		}

		static void _draw_drag_overlay(ImRect rect, Layout *layout, int idx, DockDirection dir)
		{
			if (rect.Contains(ImVec2(mouse.x, mouse.y)))
			{
				dock_target_layout = layout;
				dock_target_idx = idx;
				auto draw_list = ImGui::GetOverlayDrawList();
				auto center = rect.GetCenter();
				ImColor col0(0.7f, 0.1f, 1.f, 0.5f);
				ImColor col1(0.3f, 0.2f, 0.5f, 0.5f);
				if (dir & DockCenter)
				{
					auto _rect = ImRect(center + ImVec2(-32, -32), center + ImVec2(32, 32));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(mouse.x, mouse.y)))
					{
						draw_list->AddRectFilled(rect.Min, rect.Max, col1);
						dock_dir = DockCenter;
					}
				}
				if (dir & DockLeft)
				{
					auto _rect = ImRect(center + ImVec2(-96, -32), center + ImVec2(-64, 32));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(mouse.x, mouse.y)))
					{
						draw_list->AddRectFilled(rect.Min, rect.Max - ImVec2(rect.GetWidth() / 2.f, 0), col1);
						dock_dir = DockLeft;
					}
				}
				if (dir & DockRight)
				{
					auto _rect = ImRect(center + ImVec2(64, -32), center + ImVec2(96, 32));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(mouse.x, mouse.y)))
					{
						draw_list->AddRectFilled(rect.Min + ImVec2(rect.GetWidth() / 2.f, 0), rect.Max, col1);
						dock_dir = DockRight;
					}
				}
				if (dir & DockTop)
				{
					auto _rect = ImRect(center + ImVec2(-32, -96), center + ImVec2(32, -64));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(mouse.x, mouse.y)))
					{
						draw_list->AddRectFilled(rect.Min, rect.Max - ImVec2(0, rect.GetHeight() / 2.f), col1);
						dock_dir = DockTop;
					}
				}
				if (dir & DockBottom)
				{
					auto _rect = ImRect(center + ImVec2(-32, 64), center + ImVec2(32, 96));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(mouse.x, mouse.y)))
					{
						draw_list->AddRectFilled(rect.Min + ImVec2(0, rect.GetHeight() / 2.f), rect.Max, col1);
						dock_dir = DockBottom;
					}
				}
			}
		}

		void Layout::show_window(int idx)
		{
			ImGui::BeginTabBar("tabbar");
			for (auto &w : windows[idx])
			{
				auto open = true;
				auto curr = ImGui::TabItem(w->title.c_str(), &open);
				if (ImGui::IsItemActive())
				{
					auto mouseY = ImGui::GetMousePos().y;
					if (mouseY < ImGui::GetItemRectMin().y || mouseY > ImGui::GetItemRectMax().y)
						w->_tag = WindowTagUndock;
				}
				if (curr)
				{
					curr_tab[idx] = w;
					w->show();
				}
				if (!open)
					w->_tag = WindowTagClose;
			}
			ImGui::EndTabBar();

			if (dragging_window)
			{
				auto pos = ImGui::GetWindowPos();
				ImRect window_rect(pos, pos + ImGui::GetWindowSize());
				_draw_drag_overlay(ImRect(pos, pos + ImGui::GetWindowSize()), this, idx, DockAll);
			}
		}

		void Layout::show()
		{
			switch (type)
			{
				case LayoutCenter:
					show_window(0);
					break;
				case LayoutHorizontal: case LayoutVertical:
				{
					splitter.do_split();
					splitter.set_size_greedily();
					size_radio = splitter.size[0] / (splitter.size[0] + splitter.size[1]);
					for (int i = 0; i < 2; i++)
					{
						ImGui::PushID(i);
						ImGui::BeginChild("##part", type == LayoutHorizontal ? ImVec2(splitter.size[i], 0) : ImVec2(0, splitter.size[i]), false);
						if (children[i])
							children[i]->show();
						else
							show_window(i);
						ImGui::EndChild();
						ImGui::PopID();
						if (i == 0 && type == LayoutHorizontal)
							ImGui::SameLine();
					}
					break;
				}
			}
		}

		Layout *main_layout;

		static bool _cleanup_layout(Layout *l)
		{
			auto dirty = false;
			if (l->is_empty())
			{
				l->type = LayoutCenter;
				return false;
			}
			for (int i = 0; i < 2; i++)
			{
				if (l->is_empty(i))
				{
					auto j = 1 - i;
					if (l->children[j] == nullptr)
					{
						if (j == 1)
						{
							for (auto w : l->windows[1])
								l->add_window(0, w);
						}
						l->clear_window(1);
						l->type = LayoutCenter;
						return false;
					}
					else
					{
						auto c = l->children[j].get();
						l->type = c->type;
						l->width = c->width;
						l->height = c->height;
						for (int k = 0; k < 2; k++)
						{
							l->splitter.size[k] = c->splitter.size[k];
							l->windows[k].clear();
							for (auto w : c->windows[k])
								l->add_window(k, w);
						}
						l->set_layout(i, std::move(c->children[i]));
						l->set_layout(j, std::move(c->children[j]));
					}
					break;
				}
			}
			for (int i = 0; i < 2; i++)
			{
				if (l->children[i])
				{
					auto c = l->children[i].get();
					dirty |= _cleanup_layout(c);
					if (l->children[i]->is_empty())
					{
						l->children[i].reset();
						dirty = true;
					}
					else
					{
						for (int j = 0; j < 2; j++)
						{
							if (!c->children[j] && c->windows[j].size() == 0)
							{
								l->clear_window(i);
								auto k = 1 - j;
								for (auto w : c->windows[k])
									l->add_window(i, w);
								l->children[i].reset();
								dirty = true;
								break;
							}
						}
					}
				}
			}
			return dirty;
		}

		void cleanup_layout()
		{
			auto continue_ = true;
			while (continue_)
				continue_ = _cleanup_layout(main_layout);
		}

		static void _resize_layout(Layout *l)
		{
			l->set_size();
			for (int i = 0; i < 2; i++)
			{
				if (l->children[i])
					_resize_layout(l->children[i].get());
			}
		}

		void resize_layout()
		{
			main_layout->width = surface->cx;
			main_layout->height = surface->cy - ImGui::menubar_height - ImGui::toolbar_height - ImGui::statusbar_height;
			_resize_layout(main_layout);
		}

		void dock(Window *src, Window *dst, DockDirection dir)
		{
			if (dst == nullptr)
			{
				if (main_layout->type != LayoutCenter)
					return;
				main_layout->add_window(0, src);
				return;
			}

			auto ori_layout = dst->layout;
			auto ori_type = ori_layout->type;
			auto ori_idx = dst->idx;
			assert(ori_layout);

			if (dir == DockCenter)
				ori_layout->add_window(ori_idx, src);
			else
			{
				auto dir_id = (dir == DockLeft || dir == DockTop) ? 0 : 1;
				auto l = ori_layout;
				if (ori_type != LayoutCenter)
					l = new Layout;
				l->type = (dir == DockLeft || dir == DockRight) ? LayoutHorizontal : LayoutVertical;
				l->splitter.set_vertically(l->type == LayoutHorizontal);
				if (ori_type != LayoutCenter || ori_idx != 1 - dir_id)
				{
					for (auto w : ori_layout->windows[ori_idx])
						l->add_window(1 - dir_id, w);
					ori_layout->clear_window(ori_idx);
				}
				l->add_window(dir_id, src);
				if (ori_type != LayoutCenter)
					ori_layout->set_layout(ori_idx, l);
				l->set_size();
			}
		}

		void undock(Window *w)
		{
			if (!w->layout)
				return;

			w->layout->remove_window(w->idx, w);
			w->layout = nullptr;
			cleanup_layout();
			resize_layout();
		}

		void set_dragging_window(Window *w)
		{
			if (dragging_window == w)
				return;
			dragging_window = w;
			dock_target_layout = nullptr;
			dock_target_idx = -1;
			dock_dir = (DockDirection)-1;
		}

		static void _load_layout(XMLNode *n, Layout *layout)
		{
			for (auto &a : n->attributes)
			{
				if (a->name == "mode")
				{
					if (a->value == "horizontal")
						layout->type = LayoutHorizontal;
					else if (a->value == "vertical")
						layout->type = LayoutVertical;
					else if (a->value == "center")
						layout->type = LayoutCenter;
					else
						assert(0); // vaild name required
				}
				else if (a->name == "size_radio")
					layout->size_radio = to_float(a->value);
			}

			layout->splitter.set_vertically(layout->type == LayoutHorizontal);
			layout->set_size();

			for (int i = 0; i < 2; i++)
			{
				auto c = n->children[i].get();
				if (c->name == "node")
				{
					auto type_name = c->find_attribute("type")->value;
					if (type_name == "layout")
					{
						auto l = new Layout;
						layout->set_layout(i, l);
						_load_layout(c, l);
					}
					else if (type_name == "windows")
					{
						for (auto &cc : c->children)
						{
							if (cc->name == "window")
							{
								auto window_name = cc->find_attribute("name")->value;
								for (auto &w : get_windows())
								{
									if (w->title == window_name)
									{
										layout->add_window(i, w.get());
										break;
									}
								}
							}
						}
					}
					else
						assert(0); // vaild name required
				}
				else
					assert(0); // vaild name required
			}
		}

		void load_layout()
		{
			auto xml = load_xml("layout", "ui_layout.xml");
			if (xml)
			{
				_load_layout(xml, main_layout);
				release_xml(xml);
			}
			cleanup_layout();
		}

		static void _save_layout(XMLNode *n, Layout *layout)
		{
			std::string mode_name;
			switch (layout->type)
			{
				case LayoutHorizontal:
					mode_name = "horizontal";
					break;
				case LayoutVertical:
					mode_name = "vertical";
					break;
				case LayoutCenter:
					mode_name = "center";
					break;
			}
			n->attributes.emplace_back(new XMLAttribute("mode", mode_name));

			n->attributes.emplace_back(new XMLAttribute("size_radio", to_str(layout->size_radio)));

			for (int i = 0; i < 2; i++)
			{
				auto c = new XMLNode("node");
				n->children.emplace_back(c);
				if (layout->children[i])
				{
					c->attributes.emplace_back(new XMLAttribute("type", "layout"));
					_save_layout(c, layout->children[i].get());
				}
				else
				{
					c->attributes.emplace_back(new XMLAttribute("type", "windows"));
					for (auto w : layout->windows[i])
					{
						auto window_node = new XMLNode("window");
						window_node->attributes.emplace_back(new XMLAttribute("name", w->title));
						c->children.emplace_back(window_node);
					}
				}
			}
		}

		void save_layout()
		{
			XMLDoc xml("layout");
			_save_layout(&xml, main_layout);
			save_xml(&xml, "ui_layout.xml");
		}

		void reset_dragging()
		{
			last_dragging_window = dragging_window;
			dragging_window = nullptr;
		}

		void show_layout()
		{
			if (!main_layout->is_empty(0))
			{
				ImGui::SetNextWindowPos(ImVec2(0.f, ImGui::menubar_height + ImGui::toolbar_height));
				ImGui::SetNextWindowSize(ImVec2(main_layout->width, main_layout->height));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
				ImGui::Begin("##dock", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
				main_layout->show();
				ImGui::End();
				ImGui::PopStyleVar();
			}
			else
			{
				if (dragging_window)
					_draw_drag_overlay(ImRect(0.f, ImGui::menubar_height, main_layout->width, ImGui::menubar_height + ImGui::toolbar_height + main_layout->height), main_layout, -1, DockCenter);
			}

			if (last_dragging_window != dragging_window)
			{
				if (last_dragging_window && dock_target_layout && dock_dir != -1)
					dock(last_dragging_window, dock_target_layout->curr_tab[dock_target_idx], dock_dir);
			}
		}
	}
}
