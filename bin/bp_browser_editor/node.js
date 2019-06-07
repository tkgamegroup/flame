class Node
{
    constructor(sn)
    {
        var thiz = this;

        this.id = sn.id;
        this.type = sn.type;
    
        this.eMain = document.createElement("div");
        this.eMain.classList.add("node");
        this.eMain.setAttribute("title", this.id);
    
        var pos_sp = sn.pos.split(";");
        this.x = parseInt(pos_sp[0]);
        this.y = parseInt(pos_sp[1]);
        this.eMain.style.left = this.x.toString() + "px";
        this.eMain.style.top = this.y.toString() + "px";
    
        var thiz = this;
        $(this.eMain).draggable({
            containment: "window",
            cancel: ".slot, .slot_edit",
            drag: function (event, ui) {
                thiz.update_links_positions();
            }
        });
    
        this.eLeft = document.createElement("div");
        this.eLeft.style.display = "inline-block";
        this.eLeft.style.marginRight = "30px";
        this.eMain.appendChild(this.eLeft);
    
        this.eRight = document.createElement("div");
        this.eRight.style.display = "inline-block";
        this.eRight.style.float = "right";
        this.eMain.appendChild(this.eRight);
    
        this.btnClose = document.createElement("button");
        this.btnClose.type = "button";
        this.btnClose.innerHTML = "X";
        this.btnClose.style.position = "absolute";
        this.btnClose.style.left = "0px";
        this.btnClose.style.top = "-20px";
        this.btnClose.onclick = function(){
            remove_node(thiz);
        };
        this.eMain.appendChild(this.btnClose);
    
        this.inputs = [];
        this.outputs = [];
    
        var sp = sn.type.split("#");
        function load(u_name)
        {
            var udt = find_udt(u_name);
            if (!udt)
                return false;
    
            thiz.udt = udt;
            for (var i in udt.items)
            {
                var item = udt.items[i];
                if (item.attribute.indexOf("i") >= 0)
                {
                    var s = new Slot(item, 0);
                    s.node = thiz;
                    if (item.default_value)
                        s.data = item.default_value;
                    else
                        s.data = "";
                    thiz.inputs.push(s);
                    thiz.eLeft.appendChild(s.eMain);
                }
                else if (item.attribute.indexOf("o") >= 0)
                {
                    var s = new Slot(item, 1);
                    s.node = thiz;
                    thiz.outputs.push(s);
                    thiz.eRight.appendChild(s.eMain);
                }
            }
            if (sn.datas != "null")
            {
                for (var i in sn.datas)
                {
                    var item = sn.datas[i];
                    var input = thiz.find_input(item.name);
                    input.data = item.value;
                    if (input.eEdit)
                        input.eEdit.value = input.data;
                }
            }
            for (var i = 0; i < staging_links.length; i++)
            {
                var sl = staging_links[i];
    
                var addr_out = sl.out.split(".");
                var addr_in = sl.in.split(".");
    
                var n1 = find_node(addr_out[0]);
                var n2 = find_node(addr_in[0]);
                var output = n1.find_output(addr_out[1]);
                var input = n2.find_input(addr_in[1]);
    
                if (input && output)
                {
                    input.links[0] = output;
                    output.links.push(input);
    
                    n1.update_links_positions();
                    n2.update_links_positions();
    
                    staging_links.splice(i, 1);
                    i--;
                }
            }
            return true;
        };
        if (sp.length == 1)
            console.assert(load(sp[0]));
        else
        {
            if (!load(sp[1]))
            {
                var url = filepath + "/" + sp[0] + ".typeinfo";
                $.getJSON(url, function(res, status){
                    if (status == "success")
                    {
                        load_typeinfo(res, sp[0]);
                        console.assert(load(sp[1]));
                    }
                    else
                        console.log("load failed: " + url);
                });
            }
        }
    }

    find_input(name)
    {
        for (var i in this.inputs)
        {
            var s = this.inputs[i];
            if (s.vi.name == name)
                return s;
        }
        return null;
    }

    find_output(name)
    {
        for (var i in this.outputs)
        {
            var s = this.outputs[i];
            if (s.vi.name == name)
                return s;
        }
        return null;
    }

    update_links_positions()
    {
        this.x = parseInt(this.eMain.style.left);
        this.y = parseInt(this.eMain.style.top);
    
        for (var i in this.inputs)
        {
            var s = this.inputs[i];
            if (s.links[0])
            {
                var a = s.get_pos();
                var b = s.links[0].get_pos();
        
                s.set_path(a, b);
            }
        }
        for (var i in this.outputs)
        {
            var s = this.outputs[i];
            for (var j in s.links)
            {
                var t = s.links[j];
    
                var a = t.get_pos();
                var b = s.get_pos();
        
                t.set_path(a, b);
            }
        }
    }
}

