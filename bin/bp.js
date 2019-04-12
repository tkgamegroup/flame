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

        $(this.eMain).draggable();

        this.eLeft = document.createElement("div");
        this.eLeft.style.float = "left";
        this.eLeft.style.marginRight = "15px";
        this.eMain.appendChild(this.eLeft);

        this.eRight = document.createElement("div");
        this.eRight.style.float = "left";
        this.eMain.appendChild(this.eRight);

        this.inputs = [];
        this.outputs = [];
    }

    function Slot(name, io) {
        this.name = name;
        this.node = null;
    
        this.eMain = document.createElement("div");
        this.eMain.innerHTML = name;

        if (io == 0)
        {
            this.path = document.createElementNS(svg.ns, "path");
            this.path.setAttributeNS(null, "stroke", "#8e8e8e");
            this.path.setAttributeNS(null, "stroke-width", "2");
            this.path.setAttributeNS(null, "fill", "none");
            svg.appendChild(this.path);
        }
        else
            this.eMain.style.align = "right";
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
    
    Slot.prototype.GetPos = function () {
        var offset = GetGlobalOffset(this.eMain);
        return {
            x: offset.left + this.eMain.offsetWidth - 2,
            y: offset.top + this.eMain.offsetHeight / 2
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
        var obj = eval('(' + a.data + ')');
        var src_nodes = obj.nodes;
        for (var i in src_nodes)
        {
            var src = src_nodes[i];
            var n = new Node(src.name, Math.floor(i % 5) * 200, Math.floor(i / 5) * 200);
            for (var j in src.inputs)
                n.AddInput(src.inputs[j]);
            for (var j in src.outputs)
                n.AddOutput(src.outputs[j]);
            nodes.push(n);
        
            n.eMain.style.position = "absolute";
            document.body.appendChild(n.eMain);
        }
        var src_links = obj.links;
        for (var i in src_links)
        {
            var src = src_links[i];

            var addr_in = src.in.split(".");
            var addr_out = src.out.split(".");

            var input = FindNode(addr_in[0]).FindInput(addr_in[1]);
            var output = FindNode(addr_out[0]).FindOutput(addr_out[1]);

            var a = input.GetPos();
            var b = output.GetPos();
            input.path.setAttributeNS(null, "d", "M" + a.x + "," + a.y + " L" + b.x + "," + b.y);
        }
    };

    var cut = 1;

    /*
    var n1 = new Node("test", 100, 20);
    n1.AddInput("a");
    n1.AddInput("b");
    n1.AddInput("c");
    n1.AddOutput("1");
    n1.AddOutput("2");
    document.body.appendChild(n1.eMain);
    
    var mouse = {
        currentInput: null,
        createPath: function (a, b) {
            var diff = {
                x: b.x - a.x,
                y: b.y - a.y
            };
    
            var pathStr = "M" + a.x + "," + a.y + " ";
            pathStr += "C";
            pathStr += a.x + diff.x / 3 * 2 + "," + a.y + " ";
            pathStr += a.x + diff.x / 3 + "," + b.y + " ";
            pathStr += b.x + "," + b.y;
    
            return pathStr;
        }
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
        // DOM Element creation
        this.domElement = document.createElement("div");
        this.domElement.classList.add("node");
        this.domElement.setAttribute("title", name);
    
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
    
        // Node Stuffs
        this.value = "";
        this.inputs = [];
        this.connected = false;
    
        // SVG Connectors
        this.attachedPaths = [];
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
    
    Node.prototype.getOutputPoint = function () {
        var tmp = this.domElement.firstElementChild;
        var offset = GetFullOffset(tmp);
        return {
            x: offset.left + tmp.offsetWidth / 2,
            y: offset.top + tmp.offsetHeight / 2
        };
    };
    
    Node.prototype.addInput = function (name) {
        var input = new NodeInput(name);
        this.inputs.push(input);
        this.domElement.appendChild(input.domElement);
    
        return input;
    };
    
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
    
    Node.prototype.ownsInput = function (input) {
        for (var i = 0; i < this.inputs.length; i++) {
            if (this.inputs[i] == input) return true;
        }
        return false;
    };
    
    Node.prototype.updatePosition = function () {
        var outPoint = this.getOutputPoint();
    
        var aPaths = this.attachedPaths;
        for (var i = 0; i < aPaths.length; i++) {
            var iPoint = aPaths[i].input.getAttachPoint();
            var pathStr = this.createPath(iPoint, outPoint);
            aPaths[i].path.setAttributeNS(null, "d", pathStr);
        }
    
        for (var j = 0; j < this.inputs.length; j++) {
            if (this.inputs[j].node != null) {
                var iP = this.inputs[j].getAttachPoint();
                var oP = this.inputs[j].node.getOutputPoint();
    
                var pStr = this.createPath(iP, oP);
                this.inputs[j].path.setAttributeNS(null, "d", pStr);
            }
        }
    };
    
    Node.prototype.createPath = function (a, b) {
        var diff = {
            x: b.x - a.x,
            y: b.y - a.y
        };
    
        var pathStr = "M" + a.x + "," + a.y + " ";
        pathStr += "C";
        pathStr += a.x + diff.x / 3 * 2 + "," + a.y + " ";
        pathStr += a.x + diff.x / 3 + "," + b.y + " ";
        pathStr += b.x + "," + b.y;
    
        return pathStr;
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
        var that = this;
    
        // Make draggable
        $(this.domElement).draggable({
            containment: "window",
            cancel: ".connection,.output",
            drag: function (event, ui) {
                that.updatePosition();
            }
        });
        // Fix positioning
        this.domElement.style.position = "absolute";
    
        document.body.appendChild(this.domElement);
    
        // update.
        this.updatePosition();
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

