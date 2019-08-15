#include <flame/foundation/window.h>
#include <flame/graphics/canvas.h>
#include <flame/graphics/font.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/edit.h>

namespace flame
{
	struct cEditPrivate : cEdit
	{
		void* key_listener;

		cEditPrivate(Entity* e) :
			cEdit(e)
		{
			element = (cElement*)(e->find_component(cH("Element")));
			assert(element);
			text = (cText*)(e->find_component(cH("Text")));
			assert(text);
			event_receiver = (cEventReceiver*)(e->find_component(cH("EventReceiver")));
			assert(event_receiver);

			cursor = 0;

			key_listener = event_receiver->add_key_listener([](void* c, KeyState action, uint value) {
				auto thiz = *(cEditPrivate**)c;
				auto& str = ((cTextPrivate*)thiz->text)->text;

				if (action == KeyStateNull)
				{
					switch (value)
					{
					case L'\b':
						if (thiz->cursor > 0)
						{
							thiz->cursor--;
							str.erase(str.begin() + thiz->cursor);
						}
						break;
					case 22:
					{
						auto copied = get_clipboard();
						thiz->cursor = 0;
						str = *copied.p;
						delete_mail(copied);
					}
						break;
					case 27:
						break;
					default:
						str.insert(str.begin() + thiz->cursor, value);
						thiz->cursor++;
					}
				}
				else if (action == KeyStateDown)
				{
					switch (value)
					{
					case Key_Left:
						if (thiz->cursor > 0)
							thiz->cursor--;
						break;
					case Key_Right:
						if (thiz->cursor < str.size())
							thiz->cursor++;
						break;
					case Key_Home:
						thiz->cursor = 0;
						break;
					case Key_End:
						thiz->cursor = str.size();
						break;
					case Key_Del:
						if (thiz->cursor < str.size())
							str.erase(str.begin() + thiz->cursor);
						break;
					}
				}
			}, new_mail_p(this));
		}

		~cEditPrivate()
		{
			event_receiver->remove_key_listener(key_listener);
		}

		void update()
		{
			if (event_receiver->focusing && (int(app_total_time()) % 2 == 0))
			{
				auto text_scale = text->sdf_scale * element->global_scale;
				element->canvas->add_text(text->font_atlas, Vec2f(element->global_x, element->global_y) +
					Vec2f(element->inner_padding[0], element->inner_padding[1]) * element->global_scale +
					Vec2f(text->font_atlas->get_text_width(std::wstring_view(text->text().c_str(), cursor)) * text_scale, 0.f),
					alpha_mul(text->color, element->alpha), L"|", text_scale);
			}
		}
	};

	cEdit::cEdit(Entity* e) :
		Component("Edit", e)
	{
	}

	cEdit::~cEdit()
	{
	}

	void cEdit::update()
	{
		((cEditPrivate*)this)->update();
	}

	cEdit* cEdit::create(Entity* e)
	{
		return new cEditPrivate(e);
	}
}
