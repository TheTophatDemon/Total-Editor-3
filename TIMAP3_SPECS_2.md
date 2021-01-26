# Total Invasion Map Format Version 3

## Order of data

TI3MAP (6 chars)

File Format Version (u16) -- Should be 1.

Texture Count (u32)

Prefabs Count (u32)

Texture Paths (64 chars each) 
...

Prefabs:
	Tile Type (u16) -- Solid, slope, door, etc.
	Flags (u16) -- Bitmask for misc. flags
	Texture Index for Front Face (u16)
	Texture Index for Back Face (u16)
	Texture Index for Left Face (u16)
	Texture Index for Right Face (u16)
	Texture Index for Top Face (u16)
	Texture Index for Bottom Face (u16)
...

Map Width (u32)
Map Height (u32)
Map Length (u32)
Number of Entities (u32)

Entities:
	Position (3 x u32)
	Rotation (3 x f32) --Euler Angles in degrees
	Type (u32)
	Flags (u32)
...

Embedded PNG
	-The last 3 bytes of each pixel encode the index of the corresponding tile prefab
	-Alpha is 0 for empty tiles and otherwise 255