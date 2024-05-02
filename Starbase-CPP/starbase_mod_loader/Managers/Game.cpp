#include <Windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include "Game.h"
#include "../Core.h"
#include "../Utils/Assertion.h"
#include "../Utils/Logging/Logger.h"
#include "../Utils/Encoding.h"
#pragma comment(lib,"version.lib")

char* Game::BasePath = NULL;

bool Game::Initialize()
{
	if (!SetupPaths())
	{
		Assertion::ThrowInternalFailure("Failed to Setup Game Paths!");
		return false;
	}

	return true;
}

bool Game::SetupPaths()
{
	LPSTR filepathstr = new CHAR[MAX_PATH];
	HMODULE exe_module = GetModuleHandleA(NULL);
	GetModuleFileNameA(exe_module, filepathstr, MAX_PATH);
	std::string filepath = filepathstr;
	delete[] filepathstr;

	std::string BasePathStr = filepath.substr(0, filepath.find_last_of("\\/"));
	BasePath = new char[BasePathStr.size() + 1];
	std::copy(BasePathStr.begin(), BasePathStr.end(), BasePath);
	BasePath[BasePathStr.size()] = '\0';

	return true;
}