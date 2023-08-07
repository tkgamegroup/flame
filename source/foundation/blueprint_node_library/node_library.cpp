#include "node_library.h"
#include "../blueprint_private.h"

#include "standard/node_vec2.h"
#include "standard/node_vec3.h"
#include "standard/node_vec4.h"
#include "standard/node_decompose.h"
#include "standard/node_add.h"
#include "standard/node_subtract.h"
#include "standard/node_multiply.h"
#include "standard/node_divide.h"

namespace flame
{
	void init_node_library()
	{
		auto standard_library = BlueprintNodeLibrary::get(L"standard");

		add_node_template_vec2(standard_library);
		add_node_template_vec3(standard_library);
		add_node_template_vec4(standard_library);
		add_node_template_decompose(standard_library);
		add_node_template_add(standard_library);
		add_node_template_subtract(standard_library);
		add_node_template_multiply(standard_library);
		add_node_template_divide(standard_library);
	}
}
