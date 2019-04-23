'use strict';

window.seed = 0;
function Random()
{
	window.seed = (window.seed * 9301 + 49297) % 233280; 
	return window.seed / 233280.0;
}

class Tetris
{
    constructor(x, y, style, down_frame)
    {
		this.x = x;
		this.y = y;
		this.y_max_adv = 0;

		this.h_move = 0;
		this.v_move = 0;

		this.style = style;
		this.style_idx = 0;
		this.SetData();

		this.down_frame_max = down_frame;
		this.down_frame = this.down_frame_max;

		this.die = false;
	}

	SetData()
	{
		this.data = this.style.transformers[this.style_idx];
	}

	SetToGrids(grids, y_off, v_op, d_op)
	{
		var color = this.style.color;
		for (var y = 0; y < 4; y++)
		{
			for (var x = 0; x < 4; x++)
			{
				if (this.data[y * 4 + x] == 1)
				{
					var grid = grids[(y + this.y + y_off) * Scene.cx + x + this.x];
					if (v_op == 0)
						grid.v = 0;
					else if (v_op == 1)
						grid.v = 1;
					if (d_op == 0)
					{
						grid.s.graphics.clear();
						grid.c = "";
					}
					else if (d_op == 1)
					{
						grid.s.graphics.drawRect(0, 0, 20, 20, color);
						grid.c = color;
					}
					else if (d_op == 2)
						grid.s.graphics.drawRect(0, 0, 20, 20, null, color);
				}
			}
		}
	}

	Check(grids, y_off)
	{
		for (var y = 0; y < 4; y++)
		{
			for (var x = 0; x < 4; x++)
			{
				var px = x + this.x;
				var py = y + this.y + y_off;
				if (this.data[y * 4 + x] == 1 && (px < 0 || py < 0 || px >= Scene.cx || py >= Scene.cy ||
				(grids[py * Scene.cx + px].v == 1)))
					return false;
			}
		}
		return true;
	}
};

class App
{
    constructor()
    {
        try
        {
			Laya.init( 800, 600 );
			
			var bg = new laya.display.Sprite();
			bg.graphics.drawRect(0, 0, Scene.cx * (Scene.w + 5) + 40, 20, "#ffffff");
			bg.graphics.drawRect(0, 20, 20, Scene.cy * (Scene.h + 5), "#ffffff");
			bg.graphics.drawRect(Scene.cx * (Scene.w + 5) + 20, 20, 20, Scene.cy * (Scene.h + 5) + 20, "#ffffff");
			bg.graphics.drawRect(0, Scene.cy * (Scene.h + 5) + 20, Scene.cx * (Scene.w + 5) + 40, 20, "#ffffff");
			Laya.stage.addChild(bg);
	
			// init grids
			this.grids = [];
			for (var y = 0 ; y < Scene.cy; y++)
			{
				for (var x = 0; x < Scene.cx; x++)
				{
					var s = new laya.display.Sprite();
					s.size(Scene.w, Scene.h);
					s.x = x * (Scene.w + 5) + 20;
					s.y = y * (Scene.h + 5) + 20;
					Laya.stage.addChild(s);
	
					var item = {};
					item.v = 0;
					item.s = s;
					item.c = "";
					this.grids.push(item);
				}
			}
			//

			// init level
			this.score = 0;
			this.curr_level = 0;

			this.text_score = new laya.display.Text();
			this.text_score.x = 0;
			this.text_score.y = 0;
			this.text_score.color = "#ffffff";
			Laya.stage.addChild(this.text_score);
			this.text_score.text = this.score;

			this.text_level = new laya.display.Text();
			this.text_level.x = 200;
			this.text_level.y = 0;
			this.text_level.color = "#ffffff";
			Laya.stage.addChild(this.text_level);
			this.text_level.text = this.curr_level + 1;
			//

			// init sync
			this.frame = 0;
			this.h_move = 0;
			this.v_move = 0;
			//

			var thiz = this;

			var sock_s = new WebSocket(AppConfig.url);
			window.sock_s = sock_s;
			sock_s.onopen = function(){
				console.log("connected to server");
			};
			sock_s.onmessage = function(res){
				var src = eval('(' + res.data + ')');
				if (src.action == "start")
				{
					window.seed = parseInt(src.seed);
					thiz.GameStart();
					app.FrameSync();
				}
				else if (src.action == "frame")
				{
					app.frame++;
					if (thiz.curr_tetrises[0])
					{
						thiz.curr_tetrises[0].h_move = parseInt(src.h_move1);
						thiz.curr_tetrises[0].v_move = parseInt(src.v_move1);
					}
					if (thiz.curr_tetrises[1])
					{
						thiz.curr_tetrises[1].h_move = parseInt(src.h_move2);
						thiz.curr_tetrises[1].v_move = parseInt(src.v_move2);
					}
					Laya.timer.once(16, app, app.Update);
				}
			};
			sock_s.onclose = function(){
				setTimeout(function(){
					var s = new WebSocket(AppConfig.url);
					s.onmessage = window.sock_s.onmessage;
					s.onclose = window.sock_s.onclose;
					window.sock_s = s;
				}, 2000);
			};
        }
		catch( e )
		{
			var szError = "error : " + e.name + ": " + e.message;
		}
	}
    
    GameStart()
    {
		this.frame = 0;

		// init curr tetrises
		this.curr_tetrises = [];
		this.curr_tetrises[0] = null;
		this.curr_tetrises[1] = null;
		this.next_tetris_styles = [];
		this.next_tetris_style_count = 3;
		for (var i = 0; i < this.next_tetris_style_count; i++)
		{
			var grids = [];
			for (var y = 0 ; y < 4; y++)
			{
				for (var x = 0; x < 4; x++)
				{
					var s = new laya.display.Sprite();
					s.size(Scene.w, Scene.h);
					s.x = (x + Scene.cx) * (Scene.w + 5) + 60;
					s.y = (y + i * 4) * (Scene.h + 5) + i * 15 + 20;
					s.graphics.drawRect(0, 0, 20, 20, "#ff0000");
					Laya.stage.addChild(s);
					grids.push(s);
				}
			}

			this.next_tetris_styles.push({grids: grids, style: this.NewTetrisStyle()});
		}
		//

		this.ShowNextTetrisStyles();

		this.mouse_x = 0;
		this.mouse_y = 0;
		this.mouse_pressing = false;
		this.mouse_pos_shifted = false;
		this.mouse_down_in_short_time = false;

        Laya.stage.on(laya.events.Event.KEY_DOWN, this, this.OnKeyDown);
        Laya.stage.on(laya.events.Event.MOUSE_DOWN, this, this.OnMouseDown);
        Laya.stage.on(laya.events.Event.MOUSE_UP, this, this.OnMouseUp);
        Laya.stage.on(laya.events.Event.MOUSE_MOVE, this, this.OnMouseMove);
	}

	ShowNextTetrisStyles()
	{
		for (var i = 0; i < this.next_tetris_style_count; i++)
		{
			var grids = this.next_tetris_styles[i].grids;
			var transformer = this.next_tetris_styles[i].style.transformers[0];
			var color = this.next_tetris_styles[i].style.color;

			for (var y = 0 ; y < 4; y++)
			{
				for (var x = 0; x < 4; x++)
				{
					var idx = y * 4 + x;
					var s = grids[idx];
					if (transformer[idx] == 1)
						s.graphics.drawRect(0, 0, 20, 20, color);
					else
						s.graphics.clear();
				}
			}
		}
	}

	NewTetrisStyle()
	{
		return TetrisStyles[Math.floor(Random() * TetrisStyles.length)];
	}

	CheckForTwoTetrises(offs)
	{
		for (var i = 0; i < 2; i++)
		{
			if (!this.curr_tetrises[i])
				return true;
		}

		var a = this.curr_tetrises[0];
		var b = this.curr_tetrises[1];
		var ax = a.x;
		var ay = a.y + offs[0];
		var bx = b.x;
		var by = b.y + offs[1];

		for (var y = 0; y < 4; y++)
		{
			for (var x = 0; x < 4; x++)
			{
				var px = x + ax;
				var py = y + ay;

				if (px >= bx && px < bx + 4 &&
					py >= by && py < by + 4)
				{
					px -= bx;
					py -= by;
					if (a.data[y * 4 + x] == 1 &&
						b.data[py * 4 + px] == 1)
						return false;
				}
			}
		}

		return true;
	}

	FrameSync()
	{
		var data = {};
		data.frame = this.frame;
		data.h_move = this.h_move;
		data.v_move = this.v_move;
		this.h_move = this.v_move = 0;
		var json = JSON.stringify(data);
		window.sock_s.send(json);
	}
	
	Update()
	{
		for (var i = 0; i < 2; i++)
		{
			var curr_t = this.curr_tetrises[i];
			if (curr_t)
			{
				curr_t.SetToGrids(this.grids, curr_t.y_max_adv, 2, 0);
				curr_t.SetToGrids(this.grids, 0, 0, 0);
			}
		}

		for (var i = 0; i < 2; i++)
		{
			if (!this.curr_tetrises[i])
			{
				if (i == 0)
					this.mouse_pressing = false;

				var lines = 0;
				for (var y = Scene.cy - 1; y >= 0; y--)
				{
					var clear = true;
					for (var x = 0; x < Scene.cx; x++)
					{
						if (this.grids[y * Scene.cx + x].v == 0)
							clear = false;
					}
					if (clear)
					{
						for (var yy = y; yy > 0; yy--)
						{
							for (var x = 0; x < Scene.cx; x++)
							{
								var src = this.grids[(yy - 1) * Scene.cx + x];
								var dst = this.grids[yy * Scene.cx + x];
								dst.v = src.v;
								dst.c = src.c;
								if (dst.c == "")
									dst.s.graphics.clear();
								else
									dst.s.graphics.drawRect(0, 0, 20, 20, dst.c);
							}
						}

						lines++;
	
						y++;
					}
				}
				if (lines > 0)
				{
					if (lines >= 4)
						lines *= 50;
					else if (lines >= 3)
						lines *= 20;
					else
						lines *= 10;
					this.score += lines;
					this.text_score.text = this.score;

					if (Levels.length - 1 > this.curr_level)
					{
						if (Levels[this.curr_level + 1].score <= this.score)
						{
							this.curr_level++;
							this.text_level.text = this.curr_level + 1;

							for (var j = 0; j < 2; j++)
							{
								var curr_t = this.curr_tetrises[j];
								if (curr_t)
									curr_t.down_frame_max = Levels[this.curr_level].speed;
							}
						}
					}
				}
	
				var curr_t = new Tetris(
					(Scene.cx / 2 - 4) / 2 + (1 - i) * Scene.cx / 2, 0,
					this.next_tetris_styles[0].style,
					Levels[this.curr_level].speed);
				for (var j = 0; j < this.next_tetris_style_count - 1; j++)
					this.next_tetris_styles[j].style = this.next_tetris_styles[j + 1].style;
				this.next_tetris_styles[this.next_tetris_style_count - 1].style = this.NewTetrisStyle();
				this.curr_tetrises[i] = curr_t;
				this.ShowNextTetrisStyles();
	
				if (!curr_t.Check(this.grids, 0))
				{
					var gameover = true;

					for (var y = 0 ; y < Scene.cy; y++)
					{
						for (var x = 0; x < Scene.cx; x++)
						{
							var item = this.grids[y * Scene.cx + x];
							item.v = 0;
							item.s.visible = false;
						}
					}
					for (var i = 0; i < 2; i++)
						this.curr_tetrises[i] = null;

					this.score = this.curr_level = 0;

					this.text_score.text = this.score;
					this.text_level.text = this.curr_level + 1;

					return;
				}
	
				continue;
			}

			var curr_t = this.curr_tetrises[i];
	
			if (curr_t.h_move != 0)
			{
				var tx = curr_t.x;
				curr_t.x += curr_t.h_move;
				if (!curr_t.Check(this.grids, 0) || !this.CheckForTwoTetrises([0, 0]))
					curr_t.x = tx;
			}
	
			if (curr_t.v_move == -1)
			{
				var idx = curr_t.style_idx;
				curr_t.style_idx++;
				curr_t.style_idx %= 4;
				curr_t.SetData();
				if (!curr_t.Check(this.grids, 0) || !this.CheckForTwoTetrises([0, 0]))
				{
					curr_t.style_idx = idx;
					curr_t.SetData();
				}
			}
	
			if (curr_t.v_move == 1)
				curr_t.down_frame = 0;
	
			if (curr_t.down_frame > 0)
				curr_t.down_frame--;
			else
			{
				curr_t.down_frame = curr_t.down_frame_max;
		
				curr_t.y++;
				if (!curr_t.Check(this.grids, 0))
				{
					curr_t.y--;
					curr_t.die = true;
				}
				else if (!this.CheckForTwoTetrises([0, 0]))
					curr_t.y--;
			}

			if (curr_t.v_move == 2)
			{
				while (true)
				{
					if (!curr_t.Check(this.grids, 0))
					{
						curr_t.die = true;
						break;
					}
					if (!this.CheckForTwoTetrises([0, 0]))
						break;
					curr_t.y++;
				}
				curr_t.y--;
			}

			curr_t.h_move = 0;
			curr_t.v_move = 0;
		}

		for (var i = 0; i < 2; i++)
		{
			var off = [0, 0];
			var curr_t = this.curr_tetrises[i];
			if (curr_t)
			{
				curr_t.y_max_adv = 0;
				while (curr_t.Check(this.grids, curr_t.y_max_adv) && this.CheckForTwoTetrises(off))
				{
					curr_t.y_max_adv++;
					off[i] = curr_t.y_max_adv;
				}
				curr_t.y_max_adv--;
				curr_t.SetToGrids(this.grids, curr_t.y_max_adv, 2, 2);

			}
		}
		for (var i = 0; i < 2; i++)
		{
			var curr_t = this.curr_tetrises[i];
			if (curr_t)
				curr_t.SetToGrids(this.grids, 0, 1, 1);
		}
		for (var i = 0; i < 2; i++)
		{
			if (this.curr_tetrises[i].die)
				this.curr_tetrises[i] = null;
		}
		
		this.FrameSync();
	}

	OnKeyDown(event)
	{
		switch (event.keyCode)
		{
		case laya.events.Keyboard.LEFT:
			this.h_move = -1;
			break;
		case laya.events.Keyboard.RIGHT:
			this.h_move = 1;
			break;
		case laya.events.Keyboard.UP:
			this.v_move = -1;
			break;
		case laya.events.Keyboard.DOWN:
			this.v_move = 1;
			break;
		case laya.events.Keyboard.SPACE:
			this.v_move = 2;
			break;
		}
	}

	OnMouseDown(event)
	{
		this.mouse_x = event.stageX;
		this.mouse_y = event.stageY;
		this.mouse_pressing = true;
		this.mouse_pos_shifted = false;
		this.mouse_down_in_short_time = true;
		var thiz = this;
		Laya.timer.once(100, null, function(){
			thiz.mouse_down_in_short_time = false;
		});
	}

	OnMouseUp(event)
	{
		if (this.mouse_pressing && !this.mouse_pos_shifted)
		{
			if (Math.abs(event.stageX - this.mouse_x) < 5 &&
				Math.abs(event.stageY - this.mouse_y) < 5)
				this.v_move = -1;
		}

		this.mouse_pressing = false;
	}
	
	OnMouseMove(event)
	{
		if (!this.mouse_pressing)
			return;

		var x = event.stageX;
		if (x < this.mouse_x - 20)
		{
			this.h_move = -1;
			this.mouse_x = x;
			this.mouse_pos_shifted = true;
		}
		if (x > this.mouse_x + 20)
		{
			this.h_move = 1;
			this.mouse_x = x;
			this.mouse_pos_shifted = true;
		}

		var y = event.stageY;
		if (this.mouse_down_in_short_time && y > this.mouse_y + 30)
		{
			this.v_move = 2;
			this.mouse_y = y;
			this.mouse_pos_shifted = true;
			this.mouse_pressing = false;
		}
		if (y > this.mouse_y + 20)
		{
			this.v_move = 1;
			this.mouse_y = y;
			this.mouse_pos_shifted = true;
		}
	}
};

window.app = new App;

