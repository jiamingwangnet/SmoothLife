#include "Simulation.h"

int main()
{
	Simulation sim{ "./shaders/default.vert", "./shaders/simulation.frag", "./shaders/passthrough.frag", "./shaders/brush.frag", 1280, 720, 1280, 720};

	sim.Init();

	sim.MainLoop();

	return 0;
}