window.onload = function(){
    var svg = document.getElementById("svg");
    svg.ns = svg.namespaceURI;

    var nodes = [];

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
    
        this.eMain = document.createElement("div");

        this.eName = document.createElement("div");
        this.eName.innerHTML = name;
        this.eName.style.display = "inline-block";
        
        this.eSlot = document.createElement("div");
        this.eSlot.innerHTML = '*';
        this.eSlot.style.display = "inline-block";

        if (io == 0)
        {
            this.path = document.createElementNS(svg.ns, "path");
            this.path.setAttributeNS(null, "stroke", "#8e8e8e");
            this.path.setAttributeNS(null, "stroke-width", "2");
            this.path.setAttributeNS(null, "fill", "none");
            svg.appendChild(this.path);

            this.eMain.appendChild(this.eSlot);
            this.eMain.appendChild(this.eName);

            this.link = null;
        }
        else
        {
            this.eMain.style.textAlign = "right";

            this.eMain.appendChild(this.eName);
            this.eMain.appendChild(this.eSlot);
            
            this.link = [];
        }
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
        for (var i in this.inputs)
        {
            var s = this.inputs[i];
            if (s.link)
            {
                var a = s.GetPos();
                var b = s.link.GetPos();
        
                s.path.setAttributeNS(null, "d", "M" + a.x + " " + a.y + " C" + (a.x-50) + " " + a.y + " " + (b.x+50) + " " + b.y + " " + b.x + " " + b.y);
            }
        }
        for (var i in this.outputs)
        {
            var s = this.outputs[i];
            for (var j = 0; j < s.link.length; j++)
            {
                var a = s.link[j].GetPos();
                var b = s.GetPos();
        
                s.link[j].path.setAttributeNS(null, "d", "M" + a.x + " " + a.y + " C" + (a.x-50) + " " + a.y + " " + (b.x+50) + " " + b.y + " " + b.x + " " + b.y);
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

    function FindNode(name) {
        for (var i in nodes)
        {
            var n = nodes[i];
            if (n.name == name)
                return n;
        }
        return null;
    }

	var sock_s = new WebSocket("ws://localhost:5566/");
    sock_s.onmessage = function(a){
        var src = eval('(' + a.data + ')');
        var src_nodes = src.nodes;
        for (var i in src_nodes)
        {
            var src = src_nodes[i];
            var n = new Node(src.name, src.x, src.y);
            for (var j in src.inputs)
                n.AddInput(src.inputs[j]);
            for (var j in src.outputs)
                n.AddOutput(src.outputs[j]);
            nodes.push(n);
        
            n.eMain.style.position = "absolute";
            document.body.appendChild(n.eMain);
        }
        var src_links = src.links;
        for (var i in src_links)
        {
            var src = src_links[i];

            var addr_in = src.in.split(".");
            var addr_out = src.out.split(".");

            var input = FindNode(addr_in[0]).FindInput(addr_in[1]);
            var output = FindNode(addr_out[0]).FindOutput(addr_out[1]);

            input.link = output;
            output.link.push(input);
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
    
    var mouse = {
        currentInput: null
    };
    
    window.onmousemove = function (e) {
        if (mouse.currentInput) {
            var p = mouse.currentInput.path;
            var iP = mouse.currentInput.getAttachPoint();
            var oP = { x: e.pageX, y: e.pageY };
            var s = mouse.createPath(iP, oP);
            p.setAttributeNS(null, "d", s);
        }
    };
    
    window.onclick = function (e) {
        if (mouse.currentInput) {
            mouse.currentInput.path.removeAttribute("d");
            if (mouse.currentInput.node) {
                mouse.currentInput.node.detachInput(mouse.currentInput);
            }
            mouse.currentInput = null;
        }
    };
    
    function Node(name) {
        // Create output visual
        var outDom = document.createElement("span");
        outDom.classList.add("output");
        outDom.innerHTML = "&nbsp;";
        this.domElement.appendChild(outDom);
    
        // Output Click handler
        var that = this;
        outDom.onclick = function (e) {
            if (mouse.currentInput && !that.ownsInput(mouse.currentInput)) {
                var tmp = mouse.currentInput;
                mouse.currentInput = null;
                that.connectTo(tmp);
            }
            e.stopPropagation();
        };
    }
    
    function NodeInput(name) {
        this.name = name;
        this.node = null;
    
        // setup the varying input types
        this.domElement = document.createElement("div");
        this.domElement.innerHTML = name;
        this.domElement.classList.add("connection");
        this.domElement.classList.add("empty");
    
        // DOM Event handlers
        var that = this;
        this.domElement.onclick = function (e) {
            if (mouse.currentInput) {
                if (mouse.currentInput.path.hasAttribute("d"))
                    mouse.currentInput.path.removeAttribute("d");
                if (mouse.currentInput.node) {
                    mouse.currentInput.node.detachInput(mouse.currentInput);
                    mouse.currentInput.node = null;
                }
            }
            mouse.currentInput = that;
            if (that.node) {
                that.node.detachInput(that);
                that.domElement.classList.remove("filled");
                that.domElement.classList.add("empty");
            }
            e.stopPropagation();
        };
    }
    
    Node.prototype.detachInput = function (input) {
        var index = -1;
        for (var i = 0; i < this.attachedPaths.length; i++) {
            if (this.attachedPaths[i].input == input) index = i;
        }
    
        if (index >= 0) {
            this.attachedPaths[index].path.removeAttribute("d");
            this.attachedPaths[index].input.node = null;
            this.attachedPaths.splice(index, 1);
        }
    
        if (this.attachedPaths.length <= 0) {
            this.domElement.classList.remove("connected");
        }
    };
    
    Node.prototype.connectTo = function (input) {
        input.node = this;
        this.connected = true;
        this.domElement.classList.add("connected");
    
        input.domElement.classList.remove("empty");
        input.domElement.classList.add("filled");
    
        this.attachedPaths.push({
            input: input,
            path: input.path
        });
    
        var iPoint = input.getAttachPoint();
        var oPoint = this.getOutputPoint();
    
        var pathStr = this.createPath(iPoint, oPoint);
    
        input.path.setAttributeNS(null, "d", pathStr);
    };
    
    Node.prototype.moveTo = function (point) {
        this.domElement.style.top = point.y + "px";
        this.domElement.style.left = point.x + "px";
        this.updatePosition();
    };
    
    Node.prototype.initUI = function () {
        $(this.domElement).draggable({
            containment: "window",
            cancel: ".connection,.output"
        });
    };

    /*
    // Test nodes.
    var node01 = new Node("Generate Cube");
    node01.addInput("Name");
    node01.addInput("Size");
    
    var node02 = new Node("Add");
    node02.addInput("Left&nbsp;&nbsp;&nbsp;&nbsp;");
    node02.addInput("Right&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
    
    var node03 = new Node("Translate");
    node03.addInput("Object");
    node03.addInput("X");
    node03.addInput("Y");
    node03.addInput("Z");
    
    // Move and connect.
    node01.moveTo({ x: 75, y: 125 });
    node02.moveTo({ x: 350, y: 20 });
    node03.moveTo({ x: 500, y: 150 });
    node01.connectTo(node02.inputs[0]);
    node03.connectTo(node02.inputs[1]);
    
    // Add to canvas
    node01.initUI();
    node02.initUI();
    node03.initUI();
    */
};

function on_save_clicked()
{
    var sock_s = window.sock_s;
    if (!sock_s)
        return;

    
}
