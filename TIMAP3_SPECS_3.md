ALL HAIL THE TOPHAT DEMON (25 bytes)
file_format_version (1 byte) #Should be 1
num_textures (u16)
for each texture used in the level:
	texture_name (0-terminated string)
grid_width (u16)
grid_height (u16)
grid_length (u16)
for each grid space: 
	#The grid is traversed bottom (y = 0) to top (y = length-1), left (x = 0) to right (x  = width - 1), front(z = 0) to back(z = length-1)
	vertices (u8) 
		#Each bit represents the 8 corners of the cube, traversed like the level grid
		#A convex shape is formed around the available vertices
	if no vertices:
		skip_num (u16) #Number of grid spaces to skip, excluding this one, until the next non-empty tile
	else:
		top_tex_index (u16)
		side_tex_index (u16)
		flags (u8)
			#bit 0 - panel flag (if 1, center the geometry's XZ in the cel and make it double sided)
num_entities (u32)
for each entity: #Entities determine the properties of the tiles they sit on
	grid_x (u16)
	grid_y (u16)
	grid_z (u16)
	yaw (u8) #Represents angle as increments of 45 degrees. 360 degrees = 8...
	num_properties (u8)
	for each property:
		property_name (0-terminated string)
		property_value (0-terminated string) #Must be parsed into respective data type from string