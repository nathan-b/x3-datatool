#pragma once

#include <string>

enum operation_type {
	INVALID_OPERATION,
	DECODE_FILE,
	DUMP_INDEX,
	EXTRACT_FILE,
	EXTRACT_ARCHIVE,
	EXTRACT_ALL,
	BUILD_PACKAGE,
	SEARCH,
};

enum option_type {
	INVALID_OPTION,
	OUT_PATH,
	IN_PATH,
	PACKAGE_FILE,
};

class operation {
public:
	bool parse(int argc, char** argv);

	operation_type get_type() const { return m_type; }

	/** internal filename => the filename of the file inside the .dat container */
	const std::string& get_internal_filename() const { return m_cat_filename; }
	/** src filename => the filename in the local filesystem to use as the source for an operation */
	const std::string& get_src_filename() const { return m_src_filename; }
	/** dest path => path to directory or file to use as the destination of an operation */
	const std::string& get_dest_path() const { return m_dst_path; }
	/** input filename => the .cat file containing the catalog for the container */
	const std::string& get_input_filename() const { return m_input_filename; }

private:
	operation_type m_type;
	std::string m_cat_filename;
	std::string m_src_filename;
	std::string m_dst_path;
	std::string m_input_filename;
};
