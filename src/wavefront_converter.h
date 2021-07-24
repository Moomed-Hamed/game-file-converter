#include "intermediary.h"

// NOTE : these are little endian codes
#define WAVEFRONT_POSITION 0x2076 // "v "
#define WAVEFRONT_NORMAL   0x6e76 // "vn"
#define WAVEFRONT_TEXTURE  0x7476 // "vt"
#define WAVEFRONT_FACE     0x2066 // "f "

void convert_wavefront(const char* path, const char* binary_name, const char* text_name, bool force_no_textures = false)
{
	// read the entire file into memory
	int file_size = -1;
	char* file_memory = read_entire_file(path, &file_size);
	if (file_memory == NULL || file_size <= 0) { print("FILE ERROR: could not open '%s'\n", path); return; }

	char* read_point = file_memory; // used to read & parse the file
	char* eof = file_memory + file_size; // points to last char in the file

	// These hold data that defines the mesh, the vertices can be simplified
	vec3*  obj_positions = (vec3*)calloc(MAX_BUFFER_SIZE, sizeof(char));
	vec3*  obj_normals   = (vec3*)calloc(MAX_BUFFER_SIZE, sizeof(char));
	vec2*  obj_textures  = (vec2*)calloc(MAX_BUFFER_SIZE, sizeof(char));
	char*  obj_vertices  = (char*)calloc(MAX_BUFFER_SIZE, sizeof(char));

	int num_obj_positions = 0, num_obj_normals = 0, num_obj_textures = 0, num_obj_vertices = 0;

	while (read_point < eof)
	{
		// First 2 characters interpreted as 2-byte int
		short identifier = *((short*)(read_point));

		switch (identifier)
		{
			case WAVEFRONT_POSITION: {
				float x = -1, y = -1, z = -1;
				sscanf(read_point, " v %f %f %f", &x, &y, &z);
				obj_positions[num_obj_positions++] = { x, y, z };

				//print("position: %f %f %f\n", x, y, z);
			} break;

			case WAVEFRONT_TEXTURE: {
				float u = -1, v = -1;
				sscanf(read_point, " vt %f %f", &u, &v);
				obj_textures[num_obj_textures++] = { u, v };

				//print("texture: %f %f\n", u, v);
			} break;

			case WAVEFRONT_NORMAL: {
				float x = -1, y = -1, z = -1;
				sscanf(read_point, " vn %f %f %f", &x, &y, &z);
				obj_normals[num_obj_normals++] = { x, y, z };
				
				//print("normal: %f %f %f\n", x, y, z);
			} break;

			case WAVEFRONT_FACE: {
				if (num_obj_textures > 0 && force_no_textures == false)
				{
					int p1 = -1, p2 = -1, p3 = -1, n1 = -1, n2 = -1, n3 = -1, t1 = -1, t2 = -1, t3 = -1;
					sscanf(read_point, " f %d/%d/%d %d/%d/%d %d/%d/%d", &p1, &t1, &n1, &p2, &t2, &n2, &p3, &t3, &n3);
					// wavefront file indices start at 1!
					((ivec3*)obj_vertices)[num_obj_vertices++] = { p1 - 1 , t1 -1, n1 - 1 };
					((ivec3*)obj_vertices)[num_obj_vertices++] = { p2 - 1 , t2 -1, n2 - 1 };
					((ivec3*)obj_vertices)[num_obj_vertices++] = { p3 - 1 , t3 -1, n3 - 1 };

					//print("face: (%d,%d,%d) (%d,%d,%d) (%d,%d,%d)\n", p1, t1, n1, p2, t2, n2, p3, t3, n3);
				}
				else
				{
					int p1 = -1, p2 = -1, p3 = -1, n1 = -1, n2 = -1, n3 = -1;
					sscanf(read_point, " f %d//%d %d//%d %d//%d", &p1, &n1, &p2, &n2, &p3, &n3);
					// wavefront file indices start at 1!
					((ivec2*)obj_vertices)[num_obj_vertices++] = { p1 - 1 , n1 - 1 };
					((ivec2*)obj_vertices)[num_obj_vertices++] = { p2 - 1 , n2 - 1 };
					((ivec2*)obj_vertices)[num_obj_vertices++] = { p3 - 1 , n3 - 1 };

					//print("face: (%d,%d) (%d,%d) (%d,%d)\n", p1, n1, p2, n2, p3, n3);
				}
			} break;
		}
		while (eof - read_point > 0 && (read_point++)[0] != '\n'); // go to next line
	}

	free(file_memory);
	
	if (num_obj_textures > 0 && force_no_textures == false)
	{
		Mesh_Data_UV final_mesh = make_mesh_data(num_obj_positions, num_obj_normals, num_obj_textures, num_obj_vertices,
			obj_positions, obj_normals, obj_textures,(ivec3*)obj_vertices);

		save_mesh_data(final_mesh, binary_name, text_name);
		free_mesh_data(&final_mesh);
	}
	else
	{
		Mesh_Data final_mesh = make_mesh_data(num_obj_positions, num_obj_normals, num_obj_vertices,
			obj_positions, obj_normals, (ivec2*)obj_vertices);

		save_mesh_data(final_mesh, binary_name, text_name);
		free_mesh_data(&final_mesh);
	}

	print("finished converting %s\n", path);
}