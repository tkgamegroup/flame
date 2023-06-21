#pragma once

#include "scroll_view.h"

namespace flame
{
	struct cScrollViewPrivate : cScrollView
	{
		void* ev_update = nullptr;

		cScrollViewPrivate();
		~cScrollViewPrivate();
		void on_active() override;

		void mark_update();
		void update_scroll_view();
		void update_content_container();
		void update_content_viewport();
		void update_vertical_scroller();
		void update_vertical_scroller_slider();
		void set_content_container(const GUID& guid) override;
		void set_content_viewport(const GUID& guid) override;
		void set_vertical_scroller(const GUID& guid) override;
		void set_vertical_scroller_slider(const GUID& guid) override;
	};
}
