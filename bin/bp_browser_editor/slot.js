class Slot
{
    constructor(vi, io)
    {
        this.vi = vi;
        this.node = null;
        this.links = [];
    
        this.eMain = document.createElement("div");
    
        this.eName = document.createElement("div");
        this.eName.innerHTML = vi.name;
        this.eName.style.display = "inline-block";
        
        this.eSlot = document.createElement("div");
        this.eSlot.innerHTML = '*';
        this.eSlot.classList.add("slot");
    
        var thiz = this;
    
        this.io = io;
        if (io == 0)
        {
            this.links.push(null);
    
            this.eMain.appendChild(this.eSlot);
            this.eMain.appendChild(this.eName);
    
            if (vi.default_value)
            {
                this.eMain.appendChild(document.createElement("br"));

                let sp = vi.type.split("#");
                if (sp[0] == "enum_single")
                {
                    let select = document.createElement("select");
                    select.classList.add("slot_edit1");

                    var e = find_enum(sp[1]);
                    for (let i of e.items)
                    {
                        let o = document.createElement("option");
                        o.value = i.name;
                        o.text = i.name;
                        select.add(o);
                    }

                    this.eMain.appendChild(select);
                    this.eSelect = select;
                }
                else if (sp[0] == "enum_multi")
                {
                    let checks = [];
                    
                    var e = find_enum(sp[1]);
                    let first = true;
                    for (let i of e.items)
                    {
                        let c = document.createElement("input");
                        c.type = "checkbox";
                        c.value = i.value;
                        c.my_name = i.name;
                        checks.push(c);
                        
                        c.classList.add("slot_edit2");
                        if (!first)
                            this.eMain.appendChild(document.createElement("br"));
                        first = false;
                        this.eMain.appendChild(c);
                        this.eMain.appendChild(document.createTextNode(i.name));
                    }

                    this.eChecks = checks;
                }
                else
                {
                    if (sp[1] == "Vec<4,uchar>")
                    {
                        let color = document.createElement("input");
                        color.type = "color";
                        color.onchange = function() {
                            thiz.get_data();

                            if (!sock_s || sock_s.readyState != 1)
                                return;

                            var req = {};
                            req.type = "update";
                            req.what = "set_data";
                            req.address = thiz.get_address();
                            req.value = thiz.data;
                            sock_s.send(JSON.stringify(req));
                        };
                        this.eMain.appendChild(color);
                        this.eColor = color;
                    }
                    else
                    {
                        let input = document.createElement("input");
                        input.classList.add("slot_edit1");
                        this.eMain.appendChild(input);
                        this.eInput = input;
                    }
                }
                
            }
    
            this.path = create_path();
    
            this.eSlot.onclick = function (e) {
                if (!mouse.curr_slot) {
                    thiz.un_link();
    
                    mouse.curr_slot = thiz;
                    mouse.path.setAttributeNS(null, "d", "");
                }
                e.stopPropagation();
            };
        }
        else
        {
            this.eMain.style.textAlign = "right";
    
            this.eMain.appendChild(this.eName);
            this.eMain.appendChild(this.eSlot);
    
            this.eSlot.onclick = function (e) {
                if (mouse.curr_slot && mouse.curr_slot.io == 0) {
                    mouse.curr_slot.links[0] = thiz;
                    thiz.links.push(mouse.curr_slot);
    
                    var a = mouse.curr_slot.get_pos();
                    var b = thiz.get_pos();
                    mouse.curr_slot.set_path(a, b);
    
                    mouse.curr_slot = null;
                    mouse.path.setAttributeNS(null, "d", "");
                }
                e.stopPropagation();
            };
        }
    }

    un_link()
    {
        if (this.io == 0)
        {
            if (this.links[0])
            {
                this.path.setAttributeNS(null, "d", "");
                var o = this.links[0];
                for (let i = 0; i < o.links.length; i++)
                {
                    if (o.links[i] == this)
                    {
                        o.links.splice(i, 1);
                        break;
                    }
                }
                this.links[0] = null;
            }
        }
        else
        {
            for (let i = 0; i < this.links.length; i++)
            {
                var t = this.links[i];
                t.path.setAttributeNS(null, "d", "");
                t.links[0] = null;
            }
            this.links = [];
        }
    }

    get_pos()
    {
        var offset = get_global_offset(this.eSlot);
        return {
            x: offset.left + this.eSlot.offsetWidth / 2,
            y: offset.top + this.eSlot.offsetHeight / 2
        };
    }

    set_path(a, b)
    {
        this.path.setAttributeNS(null, "d", "M" + a.x + " " + a.y + " C" + (a.x - 50) + " " + a.y + " " + (b.x + 50) + " " + b.y + " " + b.x + " " + b.y);
    }

    get_address()
    {
        return this.node.id + "." + this.vi.name;
    }

    set_data(data)
    {
        if (this.io == 0)
        {
            this.data = data;
            if (this.eInput)
                this.eInput.value = data;
            else if (this.eColor)
            {
                let sp = data.split(';');
                let str = "#";
                for (let i = 0; i < 3; i++)
                    str += parseInt(sp[i]).toString(16);
                this.eColor.value = str;
            }
            else if (this.eSelect)
            {
                for (let o of this.eSelect.options)
                {
                    if (o.text == data)
                    {
                        o.selected = true;
                        break;
                    }
                }
            }
            else if (this.eChecks)
            {
                for (let c of this.eChecks)
                {
                    c.checked = false;
                    if (data.indexOf(c.my_name) != -1)
                        c.checked = true;
                }
            }
        }
    }

    get_data()
    {
        if (this.io == 0)
        {
            if (this.eInput)
                this.data = this.eInput.value;
            else if (this.eColor)
            {
                let color = this.eColor.value;
                let str = "";
                for (let i = 0; i < 3; i++)
                    str += parseInt(color.substr(1 + i * 2, 2), 16).toString() + ";";
                str += "255";
                this.data = str;
            }
            else if (this.eSelect)
            {
                for (let o of this.eSelect.options)
                {
                    if (o.selected)
                    {
                        this.data = o.text;
                        break;
                    }
                }
            }
            else if (this.eChecks)
            {
                this.data = "";
                for (let c of this.eChecks)
                {
                    if (c.checked)
                    {
                        if (this.data != "")
                            this.data += ";";
                        this.data += c.my_name;
                    }
                }
            }
        }
    }
}

