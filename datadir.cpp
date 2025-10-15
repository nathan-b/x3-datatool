#include <cstdint>
#include <map>
#include <string>

#include "datadir.h"


datadir::datadir(const std::string& path)
{
    // Iterate through all files in the path, looking for cat files
    // Foreach cat file, create a datafile
    // Store the datafile
}

bool add(const std::string& datafile_path)
{
    // Create a datafile
    // Store the datafile
}

bool add(datafile& file)
{

}

datafile& search(const std::string& filename);

uint32_t datadir::get_id_from_filename(const std::string& filename)
{
    // The normal case is that the file is named ##.cat, so we can just use that number
}
