# Total Invasion Map Format Version 3

In this format, both walls and floors are represented by *tiles.*
Tiles have 3 dimensional coordinates and can specify visible and invisible faces through a bit-mask.
Their coordinates are such that the top left corner of the map is always at (0, 0, 0)

From the least to most significant bit in this mask, the bits represent faces of a cube in the following order:

Front (-Z), Back (+Z), Left (-X), Right (+X), Bottom (-Y), Top (+Y)

The other major change is the introduction of the texture palette. At the beginning of the file, there is now
a list of texture names that the tile data refers back to with indices.

Entities also have 3 dimensional coordinates as well as both yaw and pitch angles.

Both tiles and entities have a type and a flag. The flag can be either used as a normal number of a bit-mask depending on the situation.
This replaces the old system of using the link id to store additional data about the wall type, like which direction a secret wall moves.

## Order of data

TIMAP (5 chars)

File Format Version (byte): Should be 3.

Texture Palette Number of Entries (byte)

Texture Name (null-term string)

Texture Name (null-term string)

...

Map Width in Tiles (short)

Map Height in Tiles (short)

Map Length in Tiles (short)

Number of Tiles (long64)

Tile #1 X (short)

Tile #1 Y (short)

Tile #1 Z (short)

Tile #1 Faces Bit-Mask (byte)

Tile #1 Texture Palette Index for Front Face (byte)

Tile #1 Texture Palette Index for Back Face (byte)

Tile #1 Texture Palette Index for Left Face (byte)

Tile #1 Texture Palette Index for Right Face (byte)

Tile #1 Texture Palette Index for Bottom Face (byte)

Tile #1 Texture Palette Index for Top Face (byte)

Tile #1 Type (byte)

Tile #1 Flag(s) (byte)

Tile #1 Link Id (byte)

...

Number of Entities (int32)

Entity #1 X (short)

Entity #1 Y (short)

Entity #1 Z (short)

Entity #1 Yaw Rotation (byte)

Entity #1 Pitch Rotation (byte)

Entity #1 Type (byte)

Entity #1 Flag(s) (byte)
