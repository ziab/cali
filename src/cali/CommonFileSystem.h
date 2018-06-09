#pragma once
#include <string>

namespace cali
{
	static const std::string shader_folder = "shaders";

	std::string get_executable_file_directory();
	std::wstring get_executable_file_directory_w();
	std::string construct_shader_path(const std::string& shader_file_name);
}
