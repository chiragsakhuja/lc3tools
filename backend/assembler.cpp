/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <sstream>
#include <vector>

#include "aliases.h"
#include "asm_types.h"
#include "assembler.h"
#include "device_regs.h"
#include "optional.h"
#include "tokenizer.h"

static constexpr uint32_t INST_NAME_CLOSENESS = 2;

std::stringstream lc3::core::Assembler::assemble(std::istream & buffer)
{
    using namespace asmbl;
    using namespace lc3::utils;

    bool success = true;
    uint32_t fail_pass = 0;

    logger.printf(PrintType::P_EXTRA, true, "===== begin identifying tokens =====");
    std::vector<StatementNew> statements = buildStatements(buffer);
    logger.printf(PrintType::P_EXTRA, true, "===== end identifying tokens =====");
    logger.newline(PrintType::P_EXTRA);

    logger.printf(PrintType::P_EXTRA, true, "===== begin marking PCs =====");
    setStatementPCField(statements);
    logger.printf(PrintType::P_EXTRA, true, "===== end marking PCs =====");
    logger.newline(PrintType::P_EXTRA);

    logger.printf(PrintType::P_EXTRA, true, "===== begin building symbol table =====");
    std::pair<bool, SymbolTable> symbols = buildSymbolTable(statements);
    success &= symbols.first;
    logger.printf(PrintType::P_EXTRA, true, "===== end building symbol table =====");
    logger.newline(PrintType::P_EXTRA);
    if(! success) {
        fail_pass = 1;
    }

    logger.printf(PrintType::P_EXTRA, true, "===== begin assembling =====");
    std::pair<bool, std::vector<MemEntry>> machine_code_blob = buildMachineCode(statements, symbols.second);
    success &= machine_code_blob.first;
    logger.printf(PrintType::P_EXTRA, true, "===== end assembling =====");
    logger.newline(PrintType::P_EXTRA);
    if(! success && fail_pass == 0) {
        fail_pass = 2;
    }

    if(! success) {
        if(fail_pass == 0) {
            logger.printf(PrintType::P_ERROR, true, "assembly failed");
        } else {
            logger.printf(PrintType::P_ERROR, true, "assembly failed (pass %d)", fail_pass);
        }
        throw lc3::utils::exception("assembly failed");
    }

    std::stringstream ret;
    ret << getMagicHeader();
    ret << getVersionString();
    for(MemEntry const & entry : machine_code_blob.second) {
        ret << entry;
    }
    return ret;
}

std::vector<lc3::core::asmbl::StatementNew> lc3::core::Assembler::buildStatements(std::istream & buffer)
{
    using namespace asmbl;
    using namespace lc3::utils;

    Tokenizer tokenizer{buffer};
    std::vector<StatementNew> statements;

    while(! tokenizer.isDone()) {
        std::vector<Token> tokens;
        Token cur_token;
        while(! (tokenizer >> cur_token) && cur_token.type != Token::Type::EOL) {
            tokens.push_back(cur_token);
#ifdef _ENABLE_DEBUG
            std::stringstream token_str;
            ::operator<<(token_str, cur_token);
            logger.printf(PrintType::P_EXTRA, true, " (token) %s", token_str.str().c_str());
#endif
        }

        if(! tokenizer.isDone()) {
            statements.push_back(buildStatement(tokens));
        }
    }

    return statements;
}

lc3::core::asmbl::StatementNew lc3::core::Assembler::buildStatement(
    std::vector<lc3::core::asmbl::Token> const & tokens)
{
    using namespace asmbl;
    using namespace lc3::utils;

    StatementNew ret;

    // A lot of special case logic here to identify tokens as labels, instructions, pseudo-ops, etc.
    // Note: This DOES NOT check for valid statements, it just identifies tokens with what they should be
    //       based on the structure of the statement.
    // Note: There is some redundancy in the code below (not too much), but it was written this way so that it's
    //       easier to follow the flowchart.
    if(tokens.size() > 0) {
        ret.line = tokens[0].line;
        ret.row = tokens[0].row;
        uint32_t operand_start_idx = 0;

        if(tokens[0].type == Token::Type::STRING) {
            // If the first token is a string, it could be a label, instruction, or pseudo-op.
            if(encoder.isStringPseudo(tokens[0].str)) {
                // If the token is identified as a pseudo-op, mark it as such.
                ret.base = StatementPiece{tokens[0], StatementPiece::Type::PSEUDO};
                operand_start_idx = 1;
            } else {
                // If the token is not a pseudo-op, it could be either a label or an instruction.
                uint32_t dist_from_inst_name = encoder.getDistanceToNearestInstructionName(tokens[0].str);
                if(dist_from_inst_name == 0) {
                    // The token has been identified to match a valid instruction string, but don't be too hasty
                    // in marking it as an instruction yet.
                    if(tokens.size() > 1 && tokens[1].type == Token::Type::STRING) {
                        if(encoder.isStringPseudo(tokens[1].str)) {
                            // If there's a following token that's a pseudo-op, the user probably accidentally used
                            // an instruction name as a label, so mark it as such.
                            ret.label = StatementPiece{tokens[0], StatementPiece::Type::LABEL};
                            ret.base = StatementPiece{tokens[1], StatementPiece::Type::PSEUDO};
                            operand_start_idx = 2;
                        } else {
                            // In most cases, the following token doesn't make a difference and the token really is
                            // an instruction.
                            ret.base = StatementPiece{tokens[0], StatementPiece::Type::INST};
                            operand_start_idx = 1;
                        }
                    } else {
                        // If the following token is a number, this token is an instruction.
                        ret.base = StatementPiece{tokens[0], StatementPiece::Type::INST};
                        operand_start_idx = 1;
                    }
                } else {
                    // The first token wasn't identified as an instruction, so it could either be a label or a typo-ed
                    // instruction.
                    if(tokens.size() > 1) {
                        if(tokens[1].type == Token::Type::STRING) {
                            if(encoder.isStringPseudo(tokens[1].str)) {
                                // If the following token is a pseudo-op, assume the user meant to type a label.
                                ret.label = StatementPiece{tokens[0], StatementPiece::Type::LABEL};
                                ret.base = StatementPiece{tokens[1], StatementPiece::Type::PSEUDO};
                                operand_start_idx = 2;
                            } else if(encoder.isStringValidReg(tokens[1].str)) {
                                // If the following token is a register, assume the user meant to type an instruction...
                                // unless the distance from any valid instruction is too large.
                                if(dist_from_inst_name < INST_NAME_CLOSENESS) {
                                    ret.base = StatementPiece{tokens[0], StatementPiece::Type::INST};
                                    operand_start_idx = 1;
                                } else {
                                    ret.label = StatementPiece{tokens[0], StatementPiece::Type::INST};
                                    operand_start_idx = 1;
                                }
                            } else {
                                // If the following token is a string that was not identified as a pseudo-op or register,
                                // compare to see which token has the closer distance to a valid instruction. Even then,
                                // only mark as an instruction if the distance is close enough to a valid instruction.
                                uint32_t next_dist_from_inst_name = encoder.getDistanceToNearestInstructionName(
                                    tokens[1].str);
                                if(next_dist_from_inst_name < dist_from_inst_name) {
                                    if(next_dist_from_inst_name < INST_NAME_CLOSENESS) {
                                        ret.label = StatementPiece{tokens[0], StatementPiece::Type::LABEL};
                                        ret.base = StatementPiece{tokens[1], StatementPiece::Type::INST};
                                        operand_start_idx = 2;
                                    } else {
                                        ret.label = StatementPiece{tokens[0], StatementPiece::Type::LABEL};
                                        operand_start_idx = 1;
                                    }
                                } else {
                                    if(dist_from_inst_name < INST_NAME_CLOSENESS) {
                                        ret.base = StatementPiece{tokens[0], StatementPiece::Type::INST};
                                        operand_start_idx = 1;
                                    } else {
                                        ret.label = StatementPiece{tokens[0], StatementPiece::Type::LABEL};
                                        operand_start_idx = 1;
                                    }
                                }
                            }
                        } else {
                            // If the following token is a number, assume the user meant to type an instruction...
                            // unless the distance from any valid instruction is too large.
                            if(dist_from_inst_name < INST_NAME_CLOSENESS) {
                                ret.base = StatementPiece{tokens[0], StatementPiece::Type::INST};
                                operand_start_idx = 1;
                            } else {
                                ret.label = StatementPiece{tokens[0], StatementPiece::Type::INST};
                                operand_start_idx = 1;
                            }
                        }
                    } else {
                        // If there are no more tokens on the line, just assume the user typed in a label rather than a
                        // mis-typed instruction.
                        ret.label = StatementPiece{tokens[0], StatementPiece::Type::LABEL};
                        operand_start_idx = 1;
                    }
                }
            }
        } else {
            ret.label = StatementPiece{tokens[0], StatementPiece::Type::NUM};
            operand_start_idx = 1;
        }

        for(uint32_t i = operand_start_idx; i < tokens.size(); i += 1) {
            if(tokens[i].type == Token::Type::STRING) {
                if(encoder.isStringValidReg(tokens[i].str)) {
                    ret.operands.emplace_back(tokens[i], StatementPiece::Type::REG);
                } else {
                    ret.operands.emplace_back(tokens[i], StatementPiece::Type::STRING);
                }
            } else {
                ret.operands.emplace_back(tokens[i], StatementPiece::Type::NUM);
            }
        }
    }

    std::stringstream statement_str;
    ::operator<<(statement_str, ret);
    logger.printf(PrintType::P_EXTRA, true, "%s", statement_str.str().c_str());

    return ret;
}

void lc3::core::Assembler::setStatementPCField(std::vector<lc3::core::asmbl::StatementNew> & statements)
{
    using namespace asmbl;
    using namespace lc3::utils;

    uint32_t cur_idx = 0;

    bool found_orig = false;
    bool previous_region_ended = false;
    uint32_t cur_pc = 0;

    // Iterate over the statements, setting the current PC every time a new .orig is found.
    while(cur_idx < statements.size()) {
        StatementNew & statement = statements[cur_idx];

        if(encoder.isPseudo(statement)) {
            if(encoder.isValidPseudoOrig(statement)) {
                if(found_orig) {
#ifndef _LIBERAL_ASM
                    if(! previous_region_ended) {
                        // If found_orig is set, meaning we've seen at least one valid .orig, and previous_region_ended
                        // is not set, meaning we haven't seen a .end yet, then the previous .orig was not ended
                        // properly.
                        logger.asmPrintf(PrintType::P_ERROR, statement,
                            "new .orig found, but previous region did not have .end");
                        logger.newline();
                        throw utils::exception("new .orig fund, but previous region did not have .end");
                    }
#endif
                    previous_region_ended = false;
                }

                found_orig = true;
                cur_pc = encoder.getPseudoOrig(statement);
                statement.pc = 0;
                ++cur_idx;
                logger.printf(PrintType::P_EXTRA, true, "setting current PC to 0x%0.4x", cur_pc);
                if((cur_pc & 0xffff) != cur_pc) {
#ifdef _LIBERAL_ASM
                    logger.printf(PrintType::P_WARNING, true, "truncating .orig from 0x%0.4x to 0x%0.4x", cur_pc, cur_pc & 0xffff);
                    logger.newline(PrintType::P_WARNING);
#else
                    logger.printf(PrintType::P_ERROR, true, ".orig 0x%0.4x is out of range", cur_pc);
                    logger.newline();
                    throw utils::exception(".orig is out of range");
#endif
                }
                cur_pc &= 0xffff;
                continue;
            } else if(encoder.isValidPseudoEnd(statement)) {
                // If we see a .end, make sure we've seen at least one .orig already (indicated by found_orig being set).
                if(! found_orig) { statement.valid = false; }
                previous_region_ended = true;
                statement.pc = 0;
                ++cur_idx;
                continue;
            }
        }

        if(statement.label && ! statement.base) {
            // If the line is only a label, give it the same PC as the line it is pointing to.
            if(! found_orig) { statement.valid = false; }
            statement.pc = cur_pc;
            ++cur_idx;
            continue;
        }

        if(found_orig) {
            if(cur_pc >= MMIO_START) {
                // If the PC has reached the MMIO region, abort!
                logger.asmPrintf(PrintType::P_ERROR, statement, "cannot write code into memory-mapped I/O region");
                logger.newline();
                throw utils::exception("cannot write code into memory-mapped I/O region");
            }

            if(previous_region_ended) {
                // If found_orig and previous_region_ended are both set, that means we have not set
                // previous_region_ended to false yet, which happens when we find a new .orig. In other
                // words, this means there is a line between a .end and a .orig that should be ignored.
                statement.valid = false;
                ++cur_idx;
                continue;
            }

            statement.pc = cur_pc;
            ++cur_pc;
            logger.printf(PrintType::P_EXTRA, true, "0x%0.4x : \'%s\'", statement.pc, statement.line.c_str());
        } else {
            // If we make it here and haven't found a .orig yet, then there are extraneous lines at the beginning
            // of the file.
            statement.valid = false;
        }

        // Finally, some pseudo-ops need to increment PC by more than 1.
        if(encoder.isPseudo(statement)) {
            if(encoder.isValidPseudoBlock(statement)) {
                cur_pc += encoder.getPseudoBlockSize(statement) - 1;
            } else if(encoder.isValidPseudoString(statement)) {
                cur_pc += encoder.getPseudoStringSize(statement) - 1;
            }
        }

        ++cur_idx;
    }

    // Trigger an error if there was no valid .orig in the file.
    if(! found_orig) {
        logger.printf(PrintType::P_ERROR, true, "could not find valid .orig");
        logger.newline();
        throw utils::exception("could not find valid .orig");
    }

#ifndef _LIBERAL_ASM
    // Trigger error if there was no .end at the end of the file.
    if(found_orig && ! previous_region_ended) {
        logger.printf(PrintType::P_ERROR, true, "no .end at end of file");
        logger.newline();
        throw utils::exception("no .end at end of file");
    }
#endif
}

std::pair<bool, lc3::core::SymbolTable> lc3::core::Assembler::buildSymbolTable(
    std::vector<lc3::core::asmbl::StatementNew> const & statements)
{
    using namespace asmbl;
    using namespace lc3::utils;

    SymbolTable symbols;
    bool success = true;

    for(StatementNew const & statement : statements) {
        if(statement.label) {
            if(statement.label->type == StatementPiece::Type::NUM) {
                logger.asmPrintf(PrintType::P_ERROR, statement, *statement.label, "label cannot be a numeric value");
                logger.newline();
                success = false;
            } else {
                if(! statement.base) {
                    if(statement.operands.size() > 0) {
                        for(StatementPiece const & operand : statement.operands) {
                            logger.asmPrintf(PrintType::P_ERROR, statement, operand, "illegal operand to a label");
                            logger.newline();
                        }
                        success = false;
                        continue;
                    } else {
#ifndef _LIBERAL_ASM
                        logger.asmPrintf(PrintType::P_ERROR, statement, *statement.label,
                            "cannot have label on its own line");
                        logger.newline();
                        success = false;
                        continue;
#endif
                    }
                }

                auto search = symbols.find(utils::toLower(statement.label->str));
                if(search != symbols.end()) {
                    uint32_t old_val = search->second;
#ifdef _LIBERAL_ASM
                    logger.asmPrintf(PrintType::P_WARNING, statement, *statement.label,
                        "redefining label \'%s\' from 0x%0.4x to 0x%0.4x", statement.label->str.c_str(),
                        old_val, statement.pc);
                    logger.newline(PrintType::P_WARNING);
#else
                    logger.asmPrintf(PrintType::P_ERROR, statement, *statement.label,
                        "attempting to redefine label \'%s\' from 0x%0.4x to 0x%0.4x", statement.label->str.c_str(),
                        old_val, statement.pc);
                    logger.newline();
                    success = false;
                    continue;
#endif
                }

#ifndef _LIBERAL_ASM
                if('0' <= statement.label->str[0] && statement.label->str[0] <= '9') {
                    logger.asmPrintf(PrintType::P_ERROR, statement, *statement.label, "label cannot begin with number");
                    logger.newline();
                    success = false;
                    continue;
                }

                if(encoder.getDistanceToNearestInstructionName(statement.label->str) == 0) {
                    logger.asmPrintf(PrintType::P_ERROR, statement, *statement.label, "label cannot be an instruction");
                    logger.newline();
                    success = false;
                    continue;
                }
#endif

                symbols[utils::toLower(statement.label->str)] = statement.pc;
                logger.printf(PrintType::P_EXTRA, true, "adding label \'%s\' := 0x%0.4x", statement.label->str.c_str(),
                    statement.pc);
            }
        }
    }

    return std::make_pair(success, symbols);
}

std::pair<bool, std::vector<lc3::core::MemEntry>> lc3::core::Assembler::buildMachineCode(
    std::vector<lc3::core::asmbl::StatementNew> const & statements, lc3::core::SymbolTable const & symbols)
{
    using namespace asmbl;
    using namespace lc3::utils;

    bool success = true;

    std::vector<MemEntry> ret;

    for(StatementNew const & statement : statements) {
        if(! statement.valid) {
#ifdef _LIBERAL_ASM
            logger.asmPrintf(PrintType::P_WARNING, statement, "ignoring statement whose address cannot be determined");
            logger.newline(PrintType::P_WARNING);
#else
            logger.asmPrintf(PrintType::P_ERROR, statement, "cannot determine address for statement");
            logger.newline();
            success = false;
#endif
            continue;
        }

        if(statement.base) {
            std::stringstream msg;
            ::operator<<(msg, statement) << " := ";

            if(encoder.isPseudo(statement)) {
                bool valid = encoder.validatePseudo(statement, symbols);
                if(valid) {
                    if(encoder.isValidPseudoOrig(statement)) {
                        uint32_t address = encoder.getPseudoOrig(statement);
                        ret.emplace_back(address, true, statement.line);
                        msg << utils::ssprintf("(orig) 0x%0.4x", address);
                    } else if(encoder.isValidPseudoFill(statement, symbols)) {
                        uint32_t value = encoder.getPseudoFill(statement, symbols);
                        ret.emplace_back(value, false, statement.line);
                        msg << utils::ssprintf("0x%0.4x", value);
                    } else if(encoder.isValidPseudoBlock(statement)) {
                        uint32_t size = encoder.getPseudoBlockSize(statement);
                        for(uint32_t i = 0; i < size; i += 1) {
                            ret.emplace_back(0, false, statement.line);
                        }
                        msg << utils::ssprintf("mem[0x%0.4x:0x%04x] = 0", statement.pc, statement.pc + size - 1);
                    } else if(encoder.isValidPseudoString(statement)) {
                        std::string const & value = encoder.getPseudoString(statement);
                        for(char c : value) {
                            ret.emplace_back(c, false, std::string(1, c));
                        }
                        ret.emplace_back(0, false, statement.line);
                        msg << utils::ssprintf("mem[0x%0.4x:0x%04x] = \'%s\\0\'", statement.pc,
                            statement.pc + value.size(), value.c_str());
                    } else if(encoder.isValidPseudoEnd(statement)) {
                        msg << "(end)";
                    } else {
#ifdef _ENABLE_DEBUG
                        // This should never happen because we already validated the pseudo-op.
                        assert(false);
#endif
                    }

                    if(valid) {
                        logger.printf(PrintType::P_EXTRA, true, "%s", msg.str().c_str());
                    } else {
                        logger.printf(PrintType::P_EXTRA, true, "%s not assembled", msg.str().c_str());
                    }
                }
                success &= valid;
            } else if(encoder.isInst(statement)) {
                logger.printf(PrintType::P_EXTRA, true, "%s", msg.str().c_str());
                bool valid = false;
                optional<PIInstruction> candidate = encoder.validateInstruction(statement);
                if(candidate) {
                    optional<uint32_t> value = encoder.encodeInstruction(statement, symbols, *candidate);
                    if(value) {
                        ret.emplace_back(*value, false, statement.line);
                        msg << utils::ssprintf("0x%0.4x", *value);
                        valid = true;
                        logger.printf(PrintType::P_EXTRA, true, "  0x%0.4x", *value);
                    }
                }

                if(! valid) {
                    logger.printf(PrintType::P_EXTRA, true, "  not assembled");
                }
                success &= valid;
            } else {
#ifdef _ENABLE_DEBUG
                // buildStatement should never assign the base field anything other than INST or PSEUDO.
                assert(false);
#endif
            }
        }
    }

    return std::make_pair(success, ret);
}
