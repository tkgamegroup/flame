struct cHierarchyItem : Component
{
	int drop_pos = -1;

	void on_event(EntityEvent e, void* t)
	{
		switch (e)
		{
		case EntityComponentAdded:
			if (t == this)
			{
				receiver->drag_hash = FLAME_CHASH("cHierarchyItem");
				receiver->set_acceptable_drops(1, &FLAME_CHASH("cHierarchyItem"));
				receiver->drag_and_drop_listeners.add([](Capture& c, DragAndDrop action, cReceiver* er, const ivec2& pos) {
					auto thiz = c.thiz<cHierarchyItem>();
					auto element = thiz->element;

					if (action == BeingOvering)
					{
						auto h = element->global_size.y * 0.3f;
						if (pos.y < element->global_pos.y + h)
							thiz->drop_pos = 0;
						else if (pos.y > element->global_pos.y + element->global_size.y - h)
							thiz->drop_pos = 2;
						else
							thiz->drop_pos = 1;
					}
					else if (action == BeenDropped)
					{
						if (!(thiz->entity->parent->get_component(cTree) && thiz->drop_pos != 1))
						{
							struct Capturing
							{
								Entity* dst;
								Entity* src;
								int i;
							}capture;
							capture.dst = thiz->e;
							capture.src = er->entity->get_component(cHierarchyItem)->e;
							capture.i = thiz->drop_pos;

							auto ok = true;
							auto p = capture.src->parent;
							while (p)
							{
								if (p == capture.dst)
								{
									ok = false;
									break;
								}
							}

							if (ok)
							{
								looper().add_event([](Capture& c) {
									auto& capture = c.data<Capturing>();

									capture.src->parent->remove_child(capture.src, false);

									if (capture.i == 1)
										capture.dst->add_child(capture.src);
									else
									{
										auto idx = capture.dst->index_;
										if (capture.i == 2)
											idx++;
										capture.dst->parent->add_child(capture.src, idx);
									}

									scene_editor.hierarchy->refresh();
								}, Capture().set_data(&capture));
							}
						}

					}

					return true;
				}, Capture().set_thiz(this));
			}
			break;
		}
	}

	void draw(graphics::Canvas* canvas)
	{
		if (receiver->dispatcher->drag_overing != receiver)
			drop_pos = -1;
		if (!element->clipped && drop_pos >= 0)
		{
			std::vector<vec2> points;
			switch (drop_pos)
			{
			case 0:
				points.push_back(element->global_pos);
				path_move(points, element->global_size.x, 0.f);
				break;
			case 1:
				path_rect(points, element->global_pos, element->global_size);
				points.push_back(points[0]);
				break;
			case 2:
				points.push_back(element->global_pos + vec2(0.f, element->global_size.y));
				path_move(points, element->global_size.x, 0.f);
				break;
			}
			canvas->stroke(points.size(), points.data(), cvec4(120, 150, 255, 255), 3.f);
		}
	}
};
