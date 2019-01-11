// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/*
	The Code Solver

	To - 
		Generate code that cannot be easily done by macro, and more clearer than macro
	How - 
		Once you write FLAME_UNRESOLVED(), the content in round brackets will be processed. 
	Then the result will generate, and wrap in FLAME_RESOLVED(), and then place right after
	the 'FLAME_UNRESOLVED' (they link by a semicolon)
	For example:

		FLAME_UNRESOLVED(
			struct House
				Orange Cat
				Black Cat
		);
		FLAME_RESOLVED(
			struct House
			{
				Cat orange_cat;
				Cat black_cat;

				enum { CAT_COUNT = 2 };
			};
		)

		*NOTE: no other characters but a semicolon can appear between 
		FLAME_UNRESOLVED and FLAME_RESOLVED (blank characters are OK)
*/

#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
	if (argc != 2)
		return 0;

	auto src = get_file_string(s2w(args[1]));
	enum
	{
		InNothing,

	};
	while (true)
	{

	}

	return 0;
}
