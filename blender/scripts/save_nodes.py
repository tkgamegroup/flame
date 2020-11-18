import bpy

m = bpy.data.materials['Material0']
t = m.node_tree
print("-----------")

o = t.nodes['Material Output'];

vid = 0

def fv1(v):
    return "%.2f" % v

def fv2(v):
    return "vec3(%.2f, %.2f)" % (v[0], v[1])

def fv3(v):
    return "vec3(%.2f, %.2f, %.2f)" % (v[0], v[1], v[2])

def fv4(v):
    return "vec4(%.2f, %.2f, %.2f, %.2f)" % (v[0], v[1], v[2], v[3])

def value_type(v):
    if type(v) is float:
        return 1
    if len(v) == 2:
        return 2
    elif len(v) == 3:
        return 3
    elif len(v) == 4:
        return 4
    return 0

def format_value(v):
    t = value_type(v)
    if t == 1:
        return fv1(v)
    if t == 2:
        return fv2(v)
    elif t == 3:
        return fv3(v)
    elif t == 4:
        return fv4(v)
    return ""

vs = []

def new_v(o, i, b):
    global vid
    global vs
    
    for v in vs:
        if v['o'] == o:
            return v
    
    v = {'o': o, 'i': i, 'n': "v%d" % vid, 's': get_value(i, b)}
    vid += 1
    vs.append(v)
    return v

def get_value(i, b):
    if len(i.links) == 0:
        if hasattr(i, 'default_value'):
            return format_value(i.default_value)
        
    l = i.links[0]
    n = l.from_node
    s = l.from_socket
    
    if b and len(s.links) > 1:
        return new_v(s, i, False)['n']
    
    #print(n.bl_idname)
    
    if n.bl_idname == 'ShaderNodeBsdfPrincipled':
        return get_value(n.inputs['Base Color'], True)
    elif n.bl_idname == 'ShaderNodeMath' or n.bl_idname == 'ShaderNodeVectorMath':
        if n.operation == 'ADD':
            return "(%s + %s)" % (vn, get_value(n.inputs[0], True), get_value(n.inputs[1], True))
        elif n.operation == 'SUBTRACT':
            return "(%s - %s)" % (vn, get_value(n.inputs[0], True), get_value(n.inputs[1], True))
        elif n.operation == 'MULTIPLY':
            return "(%s * %s)" % (vn, get_value(n.inputs[0], True), get_value(n.inputs[1], True))
        elif n.operation == 'DIVIDE':
            return "(%s / %s)" % (vn, get_value(n.inputs[0], True), get_value(n.inputs[1], True))
        elif n.operation == 'DOT_PRODUCT':
            return "dot(%s, %s)" % (get_value(n.inputs[0], True), get_value(n.inputs[1], True))
    elif n.bl_idname == 'ShaderNodeMixRGB':
        vn = new_v(None, n.inputs[0], True)['n']
        return "((1.0 - %s) * %s + %s * %s)" % (vn, get_value(n.inputs[1], True), vn, get_value(n.inputs[2], True))
    elif n.bl_idname == 'ShaderNodeValToRGB':
        es = n.color_ramp.elements
        if len(es) == 2:
            return "ramp2(%s, %s, %s, %s, %s)" % (get_value(n.inputs[0], True), fv1(es[0].color[0]), "%.2f" % es[0].position, fv1(es[1].color[0]), "%.2f" % es[1].position)
        return ""
    elif n.bl_idname == 'ShaderNodeTexImage':
        return "texture(maps[material.indices[%s]], i_uv * vec2(%s))" % (n.image.name, get_value(n.inputs[0], True))
    elif n.bl_idname == 'ShaderNodeNewGeometry':
        if s.name == 'Normal':
            return "N"
        else:
            return s.name
    elif n.bl_idname == 'ShaderNodeMapping':
        return get_value(n.inputs[3], True)
    return ""

res = "vec3 color = " + get_value(o.inputs['Surface'], True)
for v in vs:
    t = value_type(v['i'].default_value)
    ts = ""
    if t == 1:
        ts = "float"
    elif t == 2:
        ts = "vec2"
    elif t == 3:
        ts = "vec3"
    elif t == 4:
        ts = "vec4"
    print("%s %s = %s;" % (ts, v['n'], v['s']))
print(res + ";")
