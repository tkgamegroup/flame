var enums = [];
var udts = [];
var functions = [];

var svg;
var mouse;

var nodes = [];
var staging_links = [];

var sock_s = null;

var filename = null;
var filepath = null;

var add_dialog;
var add_dialog_list;
var add_dialog_type;
var overlay;

function get_global_offset(element) 
{
    var offset = {
        top: element.offsetTop,
        left: element.offsetLeft
    };

    if (element.offsetParent) 
    {
        var po = get_global_offset(element.offsetParent);
        offset.top += po.top;
        offset.left += po.left;
        return offset;
    } 
    else 
        return offset;
}

function create_path() 
{
    var path = document.createElementNS(svg.ns, "path");
    path.setAttributeNS(null, "stroke", "#8e8e8e");
    path.setAttributeNS(null, "stroke-width", "2");
    path.setAttributeNS(null, "fill", "none");
    svg.appendChild(path);
    return path;
}

var find_udt = function(name)
{
    for (var i in udts)
    {
        var u = udts[i];
        if (u.name == name)
            return u;
    }
    return null;
};

var load_typeinfo = function(json)
{
    for (var i in json.enums)
        enums.push(json.enums[i]);
    for (var i in json.udts)
        udts.push(json.udts[i]);
    for (var i in json.functions)
        functions.push(json.functions[i]);
};

function find_node(name) 
{
    for (var i in nodes)
    {
        var n = nodes[i];
        if (n.id == name)
            return n;
    }
    return null;
}

var add_node = function(n) 
{
    nodes.push(n);

    n.eMain.style.position = "absolute";
    document.body.appendChild(n.eMain);
}

var remove_node = function(n) 
{
    for (var i = 0; i < n.inputs.length; i++)
    {
        var input = n.inputs[i];
        input.un_link();
        svg.removeChild(input.path);
    }
    for (var i = 0; i < n.outputs.length; i++)
        n.outputs[i].un_link();
    document.body.removeChild(n.eMain);
}

window.onload = function()
{
    svg = document.getElementById("svg");
    svg.ns = svg.namespaceURI;

    mouse = {
        curr_slot: null,
        path: create_path()
    };

    window.onmousemove = function (e) {
        if (mouse.curr_slot) {
            var a = mouse.curr_slot.get_pos();
            var b = { x: e.pageX, y: e.pageY };
            mouse.path.setAttributeNS(null, "d", "M" + a.x + " " + a.y + " C" + (a.x - 50) + " " + a.y + " " + b.x + " " + b.y + " " + b.x + " " + b.y);
        }
    };
    
    window.onclick = function (e) {
        if (mouse.curr_slot) {
            mouse.path.setAttributeNS(null, "d", "");
            mouse.curr_slot = null;
        }
    };
    
    var wait_typeinfos = 0;
    var start = function(){
        sock_s = new WebSocket("ws://localhost:5566/");
        sock_s.onmessage = function(res){
            for (var i in nodes)
                remove_node(nodes[i]);
    
            nodes = [];
    
            var src = eval('(' + res.data + ')');

            filename = src.file;
            filepath = filename;
            for (var i = filepath.length - 1; i > 0; i--)
            {
                if (filepath[i] == "/")
                {
                    filepath = filepath.slice(0, i);
                    break;
                }
            }
    
            for (var i in src.nodes)
            {
                var sn = src.nodes[i];
                var n = new Node(sn);
                add_node(n);
            }
    
            staging_links = src.links;
        };
        sock_s.onclose = function(){
            setTimeout(function(){
                var s = new WebSocket("ws://localhost:5566/");
                s.onmessage = sock_s.onmessage;
                s.onclose = sock_s.onclose;
            }, 2000);
        };
    };
    wait_typeinfos++;
	$.getJSON("flame_foundation.typeinfo", function(res, status){
        load_typeinfo(res);
        wait_typeinfos--;
        if (wait_typeinfos == 0)
            start();
	});
    // wait_typeinfos++;
	// $.getJSON("flame_network.typeinfo", function(res, status){
    //     load_typeinfo(res);
    //     wait_typeinfos--;
    //     if (wait_typeinfos == 0)
    //         start();
	// });
    wait_typeinfos++;
	$.getJSON("flame_graphics.typeinfo", function(res, status){
        load_typeinfo(res);
        wait_typeinfos--;
        if (wait_typeinfos == 0)
            start();
	});
    // wait_typeinfos++;
	// $.getJSON("flame_sound.typeinfo", function(res, status){
    //     load_typeinfo(res);
    //     wait_typeinfos--;
    //     if (wait_typeinfos == 0)
    //         start();
	// });
    // wait_typeinfos++;
	// $.getJSON("flame_universe.typeinfo", function(res, status){
    //     load_typeinfo(res);
    //     wait_typeinfos--;
    //     if (wait_typeinfos == 0)
    //         start();
    // });
    
    add_dialog = document.getElementById("add_dialog");
    add_dialog_list = document.getElementById("add_dialog_list");
    add_dialog_type = document.getElementById("add_dialog_type");
    overlay = document.getElementById("overlay");
};

function on_add_clicked()
{
    var add_type = document.getElementById("add_type");
    var add_id = document.getElementById("add_id");

    var desc = {
        id: add_id.value,
        type: add_type.value,
        pos: "0;0",
        datas: "null"
    };

    var n = new Node(desc);
    add_node(n);
    
    add_type.value = "";
    add_id.value = "";
}

function on_add_dialog_open_clicked()
{
    add_dialog.style.display='block';
    overlay.style.display='block';

    add_dialog_list.innerHTML = "";
    for (var i = 0; i < udts.length; i++)
    {
        var item = document.createElement("li");
        item.innerHTML = udts[i].name;
        item.onclick = function() {
            add_dialog_type.value = udts[i].name;
        };
        add_dialog_list.appendChild(item);
    }
}

function on_add_dialog_close_clicked()
{
    add_dialog.style.display='none';
    overlay.style.display='none';
}

function on_save_clicked()
{
    if (!sock_s)
        return;

    var dst = {};
    dst.nodes = [];
    for (var i in nodes)
    {
        var sn = nodes[i];
        var n = {};
        n.type = sn.type;
        n.id = sn.id;
        n.pos = sn.x + ";" + sn.y;
        n.datas = [];
        for (var j in sn.inputs)
        {
            var input = sn.inputs[j];
            var vi = input.vi;
            var type_sp = vi.type.split("#");
            if (type_sp[0] != "pointer")
            {
                input.data = input.eEdit.value;
                if (!(vi.default_value && vi.default_value == input.data))
                    n.datas.push({name: vi.name, value: input.data});
            }
        }
        dst.nodes.push(n);
    }
    dst.links = [];
    for (var i in nodes)
    {
        var sn = nodes[i];
        for (var j in sn.inputs)
        {
            var input = sn.inputs[j];
            if (input.links[0])
            {
                var sl = {};
                sl.out = input.links[0].get_address();
                sl.in = input.get_address();
                dst.links.push(sl);
            }
        }
    }

    var json = JSON.stringify(dst);
    sock_s.send(json);
}
