#include "datafile.h"

#include <fstream>
#include <string>
#include <list>
#include <gtest/gtest.h>

static std::string slurp_file(const std::string& filename) {
	std::ifstream infile(filename);

	if (!infile) {
		return "";
	}

	// Get the file length
	infile.seekg(0, std::ios::end);
	int len = infile.tellg();

	// Allocate the buffer
	char* inbuf = new char[len + 1];

	// Read the file
	infile.seekg(0, std::ios::beg);
	infile.read(inbuf, len);
	infile.close();
	inbuf[len] = '\0';

	std::string out(inbuf);
	delete[] inbuf;
	return out;
}

TEST(datafile_tests, parse) {
	datafile df;

	ASSERT_TRUE(df.parse("test.cat"));

	ASSERT_EQ("test.dat", df.get_datfile_name());

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

TEST(datafile_tests, listing) {
	datafile df("test.cat");

	const std::string expected(
		"test.cat\n"
		"\totherdir/testfile.ext                                                    512\n"
		"\totherdir/zzz has spaces                                                   64\n"
		"\tspaces in dir/spaces in file                                              16\n"
		"\ttestdir/testfile.ext                                                    1024\n"
		"\ttestdir/testfile2.ext                                                    256\n"
		"\ttestdir/testfile3.new                                                      1\n");
	ASSERT_EQ(expected, df.get_index_listing());
}

TEST(datafile_tests, decrypt) {
	datafile df("test.cat");

	const std::string expected("test.dat\n"
							   "otherdir/testfile.ext 512\n"
							   "otherdir/zzz has spaces 64\n"
							   "spaces in dir/spaces in file 16\n"
							   "testdir/testfile.ext 1024\n"
							   "testdir/testfile2.ext 256\n"
							   "testdir/testfile3.new 1\n");

	df.decrypt_to_file("testcat.out");
	ASSERT_EQ(expected, slurp_file("testcat.out"));
}
