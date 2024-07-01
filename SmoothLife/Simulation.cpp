#include "Simulation.h"

Simulation::Simulation(const std::string& vertexShader, const std::string& fragmentShader, unsigned int resolutionX, unsigned int resolutionY)
	: vertp{vertexShader}, fragp{fragmentShader}, resX{resolutionX}, resY{resolutionY}
{}
