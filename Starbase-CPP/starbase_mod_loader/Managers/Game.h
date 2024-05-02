#pragma once
#include <string>

class Game
{
public:
	static char* BasePath;

	static bool Initialize();
	static bool SetupPaths();
};