bl_info = {
    "name": "flame model format",
    "blender": (2, 81, 6),
    "category": "Import-Export",
}

import bpy
from bpy.props import (
        BoolProperty,
        FloatProperty,
        StringProperty,
        EnumProperty,
        )
from bpy_extras.io_utils import (
        ImportHelper,
        ExportHelper,
        path_reference_mode,
        axis_conversion,
        )
from bpy_extras import io_utils, node_shader_utils

import ntpath
import xml.etree.ElementTree as ET

class ImportFmod(bpy.types.Operator, ImportHelper):
    bl_idname = "import_scene.fmod"
    bl_label = "Import Fmod"
    bl_options = {'PRESET', 'UNDO'}
    
    filename_ext = ".fmod"

    def execute(self, context):
        pass

    def draw(self, context):
        pass

def v3_str(v):
    return str(round(v[0], 4)) + "," + str(round(v[1], 4)) + "," + str(round(v[2], 4))
    
def name_compat(name):
    if name is None:
        return 'None'
    else:
        return name.replace(' ', '_')

def export_sub(n_meshes, data_file, mat_name, sub_vertics, sub_uvs, sub_normals, sub_indices):
    from array import array

    n_mesh = ET.SubElement(n_meshes, "meshe", material=mat_name)

    if sub_vertics:
        ET.SubElement(n_mesh, "positions", offset=str(data_file.tell()), size=str(4 * len(sub_vertics)))
        float_array = array('f', sub_vertics)
        float_array.tofile(data_file)
        sub_vertics.clear()

    if sub_uvs:
        ET.SubElement(n_mesh, "uvs", offset=str(data_file.tell()), size=str(4 * len(sub_uvs)))
        float_array = array('f', sub_uvs)
        float_array.tofile(data_file)
        sub_uvs.clear()

    if sub_normals:
        ET.SubElement(n_mesh, "normals", offset=str(data_file.tell()), size=str(4 * len(sub_normals)))
        float_array = array('f', sub_normals)
        float_array.tofile(data_file)
        sub_normals.clear()

    if sub_indices:
        ET.SubElement(n_mesh, "indices", offset=str(data_file.tell()), size=str(4 * len(sub_indicess)))
        uint_array = array('L', sub_indices)
        uint_array.tofile(data_file)
        sub_indices.clear()

class ExportFmod(bpy.types.Operator, ExportHelper):
    bl_idname = "export_scene.fmod"
    bl_label = "Export Fmod"
    bl_options = {'PRESET'}

    filename_ext = ".fmod"

    def execute(self, context):
        scene = context.scene

        if bpy.ops.object.mode_set.poll():
            bpy.ops.object.mode_set(mode='OBJECT')

        if len(context.selected_objects) < 1 :
            return {"CANCELLED"}

        ob = context.selected_objects[0].original
        
        oms = []
        arm = None
        if ob.type == "MESH":
            oms.append(ob)
        elif ob.type == "ARMATURE":
            arm = ob.data
            for o in ob.children:
                oms.append(o)
        else:
            return

        filename = self.filepath
        ppath = ntpath.dirname(filename)
        model_name = ntpath.splitext(ntpath.split(filename)[1])[0]
            
        n_model = ET.Element("model")
        n_meshes = ET.SubElement(n_model, "meshes")

        model_data_file = open(filename + ".dat", "wb")

        for ob in oms:
            me = ob.to_mesh()

            if len(me.uv_layers) == 0:
                return
            uvs = me.uv_layers.active.data[:]
            if len(uvs) == 0:
                ob.to_mesh_clear()
                return

            verts = me.vertices[:]
            if len(verts) == 0:
                ob.to_mesh_clear()
                return

            faces = me.polygons[:]
            if len(faces) == 0:
                ob.to_mesh_clear()
                return
            faces.sort(key=lambda a: (a.material_index, a.use_smooth))
        
            me.calc_normals_split()

            loops = me.loops
        
            materials = me.materials[:]
            material_names = []
            for i, m in enumerate(materials):
                mat_wrap = node_shader_utils.PrincipledBSDFWrapper(m)
                n_material = ET.Element("material", color=v3_str(mat_wrap.base_color), metallic=str(mat_wrap.metallic), roughness=str(mat_wrap.roughness))
            
                color_tex_wrap = getattr(mat_wrap, "base_color_texture", None)
                if color_tex_wrap:
                    image = color_tex_wrap.image
                    if image:
                        image.filepath

                material_name = m.name
                if not material_name:
                    material_name = str(i)
                material_name = (model_name + "_" + material_name + ".fmat").replace(' ', '_')
                material_names.append(material_name)
                doc = ET.ElementTree(n_material)
                doc.write(ntpath.join(ppath, material_name))

            group_names = [g.name for g in ob.vertex_groups]
            if arm:
                for b in arm.edit_bones:
                    if b.name not in group_names:
                        continue

            curr_mat_idx = faces[0].material_index
            sub_vertics = []
            sub_uvs = []
            sub_normals = []
            sub_indices = []
            vertex_dict = {}
            vert_cnt = 0
            for f in faces:
                if curr_mat_idx != f.material_index:
                    export_sub()
                    vert_cnt = 0
                for l_idx in f.loop_indices:
                    vi = loops[l_idx].vertex_index
                    uv = uvs[l_idx].uv
                    no = loops[l_idx].normal
                    key = vi, round(uv.x, 4), round(uv.y, 4), round(no.x, 4), round(no.y, 4), round(no.z, 4)
                    idx = vertex_dict.get(key)
                    if idx is None:
                        idx = vert_cnt
                        v = verts[vi].co
                        sub_vertics.append([v.x, v.y, v.z])
                        sub_uvs.append([uv.x, uv.y])
                        sub_normals.append([no.x, no.y, no.z])
                        vertex_dict[key] = idx
                        vert_cnt += 1
                    sub_indices.append(idx)
            export_sub()

            ob.to_mesh_clear()

        model_data_file.close()

        doc = ET.ElementTree(n_model)
        doc.write(filename)

        return {"FINISHED"}

    def draw(self, context):
        pass
    
def menu_func_import(self, context):
    self.layout.operator(ImportFmod.bl_idname, text="flame model (.fmod)")

def menu_func_export(self, context):
    self.layout.operator(ExportFmod.bl_idname, text="flame model (.fmod)")

def register():
    bpy.utils.register_class(ImportFmod)
    bpy.utils.register_class(ExportFmod)

    bpy.types.TOPBAR_MT_file_import.append(menu_func_import)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

def unregister():
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)

    bpy.utils.unregister_class(ImportFmod)
    bpy.utils.unregister_class(ExportFmod)

if __name__ == "__main__":
    register()
