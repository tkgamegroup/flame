#pragma once

#include <vector>
#include <memory>

#include <flame/engine/graphics/texture.h>
#include <flame/engine/ui/fileselector.h>

struct ResourceExplorer : flame::ui::FileSelector
{
	ResourceExplorer();
	virtual ~ResourceExplorer() override;
	virtual void on_right_area_show() override;
};

extern ResourceExplorer *resourceExplorer;