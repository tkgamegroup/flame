window.onload = function(){
    var svg = document.getElementById("svg");
    svg.ns = svg.namespaceURI;

    function CreatePath() {
        var path = document.createElementNS(svg.ns, "path");
        path.setAttributeNS(null, "stroke", "#8e8e8e");
        path.setAttributeNS(null, "stroke-width", "2");
        path.setAttributeNS(null, "fill", "none");
        svg.appendChild(path);
        return path;
    }

    var nodes = [];
    window.nodes = nodes;

    var mouse = {
        curr_slot: null,
        path: CreatePath()
    };

    function FindNode(name) {
        for (var i in nodes)
        {
            var n = nodes[i];
            if (n.name == name)
                return n;
        }
        return null;
    }

    function GetGlobalOffset(element) {
        var offset = {
            top: element.offsetTop,
            left: element.offsetLeft
        };
    
        if (element.offsetParent) {
            var po = GetGlobalOffset(element.offsetParent);
            offset.top += po.top;
            offset.left += po.left;
            return offset;
        } 
        else 
            return offset;
    }

    function Node(name, x, y) {
        this.name = name;

        this.eMain = document.createElement("div");
        this.eMain.classList.add("node");
        this.eMain.setAttribute("title", name);

        this.eMain.style.left = x + "px";
        this.eMain.style.top = y + "px";

        var thiz = this;
        $(this.eMain).draggable({
            containment: "window",
            cancel: ".slot",
            drag: function (event, ui) {
                thiz.updatePosition();
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

        this.inputs = [];
        this.outputs = [];
    }

    function Slot(name, io) {
        this.name = name;
        this.node = null;
        this.link = null;
    
        this.eMain = document.createElement("div");

        this.eName = document.createElement("div");
        this.eName.innerHTML = name;
        this.eName.style.display = "inline-block";
        
        this.eSlot = document.createElement("div");
        this.eSlot.innerHTML = '*';
        this.eSlot.classList.add("slot");

        this.io = io;
        if (io == 0)
        {
            this.path = CreatePath();

            this.eMain.appendChild(this.eSlot);
            this.eMain.appendChild(this.eName);
        }
        else
        {
            this.eMain.style.textAlign = "right";

            this.eMain.appendChild(this.eName);
            this.eMain.appendChild(this.eSlot);
        }
    
        var thiz = this;
        this.eSlot.onclick = function (e) {
            if (mouse.curr_slot) {
                ;
            }
            else{
                mouse.curr_slot = thiz;
                mouse.path.setAttributeNS(null, "d", "");
            }
            e.stopPropagation();
        };
    }

    Node.prototype.AddInput = function (name) {
        var s = new Slot(name, 0);
        s.node = this;
        this.inputs.push(s);
        this.eLeft.appendChild(s.eMain);
    };

    Node.prototype.AddOutput = function (name) {
        var s = new Slot(name, 1);
        s.node = this;
        this.outputs.push(s);
        this.eRight.appendChild(s.eMain);
    };

    Node.prototype.FindInput = function (name) {
        for (var i in this.inputs)
        {
            var s = this.inputs[i];
            if (s.name == name)
                return s;
        }
        return null;
    };

    Node.prototype.FindOutput = function (name) {
        for (var i in this.outputs)
        {
            var s = this.outputs[i];
            if (s.name == name)
                return s;
        }
        return null;
    };

    Node.prototype.updatePosition = function () {
        this.x = parseInt(this.eMain.style.left);
        this.y = parseInt(this.eMain.style.top);

        for (var i in this.inputs)
        {
            var s = this.inputs[i];
            if (s.link)
            {
                var a = s.GetPos();
                var b = s.link.GetPos();
        
                s.SetPath(a, b);
            }
        }
        for (var i in nodes)
        {
            var n = nodes[i];
            for (var j in n.inputs)
            {
                var s = n.inputs[j];
                if (s.link && s.link.node == this)
                {
                    var a = s.GetPos();
                    var b = s.link.GetPos();
            
                    s.SetPath(a, b);
                }
            }

        }
    };
    
    Slot.prototype.GetPos = function () {
        var offset = GetGlobalOffset(this.eSlot);
        return {
            x: offset.left + this.eSlot.offsetWidth / 2,
            y: offset.top + this.eSlot.offsetHeight / 2
        };
    };

    Slot.prototype.SetPath = function (a, b) {
        this.path.setAttributeNS(null, "d", "M" + a.x + " " + a.y + " C" + (a.x - 50) + " " + a.y + " " + (b.x + 50) + " " + b.y + " " + b.x + " " + b.y);
    };

    window.onmousemove = function (e) {
        if (mouse.curr_slot) {
            var a = mouse.curr_slot.GetPos();
            var b = { x: e.pageX, y: e.pageY };
            mouse.path.setAttributeNS(null, "d", "M" + a.x + " " + a.y + " C" + (a.x + (mouse.curr_slot.io == 0 ? -50 : 50)) + " " + a.y + " " + b.x + " " + b.y + " " + b.x + " " + b.y);
        }
    };
    
    window.onclick = function (e) {
        if (mouse.curr_slot) {
            mouse.path.setAttributeNS(null, "d", "");
            // if (mouse.curr_slot.io == 0) {
            //     mouse.currentInput.node.detachInput(mouse.currentInput);
            // }
            mouse.curr_slot = null;
        }
    };

	var sock_s = new WebSocket("ws://localhost:5566/");
    sock_s.onmessage = function(a){
        var src = eval('(' + a.data + ')');
        var src_nodes = src.nodes;
        for (var i in src_nodes)
        {
            var sn = src_nodes[i];
            var n = new Node(sn.name, sn.x, sn.y);
            for (var j in sn.inputs)
                n.AddInput(sn.inputs[j]);
            for (var j in sn.outputs)
                n.AddOutput(sn.outputs[j]);
            nodes.push(n);
        
            n.eMain.style.position = "absolute";
            document.body.appendChild(n.eMain);
        }
        var src_links = src.links;
        for (var i in src_links)
        {
            var sl = src_links[i];

            var addr_in = sl.in.split(".");
            var addr_out = sl.out.split(".");

            var input = FindNode(addr_in[0]).FindInput(addr_in[1]);
            var output = FindNode(addr_out[0]).FindOutput(addr_out[1]);

            input.link = output;
        }
        for (var i in src_nodes)
            nodes[i].updatePosition();
        
    };
    window.sock_s = sock_s;

    /*

    var n = new Node('test', 0, 0);
    nodes.push(n);
    n.eMain.style.position = "absolute";
    document.body.appendChild(n.eMain);

    */
};

function on_save_clicked()
{
    var sock_s = window.sock_s;
    if (!sock_s)
        return;

    var dst = {};
    dst.nodes = [];
    for (var i in window.nodes)
    {
        var sn = window.nodes[i];
        var n = {};
        n.name = sn.name;
        n.x = sn.x;
        n.y = sn.y;
        dst.nodes.push(n);
    }
    for (var i in window.nodes)
    {
        
    }
    dst.links = [];

    var json = JSON.stringify(dst);
    sock_s.send(json);
}
