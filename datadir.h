#pragma once

#include <cstdint>
#include <string>
#include <map>

#include "datafile.h"

class datadir {
public:
	datadir(const std::string& path);

	bool add(const std::string& datafile_path);
	bool add(datafile& file);
	datafile& search(const std::string& filename);

private:
	uint32_t get_id_from_filename(const std::string& filename);

	std::map<std::string, uint32_t> m_name_map;
	std::map<uint32_t, datafile> m_dir_idx;
};
