ECHO OFF
SET FILES=
SET FILES=%FILES% bin/tetris.exe bin/flame_type.dll
SET FILES=%FILES% bin/flame_foundation.dll
SET FILES=%FILES% bin/flame_graphics.dll
SET FILES=%FILES% bin/flame_universe.dll
SET FILES=%FILES% bin/freetype.dll
SET FILES=%FILES% renderpath/canvas/bp
SET FILES=%FILES% renderpath/canvas/element.vert
SET FILES=%FILES% renderpath/canvas/element.frag
SET FILES=%FILES% renderpath/canvas/text_lcd.frag
SET FILES=%FILES% renderpath/canvas/text_sdf.frag
SET FILES=%FILES% renderpath/canvas/build/Debug/bp.dll
SET FILES=%FILES% game/tetris/art/atlas/main.png
SET FILES=%FILES% game/tetris/art/atlas/main.png.atlas
"../../bin/package_maker.exe" -r../../ -drelease/ %FILES%
