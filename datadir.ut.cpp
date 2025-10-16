#include "datadir.h"

#include <gtest/gtest.h>

// Test accessor class to access private methods
class datadir_test_accessor {
public:
	static uint32_t get_id_from_filename(const datadir& dd, const std::string& filename) {
		return dd.get_id_from_filename(filename);
	}

	static uint32_t get_largest_id(const datadir& dd) {
		return dd.m_largest_id;
	}
};

// Test fixture
class datadir_tests : public ::testing::Test {
protected:
	// We need a valid path for the constructor, even though we won't use it for these tests
	datadir dd{"."};
};

TEST_F(datadir_tests, get_id_single_digit) {
	uint32_t result = datadir_test_accessor::get_id_from_filename(dd, "1.cat");
	ASSERT_EQ(1u, result);
}

TEST_F(datadir_tests, get_id_two_digits) {
	uint32_t result = datadir_test_accessor::get_id_from_filename(dd, "42.cat");
	ASSERT_EQ(42u, result);
}

TEST_F(datadir_tests, get_id_three_digits) {
	uint32_t result = datadir_test_accessor::get_id_from_filename(dd, "123.cat");
	ASSERT_EQ(123u, result);
}

TEST_F(datadir_tests, get_id_with_full_path) {
	uint32_t result = datadir_test_accessor::get_id_from_filename(dd, "/path/to/5.cat");
	ASSERT_EQ(5u, result);
}

TEST_F(datadir_tests, get_id_with_relative_path) {
	uint32_t result = datadir_test_accessor::get_id_from_filename(dd, "./10.cat");
	ASSERT_EQ(10u, result);
}

TEST_F(datadir_tests, get_id_zero) {
	uint32_t result = datadir_test_accessor::get_id_from_filename(dd, "0.cat");
	ASSERT_EQ(0u, result);
}

TEST_F(datadir_tests, get_id_large_number) {
	uint32_t result = datadir_test_accessor::get_id_from_filename(dd, "99999.cat");
	ASSERT_EQ(99999u, result);
}

TEST_F(datadir_tests, get_id_without_extension) {
	// Test that it works even without .cat extension
	uint32_t result = datadir_test_accessor::get_id_from_filename(dd, "15");
	ASSERT_EQ(15u, result);
}

TEST_F(datadir_tests, add_by_path_success) {
	// Add a valid datafile by path
	bool result = dd.add("test_artifacts/1.cat");
	ASSERT_TRUE(result);
	ASSERT_EQ(1u, datadir_test_accessor::get_largest_id(dd));
}

TEST_F(datadir_tests, add_by_path_nonexistent) {
	// Try to add a non-existent file
	bool result = dd.add("nonexistent_file_12345.cat");
	ASSERT_FALSE(result);
}

TEST_F(datadir_tests, add_by_datafile) {
	// Create a datafile and add it
	datafile df;
	ASSERT_TRUE(df.parse("test_artifacts/2.cat"));

	bool result = dd.add(df);
	ASSERT_TRUE(result);
	ASSERT_EQ(2u, datadir_test_accessor::get_largest_id(dd));
}

TEST_F(datadir_tests, add_multiple_files) {
	// Add multiple files with different IDs
	ASSERT_TRUE(dd.add("test_artifacts/1.cat"));
	ASSERT_TRUE(dd.add("test_artifacts/2.cat"));
	ASSERT_TRUE(dd.add("test_artifacts/10.cat"));
}

TEST_F(datadir_tests, constructor_loads_directory) {
	// Create a datadir pointing to the composite test directory
	datadir composite_dd{"test_artifacts/composite"};

	// Should have loaded 3 cat files (1.cat, 2.cat, 10.cat)
	ASSERT_EQ(3u, composite_dd.size());

	// Verify each ID is present
	ASSERT_TRUE(composite_dd.has_id(1));
	ASSERT_TRUE(composite_dd.has_id(2));
	ASSERT_TRUE(composite_dd.has_id(10));
	ASSERT_EQ(10u, datadir_test_accessor::get_largest_id(composite_dd));
}

TEST_F(datadir_tests, constructor_empty_directory) {
	// Create a datadir pointing to a directory with no cat files
	datadir empty_dd{"test"};

	// Should have loaded 0 files
	ASSERT_EQ(0u, empty_dd.size());
	ASSERT_EQ(0u, datadir_test_accessor::get_largest_id(dd));
}

TEST_F(datadir_tests, constructor_nonexistent_directory) {
	// Create a datadir pointing to a non-existent directory
	datadir nonexistent_dd{"nonexistent_dir_12345"};

	// Should handle gracefully and have 0 files
	ASSERT_EQ(0u, nonexistent_dd.size());
}

TEST_F(datadir_tests, search_file_in_highest_archive) {
	// Create a datadir with the composite archives
	datadir composite_dd{"test_artifacts/composite"};

	// Search for models/ship.mdl - should find it in archive 10 (highest)
	datafile& df = composite_dd.search("models/ship.mdl", true);
	ASSERT_NE("", df.get_catfile_name());
	ASSERT_TRUE(df.get_catfile_name().find("10.cat") != std::string::npos);
}

TEST_F(datadir_tests, search_file_in_middle_archive) {
	// Create a datadir with the composite archives
	datadir composite_dd{"test_artifacts/composite"};

	// Search for models/station.mdl - should find it in archive 2 (not in 10)
	datafile& df = composite_dd.search("models/station.mdl", true);
	ASSERT_NE("", df.get_catfile_name());
	ASSERT_TRUE(df.get_catfile_name().find("2.cat") != std::string::npos);
}

TEST_F(datadir_tests, search_file_in_lowest_archive) {
	// Create a datadir with the composite archives
	datadir composite_dd{"test_artifacts/composite"};

	// Search for scripts/main.lua - should find it in archive 1 (only there)
	datafile& df = composite_dd.search("scripts/main.lua", true);
	ASSERT_NE("", df.get_catfile_name());
	ASSERT_TRUE(df.get_catfile_name().find("1.cat") != std::string::npos);
}

TEST_F(datadir_tests, search_file_not_found) {
	// Create a datadir with the composite archives
	datadir composite_dd{"test_artifacts/composite"};

	// Search for a file that doesn't exist
	datafile& df = composite_dd.search("nonexistent/file.txt", true);
	ASSERT_EQ("", df.get_catfile_name());
}

TEST_F(datadir_tests, search_by_filename_only) {
	// Create a datadir with the composite archives
	datadir composite_dd{"test_artifacts/composite"};

	// Search for ship.mdl by filename only (not strict) - should find in archive 10
	datafile& df = composite_dd.search("ship.mdl", false);
	ASSERT_NE("", df.get_catfile_name());
	ASSERT_TRUE(df.get_catfile_name().find("10.cat") != std::string::npos);
}

TEST_F(datadir_tests, search_filename_only_not_found) {
	// Create a datadir with the composite archives
	datadir composite_dd{"test_artifacts/composite"};

	// Search for a filename that doesn't exist
	datafile& df = composite_dd.search("nonexistent.txt", false);
	ASSERT_EQ("", df.get_catfile_name());
}

TEST_F(datadir_tests, search_complete_composite_precedence) {
	// Create a datadir with the composite archives
	datadir composite_dd{"test_artifacts/composite"};

	// Test all 8 expected files from the composite README
	// Files from archive 10 (highest precedence)
	ASSERT_TRUE(composite_dd.search("models/ship.mdl", true).get_catfile_name().find("10.cat") != std::string::npos);
	ASSERT_TRUE(composite_dd.search("textures/hull.tex", true).get_catfile_name().find("10.cat") != std::string::npos);
	ASSERT_TRUE(composite_dd.search("sounds/engine.wav", true).get_catfile_name().find("10.cat") != std::string::npos);

	// Files from archive 2 (middle precedence)
	ASSERT_TRUE(composite_dd.search("models/station.mdl", true).get_catfile_name().find("2.cat") != std::string::npos);
	ASSERT_TRUE(composite_dd.search("scripts/init.lua", true).get_catfile_name().find("2.cat") != std::string::npos);
	ASSERT_TRUE(composite_dd.search("sounds/weapons.wav", true).get_catfile_name().find("2.cat") != std::string::npos);

	// Files from archive 1 (lowest precedence, only in archive 1)
	ASSERT_TRUE(composite_dd.search("scripts/main.lua", true).get_catfile_name().find("1.cat") != std::string::npos);
	ASSERT_TRUE(composite_dd.search("textures/cockpit.tex", true).get_catfile_name().find("1.cat") != std::string::npos);
}
