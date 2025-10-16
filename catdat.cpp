#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <list>

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

bool decode_file(const datafile& idx, const std::string& outpath) {
	return idx.decrypt_to_file(outpath);
}

bool extract_file(const datafile& idx, const std::string& filename, const std::string& outfilename) {
	std::string actual_outfilename = outfilename;
	if (actual_outfilename.empty()) {
		// Use the filename we're extracting as the output filename
		actual_outfilename = filename;
	}
	return idx.extract_one_file(filename, actual_outfilename, true);
}

bool extract_archive(const datafile& idx, const std::string& outpath) {
	return idx.extract(outpath);
}

bool extract_all(const std::string& inpath, const std::string& outpath) {
	// XXX TODO
	return false;
}

bool build_package(const std::string& cat_filename, const std::string& src_path) {
	std::filesystem::path p(src_path);

	if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p)) {
		std::cerr << p << " does not exist or is not a directory" << std::endl;
		return false;
	}

	datafile idx;
	return idx.build(p, cat_filename);
}

bool search(const std::string& inpath, const std::string& needle) {
	// XXX TODO
	return false;
}

bool replace_file(datafile& idx, const std::string& filename, const std::string& infilename) {
#if 0
	// There's a sneaky optimization we can use here: if the updated file is
	// the same size or smaller than the original, we can just overwrite the file
	// data inside the package and update the size in the index. That potentially
	// results in a bit of wastage in the file data segment, but no big deal.
	vp_file* currfile = idx->find(filename);
	if (!currfile) {
		std::cerr << "Could not find " << filename << " in package!\n";
		return false;
	}

	std::filesystem::directory_entry direntry(infilename);

	if (currfile->get_size() >= direntry.file_size()) {
		// Update the file
		if (!currfile->write_file_contents(direntry.path())) {
			std::cerr << "Could not write file contents to package for " << filename << std::endl;
			return false;
		}
		if (!idx->update_index(currfile)) {
			std::cerr << "Could not update index entry for " << filename << std::endl;
			return false;
		}
		return true;
	}

	// vp files don't tend to be massive (I'll probably regret those words at some point)
	// so for maximum reliability, just extract the whole thing, replace the file, and
	// then build the new file over top of the old.
	auto tmpd = scoped_tempdir("vptool-");
	if (!std::filesystem::is_directory(tmpd)) {
		std::cerr << "Could not create a temporary directory\n";
		return false;
	}

	if (!idx->dump(tmpd)) {
		std::cerr << "Could not dump package file to " << tmpd << std::endl;
		return false;
	}

	// Replace the file with the new file
	std::filesystem::path f = tmpd / currfile->get_path();
	if (!std::filesystem::exists(f)) {
		std::cerr << "Could not find " << filename << " at path " << f << std::endl;
		return false;
	}
	if (!std::filesystem::copy_file(infilename, f, std::filesystem::copy_options::overwrite_existing)) {
		std::cerr << "Could not copy " << infilename << " over " << f << std::endl;
		return false;
	}

	// Repackage the whole dealio
	return build_package(idx->get_filename(), tmpd);
#endif
	return false;
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
			  << "                    r / replace-file <-f filename> <-i input-file>  Replace the "
				 "contents of a single file\n"
			  << "                    p / build-package <-i input-path>  Build a new vp file with the "
				 "contents of input-path\n"
			  << "                    a / extract-all [-o output-path]  Extract every archive in the "
				 "provided directory to the output path (or current directory)\n"
			  << "                    s / search <-f filename>  Find the most recent cat file in the "
				 "provided directory which contains the given file\n";
}

int main(int argc, char** argv) {
#if 0
    datafile cat(argv[1]);

    std::cout << "The data file is " << cat.m_datfile << std::endl;

    for (auto& ie : cat.m_index) {
        std::cout << ie.relpath << "     [" << ie.size << "]\n";
    }

    return 0;

#elif 1
	operation op;

	if (!op.parse(argc, argv)) {
		std::cerr << "Command line input error\n";
		usage();
		return -1;
	}

	if (op.get_type() == BUILD_PACKAGE) {
		// Since the arguments can be a little confusing, if the user did not specify
		// an file but did specify an output file, we know what to do
		std::string catfile = op.get_input_filename();
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
	case DUMP_INDEX:
		ret = dump_index(df);
		break;
	case DECODE_FILE: {
		std::string outfilename = op.get_dest_path();
		if (outfilename.empty()) {
			outfilename = op.get_input_filename() + ".decoded";
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
		std::string outpath = op.get_dest_path();
		if (outpath.empty()) {
			outpath = ".";
		}
		ret = extract_archive(df, outpath);
	} break;
	case EXTRACT_ALL:
		// ret = extract_all(df, op.get_dest_path());
		break;
	case REPLACE_FILE:
		ret = replace_file(df, op.get_internal_filename(), op.get_src_filename());
		break;
	default:
		return -1;
	}

	if (!ret) {
		std::cerr << "Operation did not complete successfully!\n";
	}

#endif
}
