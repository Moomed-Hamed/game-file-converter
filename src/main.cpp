#include "wavefront_converter.h"
#include "collada_converter.h"

int main()
{
	convert_wavefront("examples/cube.obj", NULL, "demo_cube_out.txt");
	convert_collada_anim("examples/character_running.dae", NULL, "demo_collada_out.txt");

	return 0;
}