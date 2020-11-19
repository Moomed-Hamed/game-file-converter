#pragma once
#include "intermediary.h"

// NOTE : matrices in collada are row-major, so you probably need to transpose them

// the easy one
void convert_collada(const char* path, const char* binary_name, const char* text_name)
{
	File_Reader_Collada file = {};
	file.read_file(path);

	int num_dae_positions = -1, num_dae_normals = -1, num_dae_vertices = -1, num_dae_faces = -1;

	vec3*  dae_positions = NULL;
	vec3*  dae_normals   = NULL;
	ivec2* dae_vertices  = NULL;

	// Library Geometries : positions, normals, faces, vertices
	{
		file.seek_tag("geometry");

		dae_positions = (vec3*)file.parse_float_array(&num_dae_positions); num_dae_positions /= 3;
		dae_normals   = (vec3*)file.parse_float_array(&num_dae_normals);   num_dae_normals   /= 3;

		file.seek_tag("triangles");
		sscanf(file.read_ptr, "%*s %*s %*[a-z=\"] %d", &num_dae_faces);
		num_dae_vertices = num_dae_faces * 3;

		file.seek_tag("p"); file.prev_line(); file.prev_line();

		// there could be extra info packed in, (stride - 2) = how many values to skip per read 
		int stride = -1;
		sscanf(file.read_ptr, " %*s %*s %*s %*[a-z=\"] %d", &stride); ++stride;

		file.seek_tag("p");
		file.read_ptr += 2;

		dae_vertices = (ivec2*)malloc(num_dae_vertices * sizeof(ivec2));

		for (int i = 0; i < num_dae_vertices; i++)
		{
			dae_vertices[i].x = file.parse_int();
			dae_vertices[i].y = file.parse_int();
			for (int i = 0; i < stride - 2; i++) file.parse_int(); // skip unwanted info
		}
	}

	//print("num_positions: %d\n", num_dae_positions);
	//print("num_normals: %d\n"  , num_dae_normals);
	//print("num_faces: %d\n"    , num_dae_faces);

	file.free_memory();

	Mesh_Data final_mesh = make_mesh_data(num_dae_positions, num_dae_normals, num_dae_vertices,
		dae_positions, dae_normals, dae_vertices);
	
	save_mesh_data(final_mesh, binary_name, text_name);
	free_mesh_data(&final_mesh);

	print("finished converting %s\n", path);
}

// the insanely difficult one
void convert_collada_anim(const char* path, const char* binary_name, const char* text_name)
{
	File_Reader_Collada file = {};
	file.read_file(path);

	//mesh
	int num_dae_positions = -1, num_dae_normals = -1, num_dae_vertices = -1, num_dae_faces = -1;
	vec3*  dae_positions = NULL;
	vec3*  dae_normals   = NULL;
	ivec2* dae_vertices  = NULL;

	// Library Geometries : positions, normals, faces, vertices
	{
		file.seek_tag("geometry");

		dae_positions = (vec3*)file.parse_float_array(&num_dae_positions); num_dae_positions /= 3;
		dae_normals   = (vec3*)file.parse_float_array(&num_dae_normals);   num_dae_normals   /= 3;

		file.seek_tag("triangles");
		sscanf(file.read_ptr, "%*s %*s %*[a-z=\"] %d", &num_dae_faces);
		num_dae_vertices = num_dae_faces * 3;

		file.seek_tag("p"); file.prev_line(); file.prev_line();

		// there could be extra info packed in, (stride - 2) = how many values to skip per read 
		int stride = -1;
		sscanf(file.read_ptr, " %*s %*s %*s %*[a-z=\"] %d", &stride); ++stride;

		file.seek_tag("p");
		file.read_ptr += 2;

		dae_vertices = (ivec2*)malloc(num_dae_vertices * sizeof(ivec2));

		for (int i = 0; i < num_dae_vertices; i++)
		{
			dae_vertices[i].x = file.parse_int();
			dae_vertices[i].y = file.parse_int();
			for (int i = 0; i < stride - 2; i++) file.parse_int(); // skip unwanted info
		}
	}

	print("num_positions: %d\n", num_dae_positions);
	print("num_normals: %d\n"  , num_dae_normals);
	print("num_faces: %d\n"    , num_dae_faces);

	// The goal here is to make an array of bones sorted by depth, where bone = { bone_name, parent_index }
	// To do this we first parse out the dae file's representation of the skeleton.
	// Generating the final array requires the assembly of the following arrays:
	// bone_names   : bone_names[i]   = [i]th dae_bone's name 
	// bone_parents : bone_parents[i] = [i]th dae_bone's parent-bone index in dae array
	// bone_depths  : bone_depths[i]  = [i]th dae_bone's depth, where 0 is root depth
	// bone_ids     : index of the 3 bones that influence dae_positions[i]
	// bone_weights : [i]th dae_bone's inverse_bind_matrix
	// bone_local_transforms     : [i]th dae_bone's bind_matrix
	// bone_inv_local_transforms : [i]th dae_bone's inverse_bind_matrix

	int num_bones = 0;
	char** bone_names = NULL;

	mat4*  bone_inv_local_transforms = NULL;
	mat4*  bone_local_transforms     = NULL;
	ivec3* bone_ids  = NULL; // the 3 bones that influence a vertex
	vec3*  weights   = NULL;
	Anim_Bone* bones = NULL; // this will be the final sorted array

	// First we get the weights, bone names, bone count, and inverse-bind matrices

	// Library Controllers : bone_names, vertex_bones, weights, inv_local_transforms
	{
		file.seek_tag("library_controllers");

		// parse bone names
		bone_names = file.parse_name_array(&num_bones);
		//for (int i = 0; i < num_bones; i++) print("%s ", bone_names[i]);

		// read inverse-bind matrices
		int num_ibm = 0;
		bone_inv_local_transforms = (mat4*)file.parse_float_array(&num_ibm);  num_ibm /= 16;
		//print("\nnum ibm: %d\n", num_ibm); // should be the same as num_joints

		// parse weight values (*NOT* the weights, just values used by the weights)
		int num_weight_values = 0;
		float* weight_values = file.parse_float_array(&num_weight_values);
		//print("num weight values: %d\n", num_weight_values);

		// parse weights and bone_ids (3 bones that move a vertex)
		weights  = (vec3*) calloc(num_dae_positions, sizeof(vec3));
		bone_ids = (ivec3*)calloc(num_dae_positions, sizeof(ivec3));

		file.seek_tag("vcount"); file.seek_char('>');

		// num_vertex_bones[i] = number of bones affecting dae_vertex[i], NOT number of bone_ids
		int* num_vertex_bones = new int[num_dae_positions];
		for (int i = 0; i < num_dae_positions; i++) num_vertex_bones[i] = file.parse_int();

		file.seek_tag("v"); file.seek_char('>');
		for (int i = 0; i < num_dae_positions; i++) // num_dae_positions b/c each dae_pos has exactly 1 weight
		{
			for (int j = 0; j < num_vertex_bones[i]; j++) // find 3 most significant bones by weight
			{
				int bone_index = file.parse_int(); // get one of the bones afecting this vertex
				float weight_value = weight_values[file.parse_int()]; // get it's weight VALUE from the list

				if (weight_value > weights[i].z) // then this weight is more significant
				{
					bone_ids[i].z = bone_index;
					weights[i].z  = weight_value;
				}

				sort_weights_decending(&weights[i], &bone_ids[i]); // now weights.z is the least significant
			}

			make_sum_equal_one(&weights[i]); // this is NOT the same as normalizing!
		}

		free(weight_values);
		free(num_vertex_bones);
	}

	// now we have the weights, bone names, bone count, and inverse-bind matrices
	// next we figure out the bone heirarchy from the node list & get the bind-matrices

	// Library Visual Scenes : bone heirarchy, local transforms
	{
		bone_local_transforms = (mat4*)calloc(num_bones, sizeof(mat4));
		int* bone_depths = (int*)malloc(num_bones * sizeof(int)); // WARNING : calloc breaks this and idk why
		int* parent_pos  = (int*)malloc(num_bones * sizeof(int)); // parent index in dae array, NOT final array

		memset(bone_depths, -1, num_bones * sizeof(int));
		memset(parent_pos , -1, num_bones * sizeof(int));

		// locate the root-bone node
		file.seek_tag("library_visual_scenes");
		while (file.read_ptr < file.end_ptr)
		{
			int dummy;
			char* bone_name = file.parse_node(&dummy, 32);
			if (strcmp(bone_name, bone_names[0]) == 0) break;
		}

		// manually fill root-bone info
		bone_depths[0] = 0;
		parent_pos[0]  = 0;
		bone_local_transforms[0] = file.parse_matrix();

		// parse the rest of the skeleton for bone_depths & local_transforms
		// WARNING : idk if this algorithm works in all scenarios
		int bone_depth = 1;
		for (int i = 1; i < num_bones; i++)
		{
			int num_extras = -1; // number of <extra> tags, used to find parents
			char* bone_name = file.parse_node(&num_extras, 32);
			bone_depth -= num_extras;

			bone_depths[i] = bone_depth;
			bone_local_transforms[i] = file.parse_matrix(); // parse local transform

			if (num_extras > 0) // this node's parent is farther up the list
			{
				for (int j = 0; j < num_bones; j++)
				{
					if (bone_depths[j] == bone_depth - 1) parent_pos[i] = j;
				}
			}
			else // this bone's parent is the previous bone
			{
				parent_pos[i] = i - 1;
			}

			++bone_depth;
		}

		// print the unsorted joint heirarchy for verification
		//for (int i = 0; i < num_bones; i++) print("%-12s : %-12s : %2d\n", bone_names[i], bone_names[parent_pos[i]], bone_depths[i]);

		// now we have all the data needed to construct the final bone array
		// the first step is to sort it by depth

		// new_index[i] = index of bone_names[i] in the sorted array
		int* new_index = (int*)malloc(num_bones * sizeof(int));

		new_index[0] = 0; // root bone

		int index = 1;
		int current_depth = 1; // for every bone of this depth
		for (int i = 1; i < num_bones; i++)
		{
			for (int j = 1; j < num_bones; j++)
			{
				if (bone_depths[j] == current_depth) // if it has the correct depth
				{
					new_index[index++] = j; // put in next available position in new array
				}
			}
			current_depth++;
		}

		// print the sorted joint heirarchy for verification
		for (int i = 0; i < num_bones; i++) print("%2d : %-12s : %-12s\n", bone_depths[new_index[i]], bone_names[new_index[i]], bone_names[parent_pos[new_index[i]]]);

		// we must also update all bone references
		for (int i = 0; i < num_dae_positions; i++)
		{
			bone_ids[i].x = new_index[bone_ids[i].x];
			bone_ids[i].y = new_index[bone_ids[i].y];
			bone_ids[i].z = new_index[bone_ids[i].z];
		}

		// now that the bones are sorted, we can assemble the final array!
		bones = (Anim_Bone*)calloc(num_bones, sizeof(Anim_Bone));
		for (int i = 0; i < num_bones; i++)
		{
			bones[i].name = bone_names[new_index[i]];
			bones[i].parent_index = parent_pos[new_index[i]];
			bones[i].local_transform = bone_local_transforms[new_index[i]];
			bones[i].inv_local_transform = bone_inv_local_transforms[new_index[i]];
		}

		free(new_index);
		// TODO : FREE ALL UNNEEDED MEMORY!!!
	}

	Bone_Animation* animations = new Bone_Animation[num_bones];
	int num_animated_bones = 0;

	// Library Animations : animation keyframes
	{
		file.read_ptr = file.start_ptr; // go back to start of file
		file.seek_tag("library_animations");

		while (file.read_ptr < file.end_ptr)
		{
			// we get the bone name from the <channel> tag
			file.seek_tag("animation");
			file.seek_tag("channel");
			if (file.read_ptr >= file.end_ptr) break;

			char name[64] = {};
			sscanf(file.read_ptr, "%*s %*s target=\"Armature_%[^/\" \n]", name);

			file.seek_tag_previous("animation");

			int num_keyframes = -1;

			float* times = file.parse_float_array(&num_keyframes);
			int framerate = (int)((float)1 / (times[1] - times[0]));
			
			free(times);

			animations[num_animated_bones].framerate = framerate;
			animations[num_animated_bones].keyframes = (mat4*)file.parse_float_array(&num_keyframes);
			animations[num_animated_bones].num_keyframes = num_keyframes / 16;
			num_animated_bones++;

			print("[%-12s] : %d keyframes at %dfps\n", name, num_keyframes / 16, framerate);
		}
	}

	file.free_memory();

	// save the animated mesh
	Mesh_Data_Anim final_mesh = make_mesh_data(num_dae_positions, num_dae_normals, num_dae_vertices,
		dae_positions, dae_normals, weights, bone_ids, dae_vertices);

	save_mesh_data(final_mesh, binary_name, text_name);
	free_mesh_data(&final_mesh);

	// save the animation keyframes
	save_animation_data(animations, num_animated_bones, bones, num_bones, NULL, "animations.txt");

	print("\nfinished converting '%s'\n", path);
}