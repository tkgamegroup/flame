bl_info = {
    "name": "Flame Model Format",
    "blender": (3, 5, 0),
    "category": "Import-Export",
}

import os
import bpy
from bpy_extras.io_utils import (
        ExportHelper
)

import ntpath

class ExportFmod(bpy.types.Operator, ExportHelper):
    bl_idname = "export_selected.fmod"
    bl_label = "Export Fmod"
    bl_options = {'PRESET'}
    
    filename_ext = "." # needed by blender
    use_filter_folder = True

    def execute(self, context):
        if len(context.selected_objects) < 1 :
            return {"CANCELLED"}

        print("Export Fmods To " + self.filepath)
        
        for obj in context.selected_objects:
            x = obj.location.x
            y = obj.location.y
            z = obj.location.z

            obj.location.x = 0
            obj.location.y = 0
            obj.location.z = 0

            path = self.filepath + obj.name + ".fbx"
            bpy.ops.export_scene.fbx(filepath=path, use_selection=True, use_triangles=True)
            os.system("%s/bin/debug/model_converter.exe %s -scaling 0.01,0.01,0.01" % (os.environ["FLAME_PATH"], path))
            os.remove(path)
        
            obj.location.x = x
            obj.location.y = y
            obj.location.z = z

        return {"FINISHED"}

    def draw(self, context):
        pass
    
def menu_func_export(self, context):
    self.layout.operator(ExportFmod.bl_idname, text="Flame Model (.fmod)")

def register():
    bpy.utils.register_class(ExportFmod)

    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)

    bpy.utils.unregister_class(ExportFmod)
