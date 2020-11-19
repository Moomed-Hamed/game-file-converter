# game-file-converter

A C++ library used to extract vertex & animation data from obj & collada files

## Features

- Extract vertex data from wavefront(.obj) files
- Extract vertex & animation data from collada(.dae) files
- Extracted information can be saved as text or in binary format(see spec below)
- This code has not been thoroughly tested, so expect many ~~bugs~~ undocumented features

## Installation

No installation required, simply copy the code into your editor and run.

Note : Code only runs on windows (see additional notes section).


## Usage

If you clone the repository, there will be example code ready for you to run/experiment with.

Additionally, an example program is shown below which outputs vertex & animation data extracted from "mesh.obj" & "animated_mesh.dae"

```c++
#include "wavefront_converter.h"
#include "collada_converter.h"

//NOTE : If you do not want certain outputs, use NULL instead of a file path

int main()
{
	convert_wavefront("mesh.obj", "binary_output_path.bin", "text_output_path.txt");
	convert_collada_anim("animated_mesh.dae", NULL, "anim_text_output_path.txt");

	return 0;
}
```

## Output File Specifications
Wavefront Text Output:

```c
v: [int : number of vertices = n] f: [int : number of faces = m]
v [float [3] : position_1.x, position_1.y, position_1.z] n [float [3] : normal_1.x, normal_1.y, normal_1.z]
v [float [3] : position_2.x, position_2.y, position_2.z] n [float [3] : normal_2.x, normal_2.y, normal_2.z]
...
v [float [3] : position_n.x, position_n.y, position_n.z] n [float [3] : normal_n.x, normal_n.y, normal_n.z]
f [int [3] : face_index_1.x, face_index_1.y, face_index_1.z]
f [int [3] : face_index_2.x, face_index_2.y, face_index_2.z]
...
f [int [3] : face_index_m.x, face_index_m.y, face_index_m.z]
```
Wavefront Mesh Binary Output:

```c
[int32 : number of vertices] [int32 : number of faces]
[f32 [3] : position_1.x, position_1.y, position_1.z] [f32 [3] : normal_1.x, normal_1.y, normal_1.z]
[f32 [3] : position_2.x, position_2.y, position_2.z] [f32 [3] : normal_2.x, normal_2.y, normal_2.z]
...
[f32 [3] : position_n.x, position_n.y, position_n.z] [f32 [3] : normal_n.x, normal_n.y, normal_n.z]
[int32 [3] : face_index_1.x, face_index_1.y, face_index_1.z]
[int32 [3] : face_index_2.x, face_index_2.y, face_index_2.z]
...
[int32 [3] : face_index_m.x, face_index_m.y, face_index_m.z]
```

Collada Animated Mesh Text Output:

```c
v: [int : number of vertices = n] f: [int : number of faces = m]
v [float [3] : position_1.x, position_1.y, position_1.z] n [float [3] : normal_1.x, normal_1.y, normal_1.z] w [float [3] : weight_1.x, weight_1.y, weight_1.z] b [int [3] : bone_id_1.x, bone_id_1.y, bone_id_1.z]
v [float [3] : position_2.x, position_2.y, position_2.z] n [float [3] : normal_2.x, normal_2.y, normal_2.z] w [float [3] : weight_2.x, weight_2.y, weight_2.z] b [int [3] : bone_id_2.x, bone_id_2.y, bone_id_2.z]
...
v [float [3] : position_n.x, position_n.y, position_n.z] n [float [3] : normal_n.x, normal_n.y, normal_n.z] w [float [3] : weight_n.x, weight_n.y, weight_n.z] b [int [3] : bone_id_n.x, bone_id_n.y, bone_id_n.z]
f [int [3] : face_index_1.x, face_index_1.y, face_index_1.z]
f [int [3] : face_index_2.x, face_index_2.y, face_index_2.z]
...
f [int [3] : face_index_m.x, face_index_m.y, face_index_m.z]
```

Collada Animation Text Output:

```c
number of animated bones: [int : number of bones = n]

name: [string : bone_1 name]
keyframes: [int : number of keyframes = m]
framerate: [int : framerate of animation in frames per second]
[float [4] : mat_1[0][0], mat_1[0][1], mat_1[0][2], mat_1[0][3]] // bone-space transforms for keyframe 1
[float [4] : mat_1[1][0], mat_1[1][1], mat_1[1][2], mat_1[1][3]]
[float [4] : mat_1[2][0], mat_1[2][1], mat_1[2][2], mat_1[2][3]]
[float [4] : mat_1[3][0], mat_1[3][1], mat_1[3][2], mat_1[3][3]]

[float [4] : mat_2[0][0], mat_2[0][1], mat_2[0][2], mat_2[0][3]] // bone-space transforms for keyframe 2 
[float [4] : mat_2[1][0], mat_2[1][1], mat_2[1][2], mat_2[1][3]]
[float [4] : mat_2[2][0], mat_2[2][1], mat_2[2][2], mat_2[2][3]]
[float [4] : mat_2[3][0], mat_2[3][1], mat_2[3][2], mat_2[3][3]]

...

[float [4] : mat_m[0][0], mat_m[0][1], mat_m[0][2], mat_m[0][3]] // etc.
[float [4] : mat_m[1][0], mat_m[1][1], mat_m[1][2], mat_m[1][3]]
[float [4] : mat_m[2][0], mat_m[2][1], mat_m[2][2], mat_m[2][3]]
[float [4] : mat_m[3][0], mat_m[3][1], mat_m[3][2], mat_m[3][3]]

...

name: [string : bone_2 name]
keyframes: [int : number of keyframes]
framerate: [int : framerate of animation in frames per second]
[float [4] : mat_1[0][0], mat_1[0][1], mat_1[0][2], mat_1[0][3]]
[float [4] : mat_1[1][0], mat_1[1][1], mat_1[1][2], mat_1[1][3]]
[float [4] : mat_1[2][0], mat_1[2][1], mat_1[2][2], mat_1[2][3]]
[float [4] : mat_1[3][0], mat_1[3][1], mat_1[3][2], mat_1[3][3]]

...
```

Note : Collada animation binary formats don't exist yet because animations are like super hard for some reason and i can't decide how i want the data formatted

## License
[MIT](https://choosealicense.com/licenses/mit/)

## Additional Notes
- The only part of the code that is not cross platform is the "read_entire_file" function. To port to linux, either write a linux version of this one function, or use a cross-platform standard library solution like a monster.
- Binary formats are little endian