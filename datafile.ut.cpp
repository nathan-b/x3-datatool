#include "datafile.h"
#include "test_utils.h"

#include <fstream>
#include <string>
#include <list>
#include <filesystem>
#include <gtest/gtest.h>

static const std::string TEST_DIR = "test";
static const std::string TEST_ARTIFACTS_DIR = "test_artifacts";
static const std::string TEST_CAT = TEST_ARTIFACTS_DIR + "/test.cat";

// Setup function to create and clean test directory
static void setup_test_dir() {
	// Remove test directory if it exists
	std::error_code ec;
	std::filesystem::remove_all(TEST_DIR, ec);

	// Create fresh test directory
	std::filesystem::create_directories(TEST_DIR);

	// Create directory with spaces for testing
	std::filesystem::path spaces_dir = std::filesystem::path(TEST_DIR) / "path with spaces";
	std::filesystem::create_directories(spaces_dir);

	// Copy test files to the spaces directory
	std::filesystem::copy(TEST_CAT, spaces_dir / "test.cat", ec);
	std::filesystem::copy(TEST_ARTIFACTS_DIR + "/test.dat", spaces_dir / "test.dat", ec);
}

// Cleanup function to remove test directory
static void cleanup_test_dir() {
	std::error_code ec;
	std::filesystem::remove_all(TEST_DIR, ec);
}


// Test fixture that sets up and tears down the test directory
class datafile_tests : public ::testing::Test {
protected:
	void SetUp() override { setup_test_dir(); }

	void TearDown() override { cleanup_test_dir(); }
};

TEST_F(datafile_tests, parse) {
	datafile df;

	ASSERT_TRUE(df.parse(TEST_CAT));

	ASSERT_EQ("test_artifacts/test.dat", df.get_datfile_name());

	auto list = df.get_file_list();

	ASSERT_EQ((long unsigned)6, list.size());
	ASSERT_EQ("otherdir/testfile.ext", list.front());
	list.pop_front();
	ASSERT_EQ("otherdir/zzz has spaces", list.front());
	list.pop_front();
	ASSERT_EQ("spaces in dir/spaces in file", list.front());
	list.pop_front();
	ASSERT_EQ("testdir/testfile.ext", list.front());
	list.pop_front();
	ASSERT_EQ("testdir/testfile2.ext", list.front());
	list.pop_front();
	ASSERT_EQ("testdir/testfile3.new", list.front());
}

TEST_F(datafile_tests, parse_path_with_spaces) {
	datafile df;
	std::filesystem::path testdir(TEST_DIR);

	ASSERT_TRUE(df.parse(testdir / "path with spaces" / "test.cat"));

	// The datfile path should match the location where we copied it
	ASSERT_EQ("test/path with spaces/test.dat", df.get_datfile_name());

	auto list = df.get_file_list();

	ASSERT_EQ((long unsigned)6, list.size());
	ASSERT_EQ("otherdir/testfile.ext", list.front());
	list.pop_front();
	ASSERT_EQ("otherdir/zzz has spaces", list.front());
	list.pop_front();
	ASSERT_EQ("spaces in dir/spaces in file", list.front());
	list.pop_front();
	ASSERT_EQ("testdir/testfile.ext", list.front());
	list.pop_front();
	ASSERT_EQ("testdir/testfile2.ext", list.front());
	list.pop_front();
	ASSERT_EQ("testdir/testfile3.new", list.front());
}

TEST_F(datafile_tests, listing) {
	datafile df(TEST_CAT);

	const std::string expected("test_artifacts/test.cat\n"
	                           "\totherdir/testfile.ext                                                    512\n"
	                           "\totherdir/zzz has spaces                                                   64\n"
	                           "\tspaces in dir/spaces in file                                              16\n"
	                           "\ttestdir/testfile.ext                                                    1024\n"
	                           "\ttestdir/testfile2.ext                                                    256\n"
	                           "\ttestdir/testfile3.new                                                      1\n");
	ASSERT_EQ(expected, df.get_index_listing());
}

TEST_F(datafile_tests, decrypt) {
	datafile df(TEST_CAT);

	const std::string expected("test.dat\n"
	                           "otherdir/testfile.ext 512\n"
	                           "otherdir/zzz has spaces 64\n"
	                           "spaces in dir/spaces in file 16\n"
	                           "testdir/testfile.ext 1024\n"
	                           "testdir/testfile2.ext 256\n"
	                           "testdir/testfile3.new 1\n");

	df.decrypt_to_file(TEST_DIR + "/testcat.out");
	ASSERT_EQ(expected, test_utils::read_file(TEST_DIR + "/testcat.out"));
}

TEST_F(datafile_tests, extract_by_filename_only) {
	datafile df(TEST_CAT);

	// Extract using just the filename (no path) with strict_match=false
	ASSERT_TRUE(df.extract_one_file("testfile3.new", TEST_DIR + "/test_extract_filename.txt", false));

	// Verify the content
	std::string content = test_utils::read_file(TEST_DIR + "/test_extract_filename.txt");
	ASSERT_EQ("1", content);
}

TEST_F(datafile_tests, extract_by_filename_with_spaces) {
	datafile df(TEST_CAT);

	// Extract file with spaces in name using just filename
	ASSERT_TRUE(df.extract_one_file("zzz has spaces", TEST_DIR + "/test_extract_spaces.txt", false));

	// Verify the content starts with the expected header
	std::string content = test_utils::read_file(TEST_DIR + "/test_extract_spaces.txt");
	ASSERT_EQ("512,64,", content.substr(0, 7));
}

TEST_F(datafile_tests, extract_by_filename_ambiguous) {
	datafile df(TEST_CAT);

	// testfile.ext exists in both otherdir/ and testdir/
	// With strict_match=false, it should find the first match
	ASSERT_TRUE(df.extract_one_file("testfile.ext", TEST_DIR + "/test_extract_ambiguous.txt", false));

	// Should get the first one (otherdir/testfile.ext) which has offset 0
	std::string content = test_utils::read_file(TEST_DIR + "/test_extract_ambiguous.txt");
	ASSERT_EQ("0,512,", content.substr(0, 6));
}

TEST_F(datafile_tests, extract_by_full_path) {
	datafile df(TEST_CAT);

	// Extract using full path with strict_match=true
	ASSERT_TRUE(df.extract_one_file("testdir/testfile.ext", TEST_DIR + "/test_extract_fullpath.txt", true));

	// Verify we got the right one (offset 592)
	std::string content = test_utils::read_file(TEST_DIR + "/test_extract_fullpath.txt");
	ASSERT_EQ("592,1024,", content.substr(0, 9));
}

TEST_F(datafile_tests, extract_by_full_path_not_found) {
	datafile df(TEST_CAT);

	// Try to extract with full path that doesn't exist
	ASSERT_FALSE(df.extract_one_file("wrongdir/testfile.ext", TEST_DIR + "/test_extract_notfound.txt", true));
}

TEST_F(datafile_tests, extract_by_filename_not_found) {
	datafile df(TEST_CAT);

	// Try to extract with filename that doesn't exist
	ASSERT_FALSE(df.extract_one_file("nonexistent.txt", TEST_DIR + "/test_extract_notfound2.txt", false));
}

TEST_F(datafile_tests, extract_archive) {
	datafile df(TEST_CAT);

	// Extract entire archive to a directory
	std::string extract_dir = TEST_DIR + "/test_extract_all";
	ASSERT_TRUE(df.extract(extract_dir));

	// Verify files were created with correct content
	ASSERT_EQ("1", test_utils::read_file(extract_dir + "/testdir/testfile3.new"));

	// Verify another file
	std::string content = test_utils::read_file(extract_dir + "/otherdir/testfile.ext");
	ASSERT_EQ("0,512,", content.substr(0, 6));

	// Verify file with spaces
	content = test_utils::read_file(extract_dir + "/spaces in dir/spaces in file");
	ASSERT_EQ("576,16,", content.substr(0, 7));
}

TEST_F(datafile_tests, build_and_parse) {
	// Create a test directory structure
	std::string build_dir = TEST_DIR + "/test_build_src";
	std::string subdir = build_dir + "/subdir";
	std::filesystem::create_directories(subdir);

	// Create test files with absolute paths
	{
		std::ofstream f1(build_dir + "/file1.txt");
		f1 << "Hello World";
		f1.close();
	}
	{
		std::ofstream f2(subdir + "/file2.txt");
		f2 << "Test Content";
		f2.close();
	}

	// Build the archive from the test directory
	datafile builder;
	ASSERT_TRUE(builder.build(build_dir, TEST_DIR + "/test_build.cat"));

	// Verify the .cat and .dat files were created
	ASSERT_TRUE(std::filesystem::exists(TEST_DIR + "/test_build.cat"));
	ASSERT_TRUE(std::filesystem::exists(TEST_DIR + "/test_build.dat"));

	// Parse the created archive
	datafile parser(TEST_DIR + "/test_build.cat");
	auto files = parser.get_file_list();

	// Verify the file list (should have 2 files)
	ASSERT_EQ(2u, files.size());

	// Extract and verify content
	ASSERT_TRUE(parser.extract_one_file("file1.txt", TEST_DIR + "/test_build_extract1.txt", false));
	ASSERT_EQ("Hello World", test_utils::read_file(TEST_DIR + "/test_build_extract1.txt"));

	ASSERT_TRUE(parser.extract_one_file("file2.txt", TEST_DIR + "/test_build_extract2.txt", false));
	ASSERT_EQ("Test Content", test_utils::read_file(TEST_DIR + "/test_build_extract2.txt"));
}

TEST_F(datafile_tests, build_empty_directory) {
	// Create an empty directory
	std::string empty_dir = TEST_DIR + "/test_empty_dir";
	std::filesystem::create_directories(empty_dir);

	// Try to build from empty directory
	datafile builder;
	ASSERT_TRUE(builder.build(empty_dir, TEST_DIR + "/test_empty.cat"));

	// Parse and verify it has no files
	datafile parser(TEST_DIR + "/test_empty.cat");
	auto files = parser.get_file_list();
	ASSERT_EQ(0u, files.size());
}

TEST_F(datafile_tests, build_nonexistent_directory) {
	datafile builder;
	// Try to build from a directory that doesn't exist
	// The build function now returns false instead of throwing
	ASSERT_FALSE(builder.build("nonexistent_directory_12345", TEST_DIR + "/test_nonexist.cat"));
}

TEST_F(datafile_tests, parse_nonexistent_file) {
	datafile df;
	// Try to parse a file that doesn't exist
	ASSERT_FALSE(df.parse("nonexistent_file_12345.cat"));
}

TEST_F(datafile_tests, decrypt_to_file) {
	datafile df(TEST_CAT);

	// Decrypt to a file
	ASSERT_TRUE(df.decrypt_to_file(TEST_DIR + "/test_decrypt_output.txt"));

	// Verify the decrypted content
	std::string content = test_utils::read_file(TEST_DIR + "/test_decrypt_output.txt");
	ASSERT_TRUE(content.find("test.dat") != std::string::npos);
	ASSERT_TRUE(content.find("otherdir/testfile.ext 512") != std::string::npos);
	ASSERT_TRUE(content.find("testdir/testfile3.new 1") != std::string::npos);
}
