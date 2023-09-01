#include "library.h"
#include "../blueprint_private.h"

#include "standard/node_vec2.h"
#include "standard/node_vec3.h"
#include "standard/node_vec4.h"
#include "standard/node_decompose.h"
#include "standard/node_add.h"
#include "standard/node_subtract.h"
#include "standard/node_multiply.h"
#include "standard/node_divide.h"
#include "standard/node_integer_divide.h"
#include "standard/node_floor.h"
#include "standard/node_less.h"
#include "standard/node_string.h"
#include "standard/node_flow_control.h"

namespace flame
{
	void init_library()
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
		add_node_template_integer_divide(standard_library);
		add_node_template_floor(standard_library);
		add_node_template_less(standard_library);
		add_node_templates_string(standard_library);
		add_node_templates_flow_control(standard_library);
	}
}
