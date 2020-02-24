ECHO OFF
SET FILES=
SET FILES=%FILES% bin/tetris.exe
SET FILES=%FILES% bin/freetype.dll
SET FILES=%FILES% renderpath/canvas/bp
SET FILES=%FILES% renderpath/canvas/element.vert
SET FILES=%FILES% renderpath/canvas/element.frag
SET FILES=%FILES% renderpath/canvas/text_lcd.frag
SET FILES=%FILES% renderpath/canvas/text_sdf.frag
SET FILES=%FILES% renderpath/canvas/build/Debug/bp.dll
SET FILES=%FILES% art/font_awesome.ttf
SET FILES=%FILES% game/tetris/art/atlas/main.png
SET FILES=%FILES% game/tetris/art/atlas/main.atlas
"../../bin/package_maker.exe" -r../../ -drelease/ %FILES%
