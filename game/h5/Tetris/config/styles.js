'use strict';

var TetrisStyles = [
	{
		color: "#00ffff",
		transformers:
		[
			[
				0, 0, 0, 0,
				1, 1, 1, 1,
				0, 0, 0, 0,
				0, 0, 0, 0
			],[
				0, 1, 0, 0,
				0, 1, 0, 0,
				0, 1, 0, 0,
				0, 1, 0, 0
			],[
				0, 0, 0, 0,
				0, 0, 0, 0,
				1, 1, 1, 1,
				0, 0, 0, 0
			],[
				0, 0, 1, 0,
				0, 0, 1, 0,
				0, 0, 1, 0,
				0, 0, 1, 0
			]
		]
	},
	{
		color: "#0000ff",
		transformers:
		[
			[
				1, 0, 0, 0,
				1, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			],[
				0, 1, 0, 0,
				0, 1, 0, 0,
				1, 1, 0, 0,
				0, 0, 0, 0
			],[
				0, 0, 0, 0,
				1, 1, 1, 0,
				0, 0, 1, 0,
				0, 0, 0, 0
			],[
				0, 1, 1, 0,
				0, 1, 0, 0,
				0, 1, 0, 0,
				0, 0, 0, 0
			]
		]
	},
	{
		color: "#0000ff",
		transformers:
		[
			[
				0, 0, 1, 0,
				1, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			],[
				1, 1, 0, 0,
				0, 1, 0, 0,
				0, 1, 0, 0,
				0, 0, 0, 0
			],[
				0, 0, 0, 0,
				1, 1, 1, 0,
				1, 0, 0, 0,
				0, 0, 0, 0
			],[
				0, 1, 0, 0,
				0, 1, 0, 0,
				0, 1, 1, 0,
				0, 0, 0, 0
			]
		]
	},
	{
		color: "#ff8000",
		transformers:
		[
			[
				0, 1, 1, 0,
				0, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			],[
				0, 1, 1, 0,
				0, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			],[
				0, 1, 1, 0,
				0, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			],[
				0, 1, 1, 0,
				0, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			]
		]
	},
	{
		color: "#ffff00",
		transformers:
		[
			[
				0, 1, 1, 0,
				1, 1, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			],[
				1, 0, 0, 0,
				1, 1, 0, 0,
				0, 1, 0, 0,
				0, 0, 0, 0
			],[
				0, 0, 0, 0,
				0, 1, 1, 0,
				1, 1, 0, 0,
				0, 0, 0, 0
			],[
				0, 1, 0, 0,
				0, 1, 1, 0,
				0, 0, 1, 0,
				0, 0, 0, 0
			]
		]
	},
	{
		color: "#8000ff",
		transformers:
		[
			[
				0, 1, 0, 0,
				1, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			],[
				0, 1, 0, 0,
				1, 1, 0, 0,
				0, 1, 0, 0,
				0, 0, 0, 0
			],[
				0, 0, 0, 0,
				1, 1, 1, 0,
				0, 1, 0, 0,
				0, 0, 0, 0
			],[
				0, 1, 0, 0,
				0, 1, 1, 0,
				0, 1, 0, 0,
				0, 0, 0, 0
			]
		]
	},
	{
		color: "#ff0000",
		transformers:
		[
			[
				1, 1, 0, 0,
				0, 1, 1, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			],[
				0, 1, 0, 0,
				1, 1, 0, 0,
				1, 0, 0, 0,
				0, 0, 0, 0
			],[
				0, 0, 0, 0,
				1, 1, 0, 0,
				0, 1, 1, 0,
				0, 0, 0, 0
			],[
				0, 0, 1, 0,
				0, 1, 1, 0,
				0, 1, 0, 0,
				0, 0, 0, 0
			]
		]
	}
];
