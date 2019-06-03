#include <algorithm>
#include <bitset>
#include <fstream>
#include <string>
#include <vector>

#ifdef _ENABLE_DEBUG
    #include <chrono>
#endif

#include "mem.h"
#include "converter.h"

void lc3::core::Converter::convertBin(std::string const & bin_filename, std::string const & obj_filename)
{
    using namespace lc3::utils;

    // check if file exists
    std::ifstream in_file(bin_filename);
    if(! in_file.is_open()) {
        logger.printf(PrintType::P_ERROR, true, "could not open %s for reading", bin_filename.c_str());
        throw lc3::utils::exception("could not open file for reading");
    }

#ifdef _ENABLE_DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif

    logger.printf(PrintType::P_INFO, true, "attemping to convert \'%s\' into \'%s\'", bin_filename.c_str(),
        obj_filename.c_str());

    std::string line;
    std::vector<MemEntry> obj_blob;
    uint32_t line_no = 1;
    bool wrote_orig = false;
    bool success = true;

    while(std::getline(in_file, line)) {
        line = line.substr(0, line.find_first_of(';'));
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        line_no += 1;

        if(line.size() != 0 && line.size() != 16) {
            logger.printf(PrintType::P_ERROR, true, "line %d is %s", line_no, line_no < 16 ? "too short" : "too long");
            success = false;
            continue;
        }

        //if(line.size() == 0) { continue; }

        for(char c : line) {
            if(c != '0' && c != '1') {
                success = false;
                continue;
            }
        }

        uint32_t val = std::bitset<16>(line).to_ulong();
        logger.printf(PrintType::P_DEBUG, false, "%s => 0x%04x", line.c_str(), val);
        obj_blob.emplace_back((uint16_t) val, ! wrote_orig, line);
        wrote_orig = true;
    }

#ifdef _ENABLE_DEBUG
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    logger.printf(PrintType::P_EXTRA, true, "elapsed time: %f ms", elapsed * 1000);
#endif

    in_file.close();

    if(success) {
        logger.printf(PrintType::P_INFO, true, "conversion successful");

        std::ofstream out_file(obj_filename);
        if(! out_file.is_open()) {
            logger.printf(PrintType::P_ERROR, true, "could not open file %s for writing", obj_filename.c_str());
            throw lc3::utils::exception("could not open file for writing");
        }

        for(MemEntry entry : obj_blob) {
            out_file << entry;
        }

        out_file.close();
    } else {
        logger.printf(PrintType::P_ERROR, true, "conversion failed");
        throw lc3::utils::exception("conversion failed");
    }
}

