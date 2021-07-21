#pragma once

#define  _CRT_SECURE_NO_WARNINGS // because printf is "too dangerous"
#include <stdio.h>
#include <Windows.h>
#include <fileapi.h>
#include <iostream>

#define print printf
#define printvec(vec) printf("%f %f %f\n", vec.x, vec.y, vec.z)
#define out(val) std::cout << ' ' << val << '\n'

// WARNING : this
#define MAX_BUFFER_SIZE (32768 * sizeof(vec3)) // sue me

char* read_entire_file(const char* path, int* size)
{
	DWORD BytesRead;
	HANDLE os_file = CreateFile(path, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	LARGE_INTEGER win_size;
	GetFileSizeEx(os_file, &win_size);

	char* memory = (char*)malloc(win_size.QuadPart);
	ReadFile(os_file, memory, win_size.QuadPart, &BytesRead, NULL);

	*size = win_size.QuadPart;
	return memory;
}

struct vec3 { float x, y, z; };
struct vec2 { float x, y; };
struct ivec2
{
	int x, y;
	bool operator == (ivec2 vec) { return (x == vec.x && y == vec.y); }
	bool operator != (ivec2 vec) { return (x != vec.x && y != vec.y); }
};
struct ivec3
{
	int x, y, z;
	bool operator == (ivec3 vec) { return (x == vec.x && y == vec.y && z == vec.z); }
	bool operator != (ivec3 vec) { return (x != vec.x && y != vec.y && z != vec.z); }
};

struct mat3 { float a[12]; };
struct mat4 { float a[16]; };

struct Mesh_Data
{
	int num_vertices, num_indices;
	vec3* positions;
	vec3* normals;
	int*  indices;
};

struct Mesh_Data_UV
{
	int num_vertices, num_indices;
	vec3* positions;
	vec3* normals;
	vec2* textures;
	int*  indices;
};

struct Mesh_Data_Anim
{
	int num_vertices, num_indices;
	vec3* positions;
	vec3* normals;
	vec3* weights;
	ivec3* bone_ids;
	int*  indices;
};

Mesh_Data make_mesh_data(int num_positions, int num_normals, int num_vertices, vec3* positions, vec3* normals, ivec2* vertices)
{
	int num_unique_vertices = 0; // vertex count of final mesh
	int num_indices = num_vertices; // duplicate verts will be replaced using indices

	int* indices = (int*)malloc(num_indices * sizeof(int));
	ivec2* unique_vertices = vertices;

	for (int i = 0; i < num_vertices; ++i)
	{
		ivec2 test_vertex = vertices[i];
		if (test_vertex != ivec2{ -1, -1 }) // else it has already been marked as duplicate
		{
			// mark all instances of test_vertex as duplicates
			for (int j = 0; j < num_vertices; ++j)
			{
				if (vertices[j] == test_vertex)
				{
					vertices[j] = { -1, -1 }; // mark as duplicate

					// if obj_vertices[j] got mapped to unique_vertices[x], then indices[j] has value x
					indices[j] = num_unique_vertices;
				}
			}

			// keep one copy
			unique_vertices[num_unique_vertices++] = test_vertex;
		}
	}

	vec3* final_positions = (vec3*) malloc(num_unique_vertices * sizeof(vec3));
	vec3* final_normals   = (vec3*) malloc(num_unique_vertices * sizeof(vec3));

	for (int i = 0; i < num_unique_vertices; ++i)
	{
		final_positions[i] = positions[unique_vertices[i].x];
		final_normals[i]   = normals[unique_vertices[i].y];
	}

	// should this happen in this function?
	free(positions);
	free(normals);
	free(vertices);

	Mesh_Data ret = {};
	ret.num_vertices = num_unique_vertices;
	ret.num_indices  = num_indices;
	ret.positions    = final_positions;
	ret.normals      = final_normals;
	ret.indices      = indices;
	return ret;
}
Mesh_Data_UV make_mesh_data(int num_positions, int num_normals, int num_textures, int num_vertices, vec3* positions, vec3* normals, vec2* textures, ivec3* vertices)
{
	int num_unique_vertices = 0; // vertex count of final mesh
	int num_indices = num_vertices; // duplicate verts will be replaced using indices

	int* indices = (int*)malloc(num_indices * sizeof(int));
	ivec3* unique_vertices = vertices;

	for (int i = 0; i < num_vertices; ++i)
	{
		ivec3 test_vertex = vertices[i];
		if (test_vertex != ivec3{ -1, -1 , -1}) // else it has already been marked as duplicate
		{
			// mark all instances of test_vertex as duplicates
			for (int j = 0; j < num_vertices; ++j)
			{
				if (vertices[j] == test_vertex)
				{
					vertices[j] = { -1, -1, -1}; // mark as duplicate

					// if obj_vertices[j] got mapped to unique_vertices[x], then indices[j] has value x
					indices[j] = num_unique_vertices;
				}
			}

			// keep one copy
			unique_vertices[num_unique_vertices++] = test_vertex;
		}
	}

	vec3* final_positions = (vec3*) malloc(num_unique_vertices * sizeof(vec3));
	vec3* final_normals   = (vec3*) malloc(num_unique_vertices * sizeof(vec3));
	vec2* final_textures  = (vec2*) malloc(num_unique_vertices * sizeof(vec2));

	for (int i = 0; i < num_unique_vertices; ++i)
	{
		final_positions[i] = positions[unique_vertices[i].x];
		final_textures[i]  = textures [unique_vertices[i].y];
		final_normals[i]   = normals  [unique_vertices[i].z];
	}

	free(positions);
	free(normals);
	free(textures);
	free(vertices);

	Mesh_Data_UV ret = {};
	ret.num_vertices = num_unique_vertices;
	ret.num_indices  = num_indices;
	ret.positions    = final_positions;
	ret.normals      = final_normals;
	ret.textures     = final_textures;
	ret.indices      = indices;
	return ret;
}
Mesh_Data_Anim make_mesh_data(int num_positions, int num_normals, int num_vertices, vec3* positions, vec3* normals, vec3* weights, ivec3* bone_ids, ivec2* vertices)
{
	int num_unique_vertices = 0;    // vertex count of final mesh
	int num_indices = num_vertices; // duplicate verts will be replaced using indices

	int* indices = (int*)malloc(num_indices * sizeof(int));
	ivec2* unique_vertices = vertices;

	for (int i = 0; i < num_vertices; ++i)
	{
		ivec2 test_vertex = vertices[i];
		if (test_vertex != ivec2{-1, -1}) // else it has already been marked as duplicate
		{
			// mark all instances of test_vertex as duplicates
			for (int j = 0; j < num_vertices; ++j)
			{
				if (vertices[j] == test_vertex)
				{
					vertices[j] = {-1, -1}; // mark as duplicate

					// if obj_vertices[j] got mapped to unique_vertices[x], then indices[j] has value x
					indices[j] = num_unique_vertices;
				}
			}

			// keep one copy
			unique_vertices[num_unique_vertices++] = test_vertex;
		}
	}

	// since weights & bone_ids apply to positions, they get mapped to the same vertices
	vec3* final_positions = (vec3*) calloc(num_unique_vertices, sizeof(vec3));
	vec3* final_normals   = (vec3*) calloc(num_unique_vertices, sizeof(vec3));
	vec3* final_weights   = (vec3*) calloc(num_unique_vertices, sizeof(vec3));
	ivec3* final_bone_ids = (ivec3*)calloc(num_unique_vertices, sizeof(ivec3));

	for (int i = 0; i < num_unique_vertices; ++i)
	{
		final_positions[i] = positions[unique_vertices[i].x];
		final_weights[i]   = weights  [unique_vertices[i].x];
		final_bone_ids[i]  = bone_ids [unique_vertices[i].x];
		final_normals[i]   = normals  [unique_vertices[i].y];
	}

	free(positions);
	free(normals);
	free(weights); // hehe
	free(bone_ids);
	free(vertices);

	Mesh_Data_Anim ret = {};
	ret.num_vertices = num_unique_vertices;
	ret.num_indices  = num_indices;
	ret.positions    = final_positions;
	ret.normals      = final_normals;
	ret.weights      = final_weights;
	ret.bone_ids     = final_bone_ids;
	ret.indices      = indices;
	return ret;
}

void save_mesh_data(Mesh_Data data, const char* binary_name, const char* text_name)
{
	if (binary_name)
	{
		FILE* write = fopen(binary_name, "wb");

		fwrite(&data.num_vertices, sizeof(int) , 1, write);
		fwrite(&data.num_indices , sizeof(int) , 1, write);
		fwrite(data.positions    , sizeof(vec3), data.num_vertices, write);
		fwrite(data.normals      , sizeof(vec3), data.num_vertices, write);
		fwrite(data.indices      , sizeof(int) , data.num_indices, write);

		fclose(write);
	}

	if (text_name)
	{
		FILE* write = fopen(text_name, "w");

		fprintf(write, "v: %d f: %d\n", data.num_vertices, data.num_indices / 3);

		for (int i = 0; i < data.num_vertices; ++i)
		{
			vec3 position = data.positions[i];
			vec3 normal = data.normals[i];
			fprintf(write, "v %09f %09f %09f n %09f %09f %09f\n",
				position.x, position.y, position.z, normal.x, normal.y, normal.z);
		}

		for (int i = 0; i < data.num_indices; i += 3)
		{
			fprintf(write, "f %d %d %d\n", data.indices[i], data.indices[i + 1], data.indices[i + 2]);
		}

		fclose(write);
	}
}
void save_mesh_data(Mesh_Data_UV data, const char* binary_name, const char* text_name)
{
	if (binary_name)
	{
		FILE* write = fopen(binary_name, "wb");

		fwrite(&data.num_vertices, sizeof(int) , 1, write);
		fwrite(&data.num_indices , sizeof(int) , 1, write);
		fwrite(data.positions    , sizeof(vec3), data.num_vertices, write);
		fwrite(data.normals      , sizeof(vec3), data.num_vertices, write);
		fwrite(data.textures     , sizeof(vec2), data.num_vertices, write);
		fwrite(data.indices      , sizeof(int) , data.num_indices, write);

		fclose(write);
	}

	if (text_name)
	{
		FILE* write = fopen("text_version.txt", "w");

		fprintf(write, "v: %d f: %d\n", data.num_vertices, data.num_indices / 3);

		for (int i = 0; i < data.num_vertices; ++i)
		{
			vec3 position = data.positions[i];
			vec3 normal   = data.normals[i];
			vec2 texture  = data.textures[i];
			fprintf(write, "v %09f %09f %09f n %09f %09f %09f t %09f %09f\n",
				position.x, position.y, position.z, normal.x, normal.y, normal.z, texture.x, texture.y);
		}

		for (int i = 0; i < data.num_indices; i += 3)
		{
			fprintf(write, "f %d %d %d\n", data.indices[i], data.indices[i + 1], data.indices[i + 2]);
		}

		fclose(write);
	}
}
void save_mesh_data(Mesh_Data_Anim data, const char* binary_name, const char* text_name)
{
	if (binary_name)
	{
		FILE* write = fopen(binary_name, "wb");
		
		fwrite(&data.num_vertices, sizeof(int) , 1, write);
		fwrite(&data.num_indices , sizeof(int) , 1, write);
		fwrite(data.positions    , sizeof(vec3), data.num_vertices, write);
		fwrite(data.normals      , sizeof(vec3), data.num_vertices, write);
		fwrite(data.weights      , sizeof(vec3), data.num_vertices, write);
		fwrite(data.bone_ids     , sizeof(ivec3), data.num_vertices, write);
		fwrite(data.indices      , sizeof(int) , data.num_indices, write);
		
		fclose(write);
	}

	if (text_name)
	{
		FILE* write = fopen(text_name, "w");

		fprintf(write, "v: %d f: %d\n", data.num_vertices, data.num_indices / 3);

		for (int i = 0; i < data.num_vertices; ++i)
		{
			vec3 position = data.positions[i];
			vec3 normal   = data.normals[i];
			vec3 weight   = data.weights[i];
			ivec3 bones   = data.bone_ids[i];
			fprintf(write, "p %09f %09f %09f n %09f %09f %09f w %09f %09f %09f b %02d %02d %02d\n",
				position.x, position.y, position.z, normal.x, normal.y, normal.z,
				weight.x, weight.y, weight.z, bones.x, bones.y, bones.z);
		}

		for (int i = 0; i < data.num_indices; i += 3)
		{
			fprintf(write, "f %d %d %d\n", data.indices[i], data.indices[i + 1], data.indices[i + 2]);
		}

		fclose(write);
	}
}

struct Anim_Bone
{
	char* name;
	int parent_index;
	mat4 local_transform;
	mat4 inv_local_transform;
};

struct Bone_Animation
{
	int num_keyframes;
	mat4* keyframes;
	int framerate;
};

void fprint_matrix(FILE* write, mat4 matrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			fprintf(write, "%09f ", matrix.a[(i * 4) + j]);
		}

		fprintf(write, "\n");
	}
}

void save_animation_data(Bone_Animation* animations, Anim_Bone* bones, int num_bones, int num_keyframes, const char* binary_name = NULL, const char* text_name = NULL)
{
	if (binary_name)
	{
		FILE* write = fopen(binary_name, "wb");

		// skeleton
		fwrite(&num_bones, sizeof(int), 1, write);
		for (int i = 0; i < num_bones; i++)
		{
			fwrite(&(bones[i].parent_index), sizeof(int), 1, write);
		}

		for (int i = 0; i < num_bones; i++)
		{
			fwrite(&(bones[i].inv_local_transform), sizeof(mat4), 1, write);
		}
		
		fwrite(&num_keyframes, sizeof(int) , 1, write);

		for (int i = 0; i < num_bones; i++)
		{
			fwrite(animations[i].keyframes, sizeof(mat4), num_keyframes, write);
		}

		fclose(write);
	}

	if (text_name)
	{
		FILE* write = fopen(text_name, "w");

		// skeleton
		fprintf(write, "num bones: %d\n", num_bones);
		for (int i = 0; i < num_bones; ++i)
		{
			fprintf(write, "%d ", bones[i].parent_index);
		}

		// inverse-bind matrices
		fprintf(write, "inverse-bind matrices:");
		for (int i = 0; i < num_bones; ++i)
		{
			fprintf(write, "\n%d: %s\n", i, bones[i].name);
			fprint_matrix(write, bones[i].inv_local_transform);
		}

		// keyframes
		fprintf(write, "num keyframes: %d", 21);
		for (int i = 0; i < num_bones; ++i)
		{
			fprintf(write, "\n%d: %s\n", i, bones[i].name);

			for (int j = 0; j < animations[i].num_keyframes; j++)
			{
				fprint_matrix(write, animations[i].keyframes[j]);
				fprintf(write, "\n");
			}
		}

		fclose(write);
	}
}

void free_mesh_data(Mesh_Data* data)
{
	free(data->positions);
	free(data->normals);
	free(data->indices);
	*data = {};
}
void free_mesh_data(Mesh_Data_UV* data)
{
	free(data->positions);
	free(data->normals);
	free(data->textures);
	free(data->indices);
	*data = {};
}
void free_mesh_data(Mesh_Data_Anim* data)
{
	free(data->positions);
	free(data->normals);
	free(data->weights);
	free(data->bone_ids);
	free(data->indices);
	*data = {};
}

struct File_Reader
{
	int size;
	char* start_ptr, *end_ptr, *read_ptr;

	void read_file(const char* path)
	{
		this->start_ptr = this->read_ptr = read_entire_file(path, &this->size);
		this->end_ptr   = this->start_ptr + this->size;
		if (start_ptr == NULL) print("FILE ERROR: could not open '%s'\n", path);
	}
	void seek_char(char c) { while ((read_ptr++)[0] != c); }
	void seek_char_previous(char c) { while ((read_ptr--)[0] != c); }
	void next_line() { while ((read_ptr++)[0] != '\n'); }
	void prev_line() { while ((read_ptr--)[0] != '\n'); }
	void free_memory() { free(start_ptr); }
};

struct File_Reader_Collada : File_Reader
{
	void seek_tag(const char* tag)
	{
		while (read_ptr < end_ptr)
		{
			seek_char('<');
			char label[256] = {};
			sscanf(read_ptr, " %255[^> ]", label);
			if (strcmp(label, tag) == 0) return;
		}

		print("Tag not found : <%s>\n", tag);
	}
	void seek_tag_previous(const char* tag)
	{
		while (read_ptr > start_ptr)
		{
			seek_char_previous('<');
			char label[256] = {};
			sscanf(read_ptr, " <%255[^> ]", label);
			//print("%s ", label); Sleep(1000);
			if (strcmp(label, tag) == 0) return;
		}

		print("Tag not found : <%s>\n", tag);
	}
	int parse_int()
	{
		int i = -1;
		sscanf(read_ptr, " %d", &i);

		while (read_ptr[0] != ' ' && read_ptr[0] != '<') this->read_ptr++;
		this->read_ptr++;

		//print("%d\n", i);

		return i;
	}
	float parse_float()
	{
		float f = -1;
		sscanf(read_ptr, " %f", &f);

		while (read_ptr[0] != ' ' && read_ptr[0] != '<') this->read_ptr++;
		this->read_ptr++;

		return f;
	}
	char* parse_name(int max_length = 32)
	{
		char* name = (char*)calloc(max_length, sizeof(char));

		sscanf(read_ptr, " %[^< ]", name);

		while (read_ptr[0] != ' ' && read_ptr[0] != '<') this->read_ptr++;
		this->read_ptr++;

		return name;
	}
	float* parse_float_array(int* num_floats)
	{
		seek_tag("float_array");

		int count = -1;
		sscanf(read_ptr, "%*s %*s %*[a-z=\"] %d", &count);
		seek_char('>');

		float* floats = (float*)malloc(count * sizeof(float));

		for (int i = 0; i < count; ++i)
		{
			floats[i] = parse_float();
		}

		if(num_floats) *num_floats = count;
		return floats;
	}
	char** parse_name_array(int* num_names, int max_name_length = 64)
	{
		seek_tag("Name_array");

		int count = -1;
		sscanf(read_ptr, "%*s %*s %*[a-z=\"] %d", &count);
		seek_char('>');

		char** names = (char**)calloc(count, sizeof(char*));

		for (int i = 0; i < count; ++i)
		{
			names[i] = parse_name(max_name_length);
		}

		*num_names = count;
		return names;
	}
	char* parse_node(int* depth, int max_name_size = 32)
	{
		char* name = (char*)calloc(max_name_size, sizeof(char));
		int num_extra_tags = 0;

		while (read_ptr < end_ptr)
		{
			char temp_string[32] = {};

			seek_char('<');
			sscanf(read_ptr, " %[^<> \n]", temp_string); // read the tag name
			//print("%s\n", temp_string);

			if (strcmp(temp_string, "extra") == 0)
			{
				num_extra_tags++;
				next_line();
				//print("[extra] ");
			}

			if (strcmp(temp_string, "node") == 0)
			{
				sscanf(read_ptr, " %*s %*s %*[^\" ] \"%[^\" ]", name); // read the node name
				//print("%s\n", name);

				next_line();

				*depth = num_extra_tags;
				return name;
			}
		}

		*depth = -1;
		return NULL; // eof reached
	}
	mat4 parse_matrix()
	{
		seek_tag("matrix");
		seek_char('>');

		mat4 bind = {};
		for (int i = 0; i < 16; ++i) bind.a[i] = parse_float();

		return bind;
	}
};

// sorts vec3 elements in decending order
void sort_weights_decending(vec3* val, ivec3* ind)
{
	vec3  v = *val;
	ivec3 i = *ind;

	float t = 0;
	if (v.x < v.z) { t = v.z; v.z = v.x; v.x = t; t = i.z; i.z = i.x; i.x = t; }
	if (v.x < v.y) { t = v.y; v.y = v.x; v.x = t; t = i.y; i.y = i.x; i.x = t; }
	if (v.y < v.z) { t = v.z; v.z = v.y; v.y = t; t = i.z; i.z = i.y; i.y = t; }

	*val = v;
	*ind = i;
}

// used for animations
void make_sum_equal_one(vec3* vec)
{
	vec3 v = *vec;

	float difference = 1 - (v.x + v.y + v.z);
	float sum = v.x + v.y + v.z;
	v.x += difference * (v.x / sum);
	v.y += difference * (v.y / sum);
	v.z += difference * (v.z / sum);

	*vec = v;
}