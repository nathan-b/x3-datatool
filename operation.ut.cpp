#include "operation.h"

#include <gtest/gtest.h>
#include <vector>
#include <string>

// Helper to convert vector of strings to argc/argv format
class ArgvHelper {
public:
	ArgvHelper(const std::vector<std::string>& args) {
		m_argc = args.size();
		m_argv = new char*[m_argc];
		for (size_t i = 0; i < args.size(); ++i) {
			m_argv[i] = new char[args[i].length() + 1];
			strcpy(m_argv[i], args[i].c_str());
		}
	}

	~ArgvHelper() {
		for (int i = 0; i < m_argc; ++i) {
			delete[] m_argv[i];
		}
		delete[] m_argv;
	}

	int argc() const { return m_argc; }
	char** argv() { return m_argv; }

private:
	int m_argc;
	char** m_argv;
};

// Test short operation names
TEST(operation_tests, short_dump_index) {
	ArgvHelper args({"x3tool", "t", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(DUMP_INDEX, op.get_type());
	ASSERT_EQ("test.cat", op.get_input_filename());
}

TEST(operation_tests, short_decode_file) {
	ArgvHelper args({"x3tool", "d", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(DECODE_FILE, op.get_type());
	ASSERT_EQ("test.cat", op.get_input_filename());
}

TEST(operation_tests, short_extract_file) {
	ArgvHelper args({"x3tool", "f", "test.cat", "-f", "internal/file.txt", "-o", "output.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(EXTRACT_FILE, op.get_type());
	ASSERT_EQ("test.cat", op.get_input_filename());
	ASSERT_EQ("internal/file.txt", op.get_internal_filename());
	ASSERT_EQ("output.txt", op.get_dest_path());
}

TEST(operation_tests, short_extract_archive) {
	ArgvHelper args({"x3tool", "x", "test.cat", "-o", "output_dir"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(EXTRACT_ARCHIVE, op.get_type());
	ASSERT_EQ("test.cat", op.get_input_filename());
	ASSERT_EQ("output_dir", op.get_dest_path());
}

TEST(operation_tests, short_extract_all) {
	ArgvHelper args({"x3tool", "a", "input_dir", "-o", "output_dir"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(EXTRACT_ALL, op.get_type());
	ASSERT_EQ("input_dir", op.get_input_filename());
	ASSERT_EQ("output_dir", op.get_dest_path());
}

TEST(operation_tests, short_replace_file) {
	ArgvHelper args({"x3tool", "r", "test.cat", "-f", "internal/file.txt", "-i", "newfile.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(REPLACE_FILE, op.get_type());
	ASSERT_EQ("test.cat", op.get_input_filename());
	ASSERT_EQ("internal/file.txt", op.get_internal_filename());
	ASSERT_EQ("newfile.txt", op.get_src_filename());
}

TEST(operation_tests, short_build_package) {
	ArgvHelper args({"x3tool", "p", "-i", "source_dir", "-o", "output.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(BUILD_PACKAGE, op.get_type());
	ASSERT_EQ("source_dir", op.get_src_filename());
	ASSERT_EQ("output.cat", op.get_dest_path());
}

TEST(operation_tests, short_build_package_c_alias) {
	ArgvHelper args({"x3tool", "c", "-i", "source_dir", "-o", "output.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(BUILD_PACKAGE, op.get_type());
}

TEST(operation_tests, short_search) {
	ArgvHelper args({"x3tool", "s", "search_dir", "-f", "needle.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(SEARCH, op.get_type());
	ASSERT_EQ("search_dir", op.get_input_filename());
	ASSERT_EQ("needle.txt", op.get_internal_filename());
}

// Test long operation names with underscores
TEST(operation_tests, long_dump_index_underscore) {
	ArgvHelper args({"x3tool", "dump_index", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(DUMP_INDEX, op.get_type());
}

TEST(operation_tests, long_decode_file_underscore) {
	ArgvHelper args({"x3tool", "decode_file", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(DECODE_FILE, op.get_type());
}

TEST(operation_tests, long_extract_file_underscore) {
	ArgvHelper args({"x3tool", "extract_file", "test.cat", "-f", "file.txt", "-o", "out.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(EXTRACT_FILE, op.get_type());
}

TEST(operation_tests, long_extract_archive_underscore) {
	ArgvHelper args({"x3tool", "extract_archive", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(EXTRACT_ARCHIVE, op.get_type());
}

TEST(operation_tests, long_extract_all_underscore) {
	ArgvHelper args({"x3tool", "extract_all", "dir"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(EXTRACT_ALL, op.get_type());
}

TEST(operation_tests, long_replace_file_underscore) {
	ArgvHelper args({"x3tool", "replace_file", "test.cat", "-f", "file.txt", "-i", "new.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(REPLACE_FILE, op.get_type());
}

TEST(operation_tests, long_build_package_underscore) {
	ArgvHelper args({"x3tool", "build_package", "-i", "source_dir"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(BUILD_PACKAGE, op.get_type());
}

// Test long operation names with hyphens
TEST(operation_tests, long_dump_index_hyphen) {
	ArgvHelper args({"x3tool", "dump-index", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(DUMP_INDEX, op.get_type());
}

TEST(operation_tests, long_decode_file_hyphen) {
	ArgvHelper args({"x3tool", "decode-file", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(DECODE_FILE, op.get_type());
}

TEST(operation_tests, long_extract_file_hyphen) {
	ArgvHelper args({"x3tool", "extract-file", "test.cat", "-f", "file.txt", "-o", "out.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(EXTRACT_FILE, op.get_type());
}

TEST(operation_tests, long_extract_archive_hyphen) {
	ArgvHelper args({"x3tool", "extract-archive", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(EXTRACT_ARCHIVE, op.get_type());
}

TEST(operation_tests, long_extract_all_hyphen) {
	ArgvHelper args({"x3tool", "extract-all", "dir"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(EXTRACT_ALL, op.get_type());
}

TEST(operation_tests, long_replace_file_hyphen) {
	ArgvHelper args({"x3tool", "replace-file", "test.cat", "-f", "file.txt", "-i", "new.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(REPLACE_FILE, op.get_type());
}

TEST(operation_tests, long_build_package_hyphen) {
	ArgvHelper args({"x3tool", "build-package", "-i", "source_dir"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(BUILD_PACKAGE, op.get_type());
}

// Test option parsing
TEST(operation_tests, output_path_short) {
	ArgvHelper args({"x3tool", "d", "test.cat", "-o", "decoded.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("decoded.txt", op.get_dest_path());
}

TEST(operation_tests, output_path_long) {
	ArgvHelper args({"x3tool", "d", "test.cat", "--output-path", "decoded.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("decoded.txt", op.get_dest_path());
}

TEST(operation_tests, input_file_short) {
	ArgvHelper args({"x3tool", "p", "-i", "source_dir", "-o", "output.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("source_dir", op.get_src_filename());
}

TEST(operation_tests, input_file_long) {
	ArgvHelper args({"x3tool", "p", "--input-file", "source_dir", "-o", "output.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("source_dir", op.get_src_filename());
}

TEST(operation_tests, package_file_short) {
	ArgvHelper args({"x3tool", "f", "test.cat", "-f", "internal.txt", "-o", "out.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("internal.txt", op.get_internal_filename());
}

TEST(operation_tests, package_file_long) {
	ArgvHelper args({"x3tool", "f", "test.cat", "--package-file", "internal.txt", "-o", "out.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("internal.txt", op.get_internal_filename());
}

// Test paths with spaces
TEST(operation_tests, paths_with_spaces) {
	ArgvHelper args(
		{"x3tool", "f", "my test.cat", "-f", "path with/spaces.txt", "-o", "output file.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("my test.cat", op.get_input_filename());
	ASSERT_EQ("path with/spaces.txt", op.get_internal_filename());
	ASSERT_EQ("output file.txt", op.get_dest_path());
}

// Test failure cases
TEST(operation_tests, invalid_operation) {
	ArgvHelper args({"x3tool", "invalid", "test.cat"});
	operation op;

	ASSERT_FALSE(op.parse(args.argc(), args.argv()));
}

TEST(operation_tests, invalid_short_operation) {
	ArgvHelper args({"x3tool", "z", "test.cat"});
	operation op;

	ASSERT_FALSE(op.parse(args.argc(), args.argv()));
}

TEST(operation_tests, no_operation) {
	ArgvHelper args({"x3tool"});
	operation op;

	ASSERT_FALSE(op.parse(args.argc(), args.argv()));
}

TEST(operation_tests, invalid_option) {
	ArgvHelper args({"x3tool", "d", "test.cat", "-z", "invalid"});
	operation op;

	ASSERT_FALSE(op.parse(args.argc(), args.argv()));
}

TEST(operation_tests, multiple_input_files) {
	ArgvHelper args({"x3tool", "d", "test1.cat", "test2.cat"});
	operation op;

	ASSERT_FALSE(op.parse(args.argc(), args.argv()));
}

// Test edge cases
TEST(operation_tests, extract_archive_no_output_path) {
	ArgvHelper args({"x3tool", "x", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(EXTRACT_ARCHIVE, op.get_type());
	ASSERT_EQ("test.cat", op.get_input_filename());
	ASSERT_EQ("", op.get_dest_path()); // Empty output path should be allowed
}

TEST(operation_tests, decode_file_no_output_path) {
	ArgvHelper args({"x3tool", "d", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ(DECODE_FILE, op.get_type());
	ASSERT_EQ("", op.get_dest_path()); // Empty output path should be allowed
}

TEST(operation_tests, empty_strings) {
	ArgvHelper args({"x3tool", "t", ""});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("", op.get_input_filename());
}

// Test option ordering
TEST(operation_tests, options_before_input_file) {
	ArgvHelper args({"x3tool", "f", "-f", "internal.txt", "-o", "out.txt", "test.cat"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("test.cat", op.get_input_filename());
	ASSERT_EQ("internal.txt", op.get_internal_filename());
	ASSERT_EQ("out.txt", op.get_dest_path());
}

TEST(operation_tests, options_after_input_file) {
	ArgvHelper args({"x3tool", "f", "test.cat", "-f", "internal.txt", "-o", "out.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("test.cat", op.get_input_filename());
	ASSERT_EQ("internal.txt", op.get_internal_filename());
	ASSERT_EQ("out.txt", op.get_dest_path());
}

TEST(operation_tests, mixed_option_ordering) {
	ArgvHelper args({"x3tool", "f", "-f", "internal.txt", "test.cat", "-o", "out.txt"});
	operation op;

	ASSERT_TRUE(op.parse(args.argc(), args.argv()));
	ASSERT_EQ("test.cat", op.get_input_filename());
	ASSERT_EQ("internal.txt", op.get_internal_filename());
	ASSERT_EQ("out.txt", op.get_dest_path());
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
