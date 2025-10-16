#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "datadir.h"
#include "datafile.h"
#include "operation.h"
#include "../grep-bin/buffer.h"

struct catfile {
	std::string filename;
	std::list<std::string> file_list;
};

void save_file(const std::string& filename, const buffer& buf) {
	std::ofstream outfile(filename, std::ios::out | std::ios::binary);

	if (!outfile) {
		std::cerr << "Could not save file " << filename << "!\n";
		return;
	}

	outfile.write((char*)&buf[0], buf.length());
	outfile.close();
}

bool dump_index(const datafile& idx) {
	std::cout << idx.get_index_listing() << std::endl;
	return true;
}

bool decode_file(const datafile& idx, const std::filesystem::path& outpath) {
	return idx.decrypt_to_file(outpath);
}

bool extract_file(const datafile& idx, const std::string& filename, const std::filesystem::path& outfilename) {
	std::filesystem::path actual_outfilename = outfilename;
	if (actual_outfilename.empty()) {
		// Use the filename we're extracting as the output filename
		actual_outfilename = filename;
	}
	return idx.extract_one_file(filename, actual_outfilename, true);
}

bool extract_archive(const datafile& idx, const std::filesystem::path& outpath) {
	return idx.extract(outpath);
}

bool extract_all(const std::string& inpath, const std::string& outpath) {
	// XXX TODO
	return false;
}

bool build_package(const std::filesystem::path& cat_filename, const std::filesystem::path& src_path) {
	std::filesystem::path p(src_path);

	if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p)) {
		std::cerr << p << " does not exist or is not a directory" << std::endl;
		return false;
	}

	datafile idx;
	return idx.build(p, cat_filename);
}

bool search(const std::filesystem::path& inpath, const std::filesystem::path& needle) {
  datadir search_dir(inpath.string());

  datafile* ret = search_dir.search(needle.string(), false);

  if (ret) {
    std::cout << "The file " << needle << " is most recently found in " << ret->get_catfile_name() << "\n";
    return true;
  }
  std::cout << "The file " << needle << " was not found in any catalog in " << inpath << "\n";
  return true; // Still technically a successful operation
}

static void usage() {
	std::cout << "Usage: x3tool <operation> <cat_file> [options]\n"
			  << "  Valid operations: t / dump-index             Print the index of the package file\n"
			  << "                    d / decode-file  [-o output-path]  Decode cat file to the given "
				 "path (or current directory)\n"
			  << "                    f / extract-file <-f filename> <-o output-file>  Extract the "
				 "contents of a single file to disk\n"
			  << "                    x / extract-archive  [-o output-path]  Extract one entire archive "
				 "to the output path (or current directory)\n"
			  << "                    p / build-package <-i input-path>  Build a new vp file with the "
				 "contents of input-path\n"
        << "                    a / extract-all <-i input-path> <-o output-path>  Extract every archive in the "
				 "provided directory to the output path\n"
        << "                    s / search <-f filename>  <-i search-directory> Find the most recent "
        << "cat file in the provided directory which contains the given file\n";
}

int main(int argc, char** argv) {
	operation op;

	if (!op.parse(argc, argv)) {
		std::cerr << "Command line input error\n";
		usage();
		return -1;
	}

	if (op.get_type() == BUILD_PACKAGE) {
		// Since the arguments can be a little confusing, if the user did not specify
		// an file but did specify an output file, we know what to do
		std::filesystem::path catfile = op.get_input_filename();
		if (catfile.empty() && !op.get_dest_path().empty()) {
			catfile = op.get_dest_path();
		}

		if (catfile.empty()) {
			std::cerr << "You must specify a filename for the new .cat file\n";
			usage();
			return -1;
		}
		// Build package operations don't parse an index file beforehand
		if (!build_package(catfile, op.get_src_filename())) {
			std::cerr << "Error building package " << catfile << std::endl;
			return -2;
		}
		std::cout << "Success!\n";
		return 0;
	}

	// Parse the index file
	datafile df;

	if (!df.parse(op.get_input_filename())) {
		std::cerr << "Could not read .cat file " << op.get_input_filename() << std::endl;
		return -1;
	}

	bool ret = false;
	switch (op.get_type()) {
  case SEARCH:
    ret = search(op.get_input_filename(), op.get_src_filename());
    break;
	case DUMP_INDEX:
		ret = dump_index(df);
		break;
	case DECODE_FILE: {
		std::filesystem::path outfilename = op.get_dest_path();
		if (outfilename.empty()) {
			outfilename = op.get_input_filename().string() + ".decoded";
		}
		ret = decode_file(df, outfilename);
		if (ret) {
			std::cout << "Decoded " << op.get_input_filename() << " to " << outfilename << std::endl;
		}
	} break;
	case EXTRACT_FILE:
		ret = extract_file(df, op.get_internal_filename(), op.get_dest_path());
		break;
	case EXTRACT_ARCHIVE: {
		std::filesystem::path outpath = op.get_dest_path();
		if (outpath.empty()) {
			outpath = ".";
		}
		ret = extract_archive(df, outpath);
	} break;
	case EXTRACT_ALL:
		// ret = extract_all(df, op.get_dest_path());
		break;
	default:
		return -1;
	}

	if (!ret) {
		std::cerr << "Operation did not complete successfully!\n";
    return 1;
	}
  return 0;
}
