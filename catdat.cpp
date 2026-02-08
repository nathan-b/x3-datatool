#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "datadir.h"
#include "datafile.h"
#include "operation.h"
#include "pck.h"

struct catfile {
	std::string filename;
	std::list<std::string> file_list;
};

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

bool pack_file(const std::filesystem::path& inpath, const std::filesystem::path& outpath) {
	// Read input file
	std::ifstream infile(inpath, std::ios::in | std::ios::binary);
	if (!infile) {
		std::cerr << "Could not open input file " << inpath << " for reading\n";
		return false;
	}

	// Read entire file into buffer
	infile.seekg(0, std::ios::end);
	size_t size = infile.tellg();
	infile.seekg(0, std::ios::beg);

	std::vector<uint8_t> input_data(size);
	infile.read((char*)input_data.data(), size);
	infile.close();

	// Compress the data
	auto compressed = pack(input_data);
	if (compressed.empty()) {
		std::cerr << "Failed to compress " << inpath << "\n";
		return false;
	}

	// Determine output filename
	std::filesystem::path actual_outpath = outpath;
	if (actual_outpath.empty()) {
		actual_outpath = std::filesystem::path(inpath).stem().string() + ".pck";
	}

	// Write compressed data
	std::ofstream outfile(actual_outpath, std::ios::out | std::ios::binary);
	if (!outfile) {
		std::cerr << "Could not open output file " << actual_outpath << " for writing\n";
		return false;
	}
	outfile.write((char*)compressed.data(), compressed.size());
	outfile.close();

	std::cout << "Compressed " << inpath << " to " << actual_outpath << " (" << size << " -> " << compressed.size()
			  << " bytes)\n";
	return true;
}

bool unpack_file(const std::filesystem::path& inpath, const std::filesystem::path& outpath) {
	// Read input file
	std::ifstream infile(inpath, std::ios::in | std::ios::binary);
	if (!infile) {
		std::cerr << "Could not open input file " << inpath << " for reading\n";
		return false;
	}

	// Read entire file into buffer
	infile.seekg(0, std::ios::end);
	size_t size = infile.tellg();
	infile.seekg(0, std::ios::beg);

	std::vector<uint8_t> input_data(size);
	infile.read((char*)input_data.data(), size);
	infile.close();

	// Check if compressed
	if (!is_compressed(input_data.data(), input_data.size())) {
		std::cerr << inpath << " does not appear to be compressed\n";
		return false;
	}

	// Decompress the data
	auto decompressed = unpack(input_data);
	if (decompressed.empty()) {
		std::cerr << "Failed to decompress " << inpath << "\n";
		return false;
	}

	// Detect extension
	std::string ext = detect_extension(decompressed.data(), decompressed.size());

	// Determine output filename
	std::filesystem::path actual_outpath = outpath;
	if (actual_outpath.empty()) {
		actual_outpath = std::filesystem::path(inpath).stem().string() + ext;
	}

	// Write decompressed data
	std::ofstream outfile(actual_outpath, std::ios::out | std::ios::binary);
	if (!outfile) {
		std::cerr << "Could not open output file " << actual_outpath << " for writing\n";
		return false;
	}
	outfile.write((char*)decompressed.data(), decompressed.size());
	outfile.close();

	std::cout << "Decompressed " << inpath << " to " << actual_outpath << " (" << size << " -> " << decompressed.size()
			  << " bytes)\n";
	return true;
}

bool extract_archive(const datafile& idx, const std::filesystem::path& outpath) {
	return idx.extract(outpath);
}

bool extract_all(const std::string& inpath, const std::filesystem::path& outpath, bool unpack_pck) {
	// Create the target directory if it doesn't exist
	std::filesystem::create_directories(outpath);

	// Now extract the catalogs in the directory to the target path
	datadir dd(inpath);
	dd.unpack_on_extract(unpack_pck);
	return dd.extract(outpath);
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
	std::cout
		<< "Usage: x3tool <operation> [cat_file] [options]\n"
		<< "  Valid operations: t / dump-index             Print the index of the package file\n"
		<< "                    d / decode-file  [-o output-path]  Decode cat file to the given "
		   "path (or current directory)\n"
		<< "                    f / extract-file <-f filename> [--pck] [-o output-file]  Extract the "
		   "contents of a single file to disk\n"
		<< "                    x / extract-archive  [--pck] [-o output-path]  Extract one entire archive "
		   "to the output path (or current directory)\n"
		<< "                    p / build-package <-i input-path>  Build a new cat file with the "
		   "contents of input-path\n"
		<< "                    a / extract-all <-i input-path> [--pck] <-o output-path>  Extract every archive in the "
		   "provided directory to the output path\n"
		<< "                    s / search <-f filename>  <-i search-directory> Find the most recent "
		<< "cat file in the provided directory which contains the given file\n"
		<< "                    k / pack-file <-i input-file> [-o output.pck]  Compress a file to .pck format\n"
		<< "                    u / unpack-file <-i input.pck> [-o output-file]  Decompress a .pck file\n"
		<< "\n  Flags:\n"
		<< "                    --pck                    Automatically decompress .pck files during extraction\n";
}

int main(int argc, char** argv) {
	operation op;
	bool ret = false;

	if (!op.parse(argc, argv)) {
		std::cerr << "Command line input error\n";
		usage();
		return -1;
	}

	// search, build_package, extract_all, pack_file, and unpack_file operations do not need an input catalog file
	bool done = false;
	switch (op.get_type()) {
	case SEARCH:
		ret = search(op.get_src_filename(), op.get_internal_filename());
		done = true;
		break;
	case EXTRACT_ALL:
		ret = extract_all(op.get_src_filename(), op.get_dest_path(), op.get_pck_flag());
		done = true;
		break;
	case BUILD_PACKAGE: {
		std::filesystem::path catfile = op.get_input_filename();
		if (catfile.empty() && !op.get_dest_path().empty()) {
			// Since the arguments can be a little confusing, if the user did not specify
			// an input file but did specify an output file, we know what to do
			catfile = op.get_dest_path();
		}

		if (catfile.empty()) {
			std::cerr << "You must specify a filename for the new .cat file\n";
			usage();
			return -1;
		}
		ret = build_package(catfile, op.get_src_filename());
		done = true;
	} break;
	case PACK_FILE: {
		if (op.get_src_filename().empty()) {
			std::cerr << "You must specify an input file with -i\n";
			usage();
			return -1;
		}
		ret = pack_file(op.get_src_filename(), op.get_dest_path());
		done = true;
	} break;
	case UNPACK_FILE: {
		if (op.get_src_filename().empty()) {
			std::cerr << "You must specify an input file with -i\n";
			usage();
			return -1;
		}
		ret = unpack_file(op.get_src_filename(), op.get_dest_path());
		done = true;
	} break;
	default:
		break;
	}

	// All other operations take a catalog file
	if (!done) {
		// Parse the index file
		datafile df;

		if (!df.parse(op.get_input_filename())) {
			std::cerr << "Could not read .cat file " << op.get_input_filename() << std::endl;
			return -1;
		}

		// Enable automatic unpacking if --pck flag is set
		if (op.get_pck_flag()) {
			df.unpack_on_extract(true);
		}

		switch (op.get_type()) {
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
		default:
			return -1;
		}
	}

	if (!ret) {
		std::cerr << "Operation did not complete successfully!\n";
		return 1;
	}
	return 0;
}
