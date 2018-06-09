#include "CommonFileSystem.h"

#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

namespace cali
{
	std::string get_executable_file_directory()
	{
		char module_dir_path[MAX_PATH + 1] = {};
		DWORD ec = GetModuleFileNameA(NULL, module_dir_path, MAX_PATH);
		if (ec)
			PathRemoveFileSpecA(module_dir_path);
		else
		{
			return "";
		}

		return module_dir_path;
	}

	std::wstring get_executable_file_directory_w()
	{
		wchar_t module_dir_path[MAX_PATH + 1] = {};
		DWORD ec = GetModuleFileNameW(NULL, module_dir_path, MAX_PATH);
		if (ec)
			PathRemoveFileSpecW(module_dir_path);
		else
		{
			return L"";
		}

		return module_dir_path;
	}

	std::string construct_shader_path(const std::string & shader_file_name)
	{
		return get_executable_file_directory() + "\\" + shader_folder + "\\" + shader_file_name;
	}
}