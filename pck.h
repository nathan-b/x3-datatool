#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * PCK compression support for X3: Terran Conflict and later.
 *
 * X3TC+ uses standard gzip compression with no game-specific encryption.
 * This module provides compression, decompression, and file type detection
 * for .pck files extracted from X3 archives.
 */

/**
 * Detect if data is gzip-compressed (starts with gzip magic bytes 0x1F 0x8B).
 *
 * @param data Pointer to data buffer
 * @param size Size of data buffer in bytes (needs >= 2 for reliable detection)
 * @return true if data appears to be gzip-compressed, false otherwise
 */
bool is_compressed(const uint8_t* data, size_t size);

/**
 * Decompress gzip-compressed data.
 *
 * @param data Vector containing gzip-compressed data
 * @return Vector containing decompressed data, or empty vector on failure or if not compressed
 */
std::vector<uint8_t> unpack(const std::vector<uint8_t>& data);

/**
 * Compress data to gzip format.
 *
 * Uses compression level 9 (maximum) to minimize file size.
 *
 * @param data Vector containing uncompressed data
 * @return Vector containing gzip-compressed data, or empty vector on failure
 */
std::vector<uint8_t> pack(const std::vector<uint8_t>& data);

/**
 * Detect the likely file extension from decompressed content.
 *
 * Uses magic byte signatures to identify common X3 file types:
 * - .xml (with or without UTF-8 BOM)
 * - .dds (DirectDraw Surface textures)
 * - .bob (X3 binary object format)
 * - .txt (default fallback)
 *
 * @param data Pointer to decompressed data
 * @param size Size of data in bytes
 * @return Extension including the leading dot (e.g. ".txt", ".xml")
 */
std::string detect_extension(const uint8_t* data, size_t size);
