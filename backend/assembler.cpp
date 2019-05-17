#include <algorithm>
#include <bitset>
#include <cctype>
#include <fstream>
#include <sstream>
#include <vector>

#ifdef _ENABLE_DEBUG
    #include <chrono>
#endif

#include "aliases.h"
#include "asm_types.h"
#include "assembler.h"
#include "device_regs.h"
#include "tokenizer.h"

using namespace lc3::core;

void Assembler::assemble(std::string const & asm_filename, std::string const & obj_filename)
{
    using namespace asmbl;
    using namespace lc3::utils;

    SymbolTable symbols;
    AssemblerLogger logger(printer, print_level, asm_filename);

    // check if file exists
    std::ifstream file(asm_filename);
    if(! file.is_open()) {
        logger.printf(PrintType::P_ERROR, true, "could not open %s for reading", asm_filename.c_str());
        throw lc3::utils::exception("could not open file for reading");
    }
    file.close();

#ifdef _ENABLE_DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif

    logger.printf(PrintType::P_INFO, true, "attemping to assemble \'%s\' into \'%s\'", asm_filename.c_str(),
        obj_filename.c_str());

    // build statements from tokens
    Tokenizer tokenizer(asm_filename);
    std::vector<Statement> statements;
    while(! tokenizer.isDone()) {
        std::vector<Token> tokens;
        Token token;
        while(! (tokenizer >> token) && token.type != TokenType::EOS) {
            if(token.type != TokenType::EOS) {
                tokens.push_back(token);
            }
        }

        if(! tokenizer.isDone()) {
            statements.push_back(makeStatement(tokens));
        }
    }

    markPC(statements, logger);
    bool success = true, success_tmp = true;
    SymbolTable symbol_table = firstPass(statements, logger, success_tmp);
    success &= success_tmp;
    std::vector<MemEntry> obj_blob = secondPass(statements, symbol_table, logger, success_tmp);
    success &= success_tmp;

#ifdef _ENABLE_DEBUG
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    logger.printf(PrintType::P_EXTRA, true, "elapsed time: %f ms", elapsed * 1000);
#endif

    if(success) {
        logger.printf(PrintType::P_INFO, true, "assembly successful");
        writeFile(obj_blob, obj_filename, logger);
    } else {
        logger.printf(PrintType::P_ERROR, true, "assembly failed");
        throw lc3::utils::exception("assembly failed");
    }
}

void Assembler::convertBin(std::string const & bin_filename, std::string const & obj_filename)
{
    using namespace lc3::utils;

    Logger logger(printer, print_level);

    // check if file exists
    std::ifstream file(bin_filename);
    if(! file.is_open()) {
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

    while(std::getline(file, line)) {
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

    file.close();

    if(success) {
        logger.printf(PrintType::P_INFO, true, "conversion successful");
        writeFile(obj_blob, obj_filename, logger);
    } else {
        logger.printf(PrintType::P_ERROR, true, "conversion failed");
        throw lc3::utils::exception("conversion failed");
    }
}

SymbolTable Assembler::firstPass(std::vector<asmbl::Statement> const & statements,
    lc3::utils::AssemblerLogger & logger, bool & success)
{
    using namespace asmbl;
    using namespace lc3::utils;

    SymbolTable symbol_table;
    success = true;

    for(Statement const & state : statements) {
        if(! state.hasLabel()) { continue; }

        auto search = symbol_table.find(state.label.str);
        if(search != symbol_table.end()) {
            uint32_t old_val = search->second;
#ifdef _LIBERAL_ASM
	    PrintType p_type = PrintType::P_WARNING;
#else
	    PrintType p_type = PrintType::P_ERROR;
	    success = false;
#endif
            logger.asmPrintf(p_type, state.label, "redefining label from 0x%0.4x to 0x%0.4x", old_val,
                state.pc);
            logger.newline();
        }

        symbol_table[state.label.str] = state.pc;
        logger.printf(PrintType::P_EXTRA, true, "adding label \'%s\' => 0x%0.4x", state.label.str.c_str(), state.pc);
    }

    return symbol_table;
}

std::vector<MemEntry> lc3::core::Assembler::secondPass(std::vector<asmbl::Statement> const & statements,
    SymbolTable const & symbol_table, lc3::utils::AssemblerLogger & logger, bool & success)
{
    using namespace asmbl;
    using namespace lc3::utils;

    success = true;

    std::vector<MemEntry> ret;
    uint32_t lines_after_end = 0;
    bool found_end = false;
    for(Statement const & state : statements) {
	if(found_end) {
	    lines_after_end += 1;
	}
        for(StatementToken const & tok : state.invalid_operands) {
#ifdef _LIBERAL_ASM
            logger.asmPrintf(PrintType::P_WARNING, tok, "ignoring unexpected token");
#else
            logger.asmPrintf(PrintType::P_ERROR, tok, "unexpected token");
	    success = false;
#endif
            // TODO: I think the only time there will be an invalid operand is if there is a number at the beginning
            // of the line (or multiple times before an instruction)
            logger.printf(PrintType::P_NOTE, false, "labels cannot look like numbers");
            logger.newline();
        }

        if(state.isInst()) {
            bool encode_success = false;
            uint32_t encoding = encodeInstruction(state, symbol_table, logger, encode_success);
            success &= encode_success;
            ret.emplace_back(encoding, false, state.line);
        } else if(checkIfValidPseudoStatement(state, ".orig", logger, false)) {
	    lines_after_end = 0;
	    found_end = false;
            ret.emplace_back(state.operands[0].num & 0xffff, true, state.line);
        } else if(checkIfValidPseudoStatement(state, ".stringz", logger, false)) {
            std::string const & value = state.operands[0].str;
            for(char c : value) {
                ret.emplace_back(c, false, std::string(1, c));
            }
            ret.emplace_back((uint16_t) 0, false, state.line);
        } else if(checkIfValidPseudoStatement(state, ".blkw", logger, false)) {
            for(uint32_t i = 0; i < (uint32_t) state.operands[0].num; i += 1) {
                ret.emplace_back(0, false, state.line);
            }
        } else if(checkIfValidPseudoStatement(state, ".fill", logger, false)) {
            if(state.operands[0].type == TokenType::NUM) {
                ret.emplace_back(state.operands[0].num, false, state.line);
            } else if(state.operands[0].type == TokenType::LABEL) {
                // TODO: this is a duplicate of the code in LabelOperand::encode
                // eventually create a class for pseudo-ops that's similar to the instructions
                StatementToken const & oper = state.operands[0];
                auto search = symbol_table.find(oper.str);
                if(search == symbol_table.end()) {
                    logger.asmPrintf(PrintType::P_ERROR, oper, "unknown label \'%s\'", oper.str.c_str());
                    logger.newline();
                    success = false;
                    continue;
                }

                ret.emplace_back(search->second, false, state.line);
	    }
        } else if(checkIfValidPseudoStatement(state, ".end", logger, false)) {
	    found_end = true;
        } else {
	    if(!state.hasLabel() && state.invalid_operands.size() == 0) {
		StatementToken state_tok;
		state_tok.line = state.line;
#ifdef _LIBERAL_ASM
		logger.asmPrintf(PrintType::P_WARNING, 0, state_tok.line.size(), state_tok, "ignoring invalid line");
#else
		logger.asmPrintf(PrintType::P_ERROR, 0, state_tok.line.size(), state_tok, "invalid line");
		success = false;
#endif
		logger.newline();
	    }
	}
    }

    if(!found_end) {
#ifdef _LIBERAL_ASM
        logger.printf(PrintType::P_WARNING, true, "assembly did not end in .end", lines_after_end);
#else
        logger.printf(PrintType::P_ERROR, true, "assembly did not end in .end", lines_after_end);
#endif
	logger.newline();
#ifndef _LIBERAL_ASM
        throw utils::exception("assembly did not end in .end");
#endif
    }

    if(lines_after_end > 0) {
#ifdef _LIBERAL_ASM
        logger.printf(PrintType::P_WARNING, true, "ignoring %d lines after final .end", lines_after_end);
#else
        logger.printf(PrintType::P_ERROR, true, "%d invalid lines after final .end", lines_after_end);
#endif
	logger.newline();
#ifndef _LIBERAL_ASM
        throw utils::exception("assembly did not end in .end");
#endif
    }
    return ret;
}

void Assembler::writeFile(std::vector<MemEntry> const & obj_blob, std::string const & obj_filename,
    lc3::utils::Logger & logger)
{
    using namespace lc3::utils;

    std::ofstream file(obj_filename);
    if(! file.is_open()) {
        logger.printf(PrintType::P_ERROR, true, "could not open file %s for writing", obj_filename.c_str());
        throw lc3::utils::exception("could not open file for writing");
    }

    for(MemEntry entry : obj_blob) {
        file << entry;
    }

    file.close();
}

asmbl::Statement Assembler::makeStatement(std::vector<asmbl::Token> const & tokens)
{
    using namespace asmbl;

    std::vector<StatementToken> ret_tokens;
    Statement ret;

    for(Token const & token : tokens) {
        ret_tokens.push_back(StatementToken(token));
    }

    // shouldn't happen, but just in case...
    if(tokens.size() == 0) { return ret; }

    markRegAndPseudoTokens(ret_tokens);
    markInstTokens(ret_tokens);
    markLabelTokens(ret_tokens);
    ret = makeStatementFromTokens(ret_tokens);

    return ret;
}

void Assembler::markRegAndPseudoTokens(std::vector<asmbl::StatementToken> & tokens)
{
    using namespace asmbl;

    for(StatementToken & token : tokens) {
        if(token.type == TokenType::STRING) {
            if(token.str.size() > 0 && token.str[0] == '.') {
                token.type = TokenType::PSEUDO;
            } else if(encoder.isValidReg(token.str)) {
                token.type = TokenType::REG;
            }
        }
    }
}

void Assembler::markInstTokens(std::vector<asmbl::StatementToken> & tokens)
{
    using namespace asmbl;

    if(tokens.size() == 1) {
        if(tokens[0].type == TokenType::STRING) {
            uint32_t token_0_dist = encoder.getDistanceToNearestInstructionName(tokens[0].str);
            if(token_0_dist == 0) {
                tokens[0].type = TokenType::INST;
                tokens[0].lev_dist = token_0_dist;
            } else {
                // there's only one string on this line and it's not exactly an instruction, so assume it's a label
            }
        } else {
            // the sole token on this line is not a string, so it cannot be an instruction
        }
    } else {
        if(tokens[0].type == TokenType::STRING || tokens[0].type == TokenType::NUM) {
            // if the first token is a number, it was probably an attempt at making a label that looks like a number
            // in this case, make the distance too large to be close to an instruction
            uint32_t token_0_dist = 1 << 31;
            if(tokens[0].type != TokenType::NUM) {
                token_0_dist = encoder.getDistanceToNearestInstructionName(tokens[0].str);
            }
            if(tokens[1].type == TokenType::STRING) {
                // first two tokens are both strings, maybe they're instructions?
                uint32_t token_1_dist = encoder.getDistanceToNearestInstructionName(tokens[1].str);
                // see which is closer to an instruction
                // if they're the same, lean toward thinking first token is the instruction
                // imagine a case like 'jsr jsr'; the appropriate error message should be 'jsr is not a label'
                // and for that to be the case, the first token should be marked as the instruction
                if(token_1_dist < token_0_dist) {
                    uint32_t lev_thresh = 1;
                    if(tokens.size() >= 3) {
                        if(tokens[2].type == TokenType::PSEUDO) {
                            // if the next token is a pseudo-op, be a little less lenient in assuming this is an
                            // instruction
                            lev_thresh -= 1;
                        } else {
                            // if the next token is a reg, num, or string be a little more lenient in assuming this
                            // is an instruction
                            lev_thresh += 1;
                        }
                    }
                    if(token_1_dist <= lev_thresh) {
                        tokens[1].type = TokenType::INST;
                        tokens[1].lev_dist = token_1_dist;
                    } else {
                        // too far from an instruction
                    }
                } else {
                    if(token_0_dist < 3) {
                        tokens[0].type = TokenType::INST;
                        tokens[0].lev_dist = token_0_dist;
                    } else {
                        // too far from an instruction
                    }
                }
            } else {
                uint32_t lev_thresh = 1;
                if(tokens[1].type == TokenType::PSEUDO) {
                    if(tokens[0].type == TokenType::STRING) {
                        // if the second token is a pseudo op, then the first token should be considered a label, even if
                        // it matches an instruction
                        tokens[0].type = TokenType::LABEL;
                        return;
                    }
                } else {
                    lev_thresh += 1;
                }
                if(token_0_dist <= lev_thresh) {
                    tokens[0].type = TokenType::INST;
                    tokens[0].lev_dist = token_0_dist;
                }
            }
        } else {
            // the line starts with something other than a string...so the second token cannot be an instruction
        }
    }
}

void Assembler::markLabelTokens(std::vector<asmbl::StatementToken> & tokens)
{
    using namespace asmbl;

    if(tokens.size() > 0 && tokens[0].type == TokenType::STRING) {
        tokens[0].type = TokenType::LABEL;
    }

    // mark any strings after an inst as labels
    bool found_inst = false;
    for(StatementToken & token : tokens) {
        if(found_inst && token.type == TokenType::STRING) {
            token.type = TokenType::LABEL;
        }

        if(token.type == TokenType::INST || checkIfValidPseudoToken(token, ".fill")) {
            found_inst = true;
        }
    }
}

asmbl::Statement Assembler::makeStatementFromTokens(std::vector<asmbl::StatementToken> & tokens)
{
    using namespace asmbl;

    Statement ret;
    uint32_t pos = 0;
    if(tokens.size() > 0) {
        ret.line = tokens[0].line;
        if(tokens[0].type == TokenType::LABEL) {
            StatementToken temp = tokens[0];
            std::transform(temp.str.begin(), temp.str.end(), temp.str.begin(), ::tolower);
            ret.label = temp;
            pos += 1;
        }
    }

    while(pos < tokens.size()) {
        if(tokens[pos].type == TokenType::INST || tokens[pos].type == TokenType::PSEUDO) {
            StatementToken temp = tokens[pos];
            std::transform(temp.str.begin(), temp.str.end(), temp.str.begin(), ::tolower);
            ret.inst_or_pseudo = temp;
            pos += 1;
            break;
        } else {
            // all tokens before an instruction (excluding a possible label) are invalid
            ret.invalid_operands.push_back(tokens[pos]);
        }
        pos += 1;
    }

    while(pos < tokens.size()) {
        StatementToken temp = tokens[pos];
        // at this point, the only STRING is the operand to a stringz
        if(temp.type != TokenType::STRING) {
            // everything is case insensitive
            std::transform(temp.str.begin(), temp.str.end(), temp.str.begin(), ::tolower);
        } else {
            // process stringz operand for escape sequences
            std::stringstream new_str;
            std::string const & str = temp.str;
            for(uint32_t i = 0; i < str.size(); i += 1) {
                char c = str[i];
                if(c == '\\') {
                    if(i + 1 < str.size()) {
                        // if the next character is a recognized escape sequence
                        switch(str[i + 1]) {
                            case 'n' : c = '\n'; i += 1; break;
                            case 'r' : c = '\n'; i += 1; break;
                            case 't' : c = '\t'; i += 1; break;
                            case '\\': c = '\\'; i += 1; break;
                            default: break;
                        }
                    }
                }
                new_str << c;
            }
            temp.str = new_str.str();
        }

        ret.operands.push_back(temp);
        pos += 1;
    }

    return ret;
}

void Assembler::markPC(std::vector<asmbl::Statement> & statements, lc3::utils::AssemblerLogger & logger)
{
    using namespace asmbl;
    using namespace lc3::utils;

    uint32_t cur_pc = 0;
    uint32_t cur_pos = 0;
    bool found_orig = false;

    // find the first valid orig
    while(! found_orig && cur_pos < statements.size()) {
        while(cur_pos < statements.size())  {
            Statement const & state = statements[cur_pos];

            if(checkIfValidPseudoStatement(state, ".orig", logger, true)) {
                found_orig = true;
                break;
            }

            logger.printf(PrintType::P_EXTRA, true, "ignoring line \'%s\' before .orig", state.line.c_str());
            cur_pos += 1;
        }

        if(cur_pos == statements.size()) {
            break;
        }

        Statement const & state = statements[cur_pos];

        uint32_t val = state.operands[0].num;
        uint32_t trunc_val = val & 0xffff;
        if(val != trunc_val) {
#ifdef _LIBERAL_ASM
	    PrintType p_type = PrintType::P_WARNING;
#else
	    PrintType p_type = PrintType::P_ERROR;
#endif
            logger.asmPrintf(p_type, state.operands[0], "truncating address to 0x%0.4x", trunc_val);
            logger.newline();
#ifndef _LIBERAL_ASM
	    throw utils::exception("could not find valid .orig");
#endif
        }

        cur_pc = trunc_val;
        found_orig = true;
    }

    if(! found_orig) {
        logger.printf(PrintType::P_ERROR, true, "could not find valid .orig", cur_pos);
        throw utils::exception("could not find valid .orig");
    }

    if(cur_pos != 0) {
#ifdef _LIBERAL_ASM
        logger.printf(PrintType::P_WARNING, true, "ignoring %d lines before .orig", cur_pos);
#else
        logger.printf(PrintType::P_ERROR, true, "%d invalid lines before .orig", cur_pos);
#endif
	logger.newline();
#ifndef _LIBERAL_ASM
        throw utils::exception("orig is not first line in program");
#endif
    }

    // start at the statement right after the first orig
    cur_pos += 1;

    // once the first valid orig is found, mark the remaining statements
    for(uint32_t i = cur_pos; i < statements.size(); i += 1) {
        Statement & state = statements[i];

        if(cur_pc >= MMIO_START) {
            logger.asmPrintf(PrintType::P_ERROR, 0, state.line.size(), state.label, "no more room in writeable memory");
            logger.newline();
            throw utils::exception("no more room in writeable memory");
        }

        state.pc = cur_pc;
        for(StatementToken & operand : state.operands) {
            operand.pc = cur_pc;
        }

        if(checkIfValidPseudoStatement(state, ".blkw", logger, true)) {
            cur_pc += state.operands[0].num;
        } else if(checkIfValidPseudoStatement(state, ".stringz", logger, true)) {
            cur_pc += state.operands[0].str.size() + 1;
        } else if(checkIfValidPseudoStatement(state, ".orig", logger, true)) {
            uint32_t val = state.operands[0].num;
            uint32_t trunc_val = val & 0xffff;
            if(val != trunc_val) {
#ifdef _LIBERAL_ASM
		PrintType p_type = PrintType::P_WARNING;
#else
		PrintType p_type = PrintType::P_ERROR;
#endif
		logger.asmPrintf(p_type, state.operands[0], "truncating address to 0x%0.4x", trunc_val);
		logger.newline();
#ifndef _LIBERAL_ASM
		throw utils::exception("could not find valid .orig");
#endif
            }

            cur_pc = trunc_val;
        } else {
            if(! state.isLabel()) {
                cur_pc += 1;
            }
        }
    }
}

uint32_t Assembler::encodeInstruction(asmbl::Statement const & state, SymbolTable const & symbol_table,
    lc3::utils::AssemblerLogger & logger, bool & success)
{
    using namespace asmbl;
    using namespace lc3::utils;

    std::string stripped_line = state.line;
    stripped_line.erase(stripped_line.begin(), std::find_if(stripped_line.begin(), stripped_line.end(), [](int c) {
        return ! std::isspace(c);
    }));
    StatementToken const & inst = state.inst_or_pseudo;

    // get candidates
    // first element in pair is the candidate
    // second element is the distance from the candidate
    using Candidate = std::pair<PIInstruction, uint32_t>;

    std::vector<Candidate> candidates = encoder.getInstructionCandidates(state);
    std::sort(std::begin(candidates), std::end(candidates), [](Candidate a, Candidate b) {
        return std::get<1>(a) < std::get<1>(b);
    });

    // there is an exact match iff only one candidate was found and its distance is 0
    // otherwise, list top 3 possibilities
    if(! (candidates.size() == 1 && std::get<1>(candidates[0]) == 0)) {
        if(inst.lev_dist == 0) {
            // instruction matched perfectly, but operands did not
            logger.asmPrintf(PrintType::P_ERROR, inst, "invalid usage of \'%s\' instruction", inst.str.c_str());
        } else {
            // instruction didn't match perfectly
            logger.asmPrintf(PrintType::P_ERROR, inst, "invalid instruction");
        }
        // list out possibilities
        uint32_t count = 0;
        for(auto candidate : candidates) {
            logger.printf(PrintType::P_NOTE, false, "did you mean \'%s\'?",
                std::get<0>(candidate)->toFormatString().c_str());
            count += 1;
            if(count >= 3) {
                break;
            }
        }
        if(candidates.size() > 3) {
            logger.printf(PrintType::P_NOTE, false, "...other possible options hidden");
        }
        logger.newline();
        success = false;
        return 0;
    }

    // if we've made it here, we've found a valid instruction
    logger.printf(PrintType::P_EXTRA, true, "%s", stripped_line.c_str());
    uint32_t encoding = encoder.encodeInstruction(state, std::get<0>(candidates[0]), symbol_table, logger, success);
    if(success) {
        logger.printf(PrintType::P_EXTRA, true, " => 0x%0.4x", encoding);
        return encoding;
    }

    return 0;
}

bool Assembler::checkIfValidPseudoToken(asmbl::StatementToken const & tok, std::string const & check)
{
    using namespace asmbl;

    if(tok.type == TokenType::PSEUDO) {
        std::string temp = tok.str;
        std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
        return temp == check;
    }

    return false;
}

bool Assembler::checkIfValidPseudoStatement(asmbl::Statement const & state, std::string const & check,
    lc3::utils::AssemblerLogger & logger, bool log_enable)
{
    using namespace asmbl;
    using namespace lc3::utils;

    if(! state.isPseudo()) { return false; }
    if(state.inst_or_pseudo.str != check) { return false; }

    std::vector<std::vector<TokenType>> valid_operands = {{TokenType::NUM}};
    if(check == ".stringz") {
        valid_operands = {{TokenType::STRING}};
    } else if(check == ".end") {
        valid_operands = {};
    } else if(check == ".fill") {
        valid_operands[0].emplace_back(TokenType::LABEL);
    }

    if(state.operands.size() != valid_operands.size()) {
        if(log_enable) {
            logger.asmPrintf(PrintType::P_ERROR, state.inst_or_pseudo, "incorrect number of operands");
            logger.newline();
        }
        return false;
    }

    for(uint32_t i = 0; i < valid_operands.size(); i += 1) {
        bool valid = false;
        for(TokenType it : valid_operands[i]) {
            if(state.operands[i].type == it) {
                valid = true;
                break;
            }
        }

        if(! valid) {
            if(log_enable) {
                logger.asmPrintf(PrintType::P_ERROR, state.operands[i], "invalid operand");
                logger.newline();
            }
            return false;
        }
    }

    return true;
}
