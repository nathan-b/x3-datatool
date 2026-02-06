#include "pck.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

// Test basic compression detection with gzip magic bytes
TEST(pck, detect_compressed_valid) {
	uint8_t gzip_data[] = {0x1F, 0x8B, 0x08, 0x00};
	EXPECT_TRUE(is_compressed(gzip_data, sizeof(gzip_data)));
}

TEST(pck, detect_compressed_invalid) {
	uint8_t plain_data[] = {'H', 'e', 'l', 'l', 'o'};
	EXPECT_FALSE(is_compressed(plain_data, sizeof(plain_data)));
}

TEST(pck, detect_compressed_too_short) {
	uint8_t short_data[] = {0x1F};
	EXPECT_FALSE(is_compressed(short_data, sizeof(short_data)));
}

TEST(pck, detect_compressed_empty) {
	EXPECT_FALSE(is_compressed(nullptr, 0));
}

// Test round-trip compression and decompression
TEST(pck, round_trip_small) {
	std::vector<uint8_t> original = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};

	auto compressed = pack(original);
	ASSERT_FALSE(compressed.empty());
	EXPECT_TRUE(is_compressed(compressed.data(), compressed.size()));

	auto decompressed = unpack(compressed);
	ASSERT_FALSE(decompressed.empty());
	EXPECT_EQ(original, decompressed);
}

TEST(pck, round_trip_large) {
	// Create a larger test dataset with repetitive data (compresses well)
	std::vector<uint8_t> original(8192);
	for (size_t i = 0; i < original.size(); i++) {
		original[i] = i % 256;
	}

	auto compressed = pack(original);
	ASSERT_FALSE(compressed.empty());
	EXPECT_LT(compressed.size(), original.size()); // Should be smaller due to compression

	auto decompressed = unpack(compressed);
	ASSERT_FALSE(decompressed.empty());
	EXPECT_EQ(original, decompressed);
}

TEST(pck, round_trip_empty) {
	std::vector<uint8_t> original;
	auto compressed = pack(original);
	EXPECT_TRUE(compressed.empty()); // Cannot compress empty data
}

TEST(pck, unpack_non_compressed) {
	std::vector<uint8_t> plain_data = {'H', 'e', 'l', 'l', 'o'};
	auto result = unpack(plain_data);
	EXPECT_TRUE(result.empty()); // Should return empty for non-compressed data
}

// Test file extension detection
TEST(pck, detect_extension_xml_with_bom) {
	// UTF-8 BOM + "<?xml"
	uint8_t xml_data[] = {0xEF, 0xBB, 0xBF, 0x3C, 0x3F, 0x78, 0x6D, 0x6C};
	std::string ext = detect_extension(xml_data, sizeof(xml_data));
	EXPECT_EQ(ext, ".xml");
}

TEST(pck, detect_extension_xml_no_bom) {
	// "<?xml"
	uint8_t xml_data[] = {0x3C, 0x3F, 0x78, 0x6D, 0x6C};
	std::string ext = detect_extension(xml_data, sizeof(xml_data));
	EXPECT_EQ(ext, ".xml");
}

TEST(pck, detect_extension_dds) {
	// "DDS "
	uint8_t dds_data[] = {0x44, 0x44, 0x53, 0x20};
	std::string ext = detect_extension(dds_data, sizeof(dds_data));
	EXPECT_EQ(ext, ".dds");
}

TEST(pck, detect_extension_bob1) {
	// "BOB1"
	uint8_t bob_data[] = {0x42, 0x4F, 0x42, 0x31};
	std::string ext = detect_extension(bob_data, sizeof(bob_data));
	EXPECT_EQ(ext, ".bob");
}

TEST(pck, detect_extension_cut1) {
	// "CUT1"
	uint8_t cut_data[] = {0x43, 0x55, 0x54, 0x31};
	std::string ext = detect_extension(cut_data, sizeof(cut_data));
	EXPECT_EQ(ext, ".bob");
}

TEST(pck, detect_extension_txt_fallback) {
	// Random data that doesn't match any signature
	uint8_t unknown_data[] = {0x00, 0x01, 0x02, 0x03};
	std::string ext = detect_extension(unknown_data, sizeof(unknown_data));
	EXPECT_EQ(ext, ".txt");
}

TEST(pck, detect_extension_empty) {
	std::string ext = detect_extension(nullptr, 0);
	EXPECT_EQ(ext, ".txt"); // Should default to .txt
}

// Test compression of various data patterns
TEST(pck, compress_highly_compressible) {
	// Repetitive data should compress very well
	std::vector<uint8_t> original(4096, 'A');

	auto compressed = pack(original);
	ASSERT_FALSE(compressed.empty());
	// Should compress to much less than original size
	EXPECT_LT(compressed.size(), original.size() / 10);

	auto decompressed = unpack(compressed);
	EXPECT_EQ(original, decompressed);
}

TEST(pck, compress_random_data) {
	// Random data compresses poorly
	std::vector<uint8_t> original(1024);
	for (size_t i = 0; i < original.size(); i++) {
		original[i] = (i * 37 + 17) % 256; // Pseudo-random sequence
	}

	auto compressed = pack(original);
	ASSERT_FALSE(compressed.empty());

	auto decompressed = unpack(compressed);
	EXPECT_EQ(original, decompressed);
}

// Test real-world scenario: compress text, check it looks like gzip
TEST(pck, compressed_has_gzip_header) {
	std::string text = "This is a test string for compression. ";
	// Make it longer to ensure good compression
	for (int i = 0; i < 10; i++) {
		text += text;
	}

	std::vector<uint8_t> data(text.begin(), text.end());
	auto compressed = pack(data);

	ASSERT_FALSE(compressed.empty());
	ASSERT_GE(compressed.size(), 2UL);

	// Check gzip magic bytes
	EXPECT_EQ(compressed[0], 0x1F);
	EXPECT_EQ(compressed[1], 0x8B);
	EXPECT_TRUE(is_compressed(compressed.data(), compressed.size()));
}
