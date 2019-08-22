/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <cstring>

#include "common.h"

std::vector<std::pair<std::string, std::string>> parseCLIArgs(int argc, char * argv[])
{
    std::vector<std::pair<std::string, std::string>> parsed_args;
    for(uint32_t i = 1; i < (uint32_t) argc; i += 1) {
        uint32_t len = std::strlen(argv[0]);
        if(len > 0 && argv[i][0] == '-') {
            uint32_t start_index = 1;
            if(len > 1 && argv[i][1] == '-') {
                start_index += 1;
            }
            char * equals = std::strchr(argv[i], '=');

            if(equals != nullptr) {
                std::string arg_str(argv[i], start_index, equals - argv[i] - start_index);
                std::string val_str(equals + 1);
                parsed_args.push_back(std::make_pair(arg_str, val_str));
            } else {
                std::string arg_str(argv[i], start_index, strlen(argv[i]));
                parsed_args.push_back(std::make_pair(arg_str, ""));
            }
        }
    }
    return parsed_args;
}

