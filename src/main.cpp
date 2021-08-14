#include "wavefront_converter.h"
#include "collada_converter.h"
#include "wav_converter.h"

int main()
{
	//convert_wavefront("examples/cube.obj", NULL, "demo_cube_out.txt");
	//convert_collada_anim("examples/character_running.dae", "test.mesh_anim", "demo_collada_out.txt");
	//convert_collada_anim("test.dae", "enemy.mesh_anim", "anim_mesh.txt");
	//convert_collada_anim("sniper.dae", "sniper.mesh_anim", "anim_mesh.txt", "sniper.anim");
	convert_collada_anim_uv("pistoluv.dae", "pistol.mesh_anim", "anim_mesh.txt", "pistol.anim");

	//convert_wavefront("b.obj" , "billboard.mesh_uv", NULL);
	//convert_wav("orb_pickup.wav", "orb.audio");

	//print_wav("test.wav");
	//convert_wav("test.wav", "test.audio");

	return 0;
}

// README for converting collada animtions
// make sure your armature is named "Armature" (case sensitive)