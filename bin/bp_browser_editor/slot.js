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
                thiz.eEdit = document.createElement("input");
                thiz.eEdit.classList.add("slot_edit");
                thiz.eEdit.value = vi.default_value;
                
                this.eMain.appendChild(thiz.eEdit);
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
}

