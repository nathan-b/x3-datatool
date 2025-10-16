#pragma once

#include <string>
#include <fstream>
#include <filesystem>

/**
 * Shared test utilities for x3_datatool unit tests.
 */
namespace test_utils {

/**
 * Read the entire contents of a file into a string.
 * Returns an empty string if the file cannot be opened.
 */
inline std::string read_file(const std::filesystem::path& filepath) {
	std::ifstream infile(filepath);
	if (!infile) {
		return "";
	}
	return std::string((std::istreambuf_iterator<char>(infile)),
	                    std::istreambuf_iterator<char>());
}

} // namespace test_utils
