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
  datafile& search(const std::string& filename, bool strict_match = false) const;

	// Getters for testing
	size_t size() const { return m_dir_idx.size(); }
	bool has_id(uint32_t id) const { return m_dir_idx.find(id) != m_dir_idx.end(); }

private:
  uint32_t get_id_from_filename(const std::string& filename) const;

	std::map<std::string, uint32_t> m_name_map;
	std::map<uint32_t, datafile> m_dir_idx;

  uint32_t m_largest_id;

	// Friend class for testing private methods
	friend class datadir_test_accessor;
};
