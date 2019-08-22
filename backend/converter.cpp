/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <algorithm>
#include <bitset>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#ifdef _ENABLE_DEBUG
    #include <chrono>
#endif

#include "mem.h"
#include "converter.h"

std::stringstream lc3::core::Converter::convertBin(std::istream & buffer)
{
    using namespace lc3::utils;

    std::string line;
    std::vector<MemEntry> obj_blob;
    uint32_t line_no = 0;
    bool wrote_orig = false;
    bool success = true;

    while(std::getline(buffer, line)) {
        line = line.substr(0, line.find_first_of(';'));
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        line_no += 1;

        if(line.size() != 0 && line.size() != 16) {
            logger.printf(PrintType::P_ERROR, true, "line %d is %s", line_no, line_no < 16 ? "too short" : "too long");
            success = false;
            continue;
        }

        if(line.size() == 0) { continue; }

        bool skip_conversion = false;
        for(char c : line) {
            if(c != '0' && c != '1') {
                success = false;
                skip_conversion = true;
                logger.printf(PrintType::P_ERROR, true, "line %d contains illegal characters", line_no);
                break;
            }
        }

        if(skip_conversion) { continue; }

        uint32_t val = std::bitset<16>(line).to_ulong();
        logger.printf(PrintType::P_DEBUG, false, "%s => 0x%04x", line.c_str(), val);
        obj_blob.emplace_back((uint16_t) val, ! wrote_orig, line);
        wrote_orig = true;
    }

    if(! success) {
        logger.printf(PrintType::P_ERROR, true, "conversion failed");
        throw lc3::utils::exception("conversion failed");
    }

    logger.printf(PrintType::P_INFO, true, "conversion successful");

    std::stringstream ret;
    ret << getMagicHeader();
    ret << getVersionString();
    for(MemEntry entry : obj_blob) {
        ret << entry;
    }

    return ret;
}

