#include "pck.h"

#include <cstring>
#include <iostream>
#include <zlib.h>

// Gzip magic bytes
constexpr uint8_t GZIP_MAGIC1 = 0x1F;
constexpr uint8_t GZIP_MAGIC2 = 0x8B;

// Buffer size for compression/decompression
constexpr size_t CHUNK_SIZE = 16384; // 16 KB

bool is_compressed(const uint8_t* data, size_t size) {
	if (size < 2) {
		return false;
	}
	return data[0] == GZIP_MAGIC1 && data[1] == GZIP_MAGIC2;
}

std::vector<uint8_t> unpack(const std::vector<uint8_t>& data) {
	if (data.empty() || !is_compressed(data.data(), data.size())) {
		return {}; // Not compressed or invalid
	}

	// Initialize zlib for gzip decompression
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	// windowBits = 15 + 16 tells zlib to decode gzip format (automatic header handling)
	if (inflateInit2(&zs, 15 + 16) != Z_OK) {
		std::cerr << "Failed to initialize zlib for decompression\n";
		return {};
	}

	zs.next_in = const_cast<uint8_t*>(data.data());
	zs.avail_in = data.size();

	std::vector<uint8_t> output;
	std::vector<uint8_t> temp_buffer(CHUNK_SIZE);

	int ret;
	do {
		zs.next_out = temp_buffer.data();
		zs.avail_out = temp_buffer.size();

		ret = inflate(&zs, Z_NO_FLUSH);

		if (ret != Z_OK && ret != Z_STREAM_END) {
			std::cerr << "Decompression failed with error code: " << ret << "\n";
			inflateEnd(&zs);
			return {};
		}

		size_t bytes_written = temp_buffer.size() - zs.avail_out;
		output.insert(output.end(), temp_buffer.begin(), temp_buffer.begin() + bytes_written);

	} while (ret != Z_STREAM_END);

	inflateEnd(&zs);
	return output;
}

std::vector<uint8_t> pack(const std::vector<uint8_t>& data) {
	if (data.empty()) {
		return {}; // Cannot compress empty data
	}

	// Initialize zlib for gzip compression
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	// windowBits = 15 + 16 tells zlib to produce gzip format
	// Level 9 = maximum compression (matching reference implementation)
	// memLevel 9 = maximum memory usage for better compression
	if (deflateInit2(&zs, 9, Z_DEFLATED, 15 + 16, 9, Z_DEFAULT_STRATEGY) != Z_OK) {
		std::cerr << "Failed to initialize zlib for compression\n";
		return {};
	}

	zs.next_in = const_cast<uint8_t*>(data.data());
	zs.avail_in = data.size();

	std::vector<uint8_t> output;
	std::vector<uint8_t> temp_buffer(CHUNK_SIZE);

	int ret;
	do {
		zs.next_out = temp_buffer.data();
		zs.avail_out = temp_buffer.size();

		ret = deflate(&zs, Z_FINISH);

		if (ret != Z_OK && ret != Z_STREAM_END) {
			std::cerr << "Compression failed with error code: " << ret << "\n";
			deflateEnd(&zs);
			return {};
		}

		size_t bytes_written = temp_buffer.size() - zs.avail_out;
		output.insert(output.end(), temp_buffer.begin(), temp_buffer.begin() + bytes_written);

	} while (ret != Z_STREAM_END);

	deflateEnd(&zs);
	return output;
}

std::string detect_extension(const uint8_t* data, size_t size) {
	// Check signatures in priority order (longest first)

	// XML with UTF-8 BOM: EF BB BF 3C 3F 78 6D 6C ("<?xml" with BOM)
	if (size >= 8) {
		if (data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF && data[3] == 0x3C && data[4] == 0x3F &&
		    data[5] == 0x78 && data[6] == 0x6D && data[7] == 0x6C) {
			return ".xml";
		}
	}

	// DDS: 44 44 53 20 ("DDS ")
	if (size >= 4) {
		if (data[0] == 0x44 && data[1] == 0x44 && data[2] == 0x53 && data[3] == 0x20) {
			return ".dds";
		}
	}

	// XML without BOM: 3C 3F 78 6D 6C ("<?xml")
	if (size >= 5) {
		if (data[0] == 0x3C && data[1] == 0x3F && data[2] == 0x78 && data[3] == 0x6D && data[4] == 0x6C) {
			return ".xml";
		}
	}

	// BOB1: 42 4F 42 31 ("BOB1")
	if (size >= 4) {
		if (data[0] == 0x42 && data[1] == 0x4F && data[2] == 0x42 && data[3] == 0x31) {
			return ".bob";
		}
	}

	// CUT1: 43 55 54 31 ("CUT1")
	if (size >= 4) {
		if (data[0] == 0x43 && data[1] == 0x55 && data[2] == 0x54 && data[3] == 0x31) {
			return ".bob";
		}
	}

	// Default fallback
	return ".txt";
}
