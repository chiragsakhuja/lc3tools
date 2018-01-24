#include <algorithm>
#include <iostream> // TODO: remove
#include <fstream>
#include <sstream>
#include <vector>

#ifdef _ENABLE_DEBUG
    #include <chrono>
#endif

#include "assembler.h"
#include "device_regs.h"
#include "tokens.h"
#include "tokenizer.h"

void lc3::core::Assembler::assemble(std::string const & asm_filename, std::string const & obj_filename)
{
    using namespace asmbl;
    using namespace lc3::utils;

    std::map<std::string, uint32_t> symbols;
    lc3::utils::AssemblerLogger logger(printer, print_level, asm_filename);

    // check if file exists
    std::ifstream file(asm_filename);
    if(! file.is_open()) {
        logger.printf(PrintType::WARNING, true, "skipping file %s ...", asm_filename.c_str());
        return;
    }
    file.close();

#ifdef _ENABLE_DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif

    logger.printf(PrintType::INFO, true, "attemping to assemble \'%s\' into \'%s\'", asm_filename.c_str(),
        obj_filename.c_str());

    // build statements from tokens
    Tokenizer tokenizer(asm_filename);
    std::vector<asmbl::Statement> statements;
    while(! tokenizer.isDone()) {
        std::vector<Token> tokens;
        Token token;
        while(! (tokenizer >> token) && token.type != TokenType::EOS) {
            if(token.type != asmbl::TokenType::EOS) {
                tokens.push_back(token);
            }
        }

        statements.push_back(makeStatement(tokens));
    }

    markPC(statements, logger);
    for(asmbl::Statement i : statements) {
        std::cout << i;
    }

#ifdef _ENABLE_DEBUG
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    logger.printf(lc3::utils::PrintType::EXTRA, false, "elapsed time: %f ms", elapsed * 1000);
#endif

/*
 *    std::map<std::string, uint32_t> symbols;
 *    lc3::utils::AssemblerLogger logger(printer, print_level);
 *
 *    FILE * orig_file = fopen(asm_filename.c_str(), "r");
 *
 *    if(orig_file == nullptr) {
 *        logger.printf(lc3::utils::PrintType::WARNING, true, "skipping file %s ...", asm_filename.c_str());
 *    } else {
 *        row_num = 0;
 *        col_num = 0;
 *
 *        // FIXME
 *        // this is a terrible hack that adds a newline to the end of the file because flex
 *        // can't properly parse EOF without a newline (aka I'm really bad at writing a lexer)
 *        std::ifstream orig_file_stream(asm_filename);
 *        std::stringstream buffer;
 *        buffer << orig_file_stream.rdbuf() << "\n";
 *        std::string mod_filename = "." + asm_filename;
 *        std::ofstream mod_file_stream(mod_filename);
 *        mod_file_stream << buffer.rdbuf();
 *        mod_file_stream.close();
 *        
 *        yyin = fopen(mod_filename.c_str(), "r");
 *        yyparse();
 *
 *        remove(mod_filename.c_str());
 *
 *        logger.printf(lc3::utils::PrintType::INFO, true, "assembling \'%s\' into \'%s\'", asm_filename.c_str(),
 *            obj_filename.c_str());
 *
 *        logger.filename = asm_filename;
 *        logger.asm_blob = readFile(asm_filename);
 *        std::vector<Statement> obj_file_blob = assembleChain(root, symbols, logger);
 *
 *        fclose(yyin);
 *
 *        std::ofstream obj_file(obj_filename);
 *        if(! obj_file) {
 *            logger.printf(lc3::utils::PrintType::ERROR, true, "could not open file \'%s\' for writing", obj_filename.c_str());
 *            throw utils::exception("could not open file");
 *        }
 *
 *        for(Statement i : obj_file_blob) {
 *            obj_file << i;
 *        }
 *
 *        obj_file.close();
 *    }
 */
}

lc3::core::asmbl::Statement lc3::core::Assembler::makeStatement(std::vector<asmbl::Token> const & tokens)
{ 
    std::vector<asmbl::StatementToken> ret_tokens;
    asmbl::Statement ret;

    for(asmbl::Token const & token : tokens) {
        ret_tokens.push_back(asmbl::StatementToken(token));
    }

    // shouldn't happen, but just in case...
    if(tokens.size() == 0) { return ret; }

    markRegAndPseudoTokens(ret_tokens);
    markInstTokens(ret_tokens);
    markLabelTokens(ret_tokens);
    ret = makeStatementFromTokens(ret_tokens);

    return ret;
}

void lc3::core::Assembler::markRegAndPseudoTokens(std::vector<asmbl::StatementToken> & tokens)
{
    for(asmbl::StatementToken & token : tokens) {
        if(token.type == asmbl::TokenType::STRING) {
            if(token.str.size() > 0 && token.str[0] == '.') {
                token.type = asmbl::TokenType::PSEUDO;
            } else if(encoder.checkIfReg(token.str)) {
                token.type = asmbl::TokenType::REG;
            }
        }
    }
}

void lc3::core::Assembler::markInstTokens(std::vector<asmbl::StatementToken> & tokens)
{
    if(tokens.size() == 1) {
        if(tokens[0].type == asmbl::TokenType::STRING) {
            uint32_t token_0_dist = encoder.getDistanceToNearestInstructionName(tokens[0].str);
            if(token_0_dist == 0) {
                tokens[0].type = asmbl::TokenType::INST;
                tokens[0].lev_dist = token_0_dist;
            } else {
                // there's only one string on this line and it's not exactly an instruction, so assume it's a label
            }
        } else {
            // the sole token on this line is not a string, so it cannot be an instruction
        }
    } else {
        if(tokens[0].type == asmbl::TokenType::STRING) {
            uint32_t token_0_dist = encoder.getDistanceToNearestInstructionName(tokens[0].str);
            if(tokens[1].type == asmbl::TokenType::STRING) {
                // first two tokens are both strings, maybe they're instructions?
                uint32_t token_1_dist = encoder.getDistanceToNearestInstructionName(tokens[1].str);
                // see which is closer to an instruction
                // if they're the same, lean toward thinking first token is the instruction
                // imagine a case like 'jsr jsr'; the appropriate error message should be 'jsr is not a label'
                // and for that to be the case, the first token should be marked as the instruction
                if(token_1_dist < token_0_dist) {
                    uint32_t lev_thresh = 1;
                    if(tokens.size() >= 3) {
                        if(tokens[2].type == asmbl::TokenType::PSEUDO) {
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
                        tokens[1].type = asmbl::TokenType::INST;
                        tokens[1].lev_dist = token_1_dist;
                    } else {
                        // too far from an instruction
                    }
                } else {
                    if(token_0_dist < 3) {
                        tokens[0].type = asmbl::TokenType::INST;
                        tokens[0].lev_dist = token_0_dist;;
                    } else {
                        // too far from an instruction
                    }
                }
            } else {
                uint32_t lev_thresh = 1;
                if(tokens[1].type == asmbl::TokenType::PSEUDO) {
                    // if the second token is a pseudo op, then the first token should be considered a label, even if
                    // it matches an instruction
                    tokens[0].type = asmbl::TokenType::LABEL;
                    return;
                } else {
                    lev_thresh += 1;
                }
                if(token_0_dist <= lev_thresh) {
                    tokens[0].type = asmbl::TokenType::INST;
                    tokens[0].lev_dist = token_0_dist;
                }
            }
        } else {
            // the line starts with something other than a string...so the second token cannot be an instruction
        }
    }
}

void lc3::core::Assembler::markLabelTokens(std::vector<asmbl::StatementToken> & tokens)
{
    if(tokens.size() > 0 && tokens[0].type == asmbl::TokenType::STRING) {
        tokens[0].type = asmbl::TokenType::LABEL;
    }

    // mark any strings after an inst as labels
    bool found_inst = false;
    for(asmbl::StatementToken & token : tokens) {
        if(found_inst && token.type == asmbl::TokenType::STRING) {
            token.type = asmbl::TokenType::LABEL;
        }

        if(token.type == asmbl::TokenType::INST) {
            found_inst = true;
        }
    }
}

lc3::core::asmbl::Statement lc3::core::Assembler::makeStatementFromTokens(std::vector<asmbl::StatementToken> & tokens)
{
    asmbl::Statement ret;
    if(tokens.size() > 0) {
        ret.line = tokens[0].line;
        if(tokens[0].type == asmbl::TokenType::LABEL) {
            asmbl::StatementToken temp = tokens[0];
            std::transform(temp.str.begin(), temp.str.end(), temp.str.begin(), ::tolower);
            ret.label = temp;
        }
    }

    uint32_t pos = 0;
    for(pos = 0; pos < tokens.size(); pos += 1) {
        if(tokens[pos].type == asmbl::TokenType::INST || tokens[pos].type == asmbl::TokenType::PSEUDO) {
            asmbl::StatementToken temp = tokens[pos];
            std::transform(temp.str.begin(), temp.str.end(), temp.str.begin(), ::tolower);

            ret.inst_or_pseudo = temp;
            pos += 1;
            break;
        }
    }

    for( ; pos < tokens.size(); pos += 1) {
        asmbl::StatementToken temp = tokens[pos];
        if(temp.type != asmbl::TokenType::STRING) {
            std::transform(temp.str.begin(), temp.str.end(), temp.str.begin(), ::tolower);
        }

        ret.operands.push_back(tokens[pos]);
    }

    return ret;
}

void lc3::core::Assembler::markPC(std::vector<asmbl::Statement> & statements, lc3::utils::AssemblerLogger & logger)
{
    using namespace lc3::utils;
    using namespace asmbl;

    uint32_t cur_pc = 0;
    uint32_t cur_pos = 0;
    bool found_orig = false;

    while(! found_orig && cur_pos < statements.size()) {
        while(cur_pos < statements.size())  {
            asmbl::Statement const & state = statements[cur_pos];

            if(checkIfValidPseudo(state, ".orig", logger)) {
                found_orig = true;
                break;
            }

            logger.printf(PrintType::EXTRA, true, "ignoring line \'%s\' before .orig", state.line.c_str());
            cur_pos += 1;
        }

        if(cur_pos == statements.size()) {
            break;
        }

        asmbl::Statement const & state = statements[cur_pos];

        uint32_t val = state.operands[0].num;
        uint32_t trunc_val = val & 0xffff;
        if(val != trunc_val) {
            logger.asmPrintf(PrintType::WARNING, state.operands[0], "truncating address to 0x%0.4x", trunc_val);
            logger.newline();
        }

        cur_pc = trunc_val;
        found_orig = true;
    }

    if(! found_orig) {
        logger.printf(PrintType::ERROR, true, "could not find valid .orig", cur_pos);
        throw utils::exception("could not find valid .orig");
    }

    if(cur_pos != 0) {
        logger.printf(PrintType::WARNING, true, "ignoring %d lines before .orig", cur_pos);
    }

    // start at the statemetn right after the first orig
    cur_pos += 1;

    // once the first valid orig is found, mark the remaining statements
    for(uint32_t i = cur_pos; i < statements.size(); i += 1) {
        asmbl::Statement & state = statements[i];

        if(cur_pc >= MMIO_START) {
            logger.asmPrintf(PrintType::ERROR, 0, state.line.size(), state.label, "no more room in writeable memory");
            logger.newline();
            throw utils::exception("no more room in writeable memory");
        }

        state.pc = cur_pc;

        if(checkIfValidPseudo(state, ".blkw", logger)) {
            cur_pc += state.operands[0].num;
        } else if(checkIfValidPseudo(state, ".stringz", logger)) {
            cur_pc += state.operands[0].str.size() + 1;
        } else if(checkIfValidPseudo(state, ".orig", logger)) {
            uint32_t val = state.operands[0].num;
            uint32_t trunc_val = val & 0xffff;
            if(val != trunc_val) {
                logger.asmPrintf(PrintType::WARNING, state.operands[0], "truncating address to 0x%0.4x", trunc_val);
                logger.newline();
            }

            cur_pc = trunc_val;
        } else {
            cur_pc += 1;
        }
    }
}

bool lc3::core::Assembler::checkIfValidPseudo(asmbl::Statement const & state, std::string const & check,
    lc3::utils::AssemblerLogger & logger)
{
    using namespace lc3::utils;
    using namespace asmbl;

    if(! state.isPseudo()) { return false; }
    if(state.inst_or_pseudo.str != check) { return false; }

    std::vector<asmbl::TokenType> valid_operands = {asmbl::TokenType::NUM};
    if(check == ".stringz") {
        valid_operands = {asmbl::TokenType::STRING};
    } else if(check == ".end") {
        valid_operands = {};
    }

    if(state.operands.size() != valid_operands.size()) {
        logger.asmPrintf(PrintType::ERROR, state.inst_or_pseudo, "incorrect number of operands");
        logger.newline();
        return false;
    }

    for(uint32_t i = 0; i < valid_operands.size(); i += 1) {
        if(state.operands[i].type != valid_operands[i]) {
            logger.asmPrintf(PrintType::ERROR, state.operands[i], "invalid operand");
            logger.newline();
            return false;
        }
    }

    return true;
}

/*
 *
 *std::vector<lc3::core::Statement> lc3::core::Assembler::assembleChain(Token * program,
 *    std::map<std::string, uint32_t> & symbols, lc3::utils::AssemblerLogger & logger)
 *{
 *    logger.printf(lc3::utils::PrintType::INFO, true, "beginning first pass ...");
 *
 *    Token * program_start = nullptr;
 *
 *    try {
 *        program_start = firstPass(program, symbols, logger);
 *    } catch(utils::exception const & e) {
 *        logger.printf(lc3::utils::PrintType::ERROR, true, "first pass failed");
 *        throw e;
 *    }
 *
 *    logger.printf(lc3::utils::PrintType::INFO, true, "first pass completed successfully, beginning second pass ...");
 *
 *    std::vector<Statement> ret;
 *    try {
 *        ret = secondPass(program_start, symbols, logger);
 *    } catch(utils::exception const & e) {
 *        logger.printf(lc3::utils::PrintType::ERROR, true, "second pass failed");
 *        throw e;
 *    }
 *
 *    logger.printf(lc3::utils::PrintType::INFO, true, "second pass completed successfully");
 *
 *    return ret;
 *}
 *
 *Token * lc3::core::Assembler::firstPass(Token * program, std::map<std::string, uint32_t> & symbols,
 *    lc3::utils::AssemblerLogger & logger)
 *{
 *    // TODO: make sure we aren't leaking tokens by changing the program start
 *    Token * program_start = removeNewlineTokens(program);
 *    toLower(program_start);
 *    separateLabels(program_start, logger);
 *    program_start = findOrig(program_start, logger);
 *    processStatements(program_start, logger);
 *    saveSymbols(program_start, symbols, logger);
 *
 *    return program_start;
 *}
 *
 *Token * lc3::core::Assembler::removeNewlineTokens(Token * program)
 *{
 *    Token * program_start = program;
 *    Token * prev_tok = nullptr;
 *    Token * cur_tok = program;
 *
 *    // remove newline toks
 *    while(cur_tok != nullptr) {
 *        bool del_cur_tok = false;
 *        if(cur_tok->type == NEWLINE) {
 *            if(prev_tok != nullptr) {
 *                prev_tok->next = cur_tok->next;
 *            } else {
 *                // if we start off with newlines, move the program pointer forward
 *                program_start = cur_tok->next;
 *            }
 *            del_cur_tok = true;
 *        } else {
 *            prev_tok = cur_tok;
 *        }
 *
 *        Token * next_tok = cur_tok->next;
 *        if(del_cur_tok) {
 *            delete cur_tok;
 *        }
 *        cur_tok = next_tok;
 *    }
 *
 *
 *    // since you may have moved the program pointer, you need to return it
 *    return program_start;
 *}
 */

/*
 *void lc3::core::Assembler::toLower(Token * token_chain)
 *{
 *    Token * cur_token = token_chain;
 *    while(cur_token != nullptr) {
 *        if(cur_token->type != NUM) {
 *            std::string & cur_str = cur_token->str;
 *            std::transform(cur_str.begin(), cur_str.end(), cur_str.begin(), ::tolower);
 *            if(! cur_token->checkPseudoType("stringz")) {
 *                toLower(cur_token->opers);
 *            }
 *        }
 *        cur_token = cur_token->next;
 *    }
 *}
 */
/*
 *
 *void lc3::core::Assembler::separateLabels(Token * program, lc3::utils::AssemblerLogger & logger)
 *{
 *    Token * cur_tok = program;
 *    // since the parser can't distinguish between an instruction and a label by design,
 *    // we need to do it while analyzing the tokens using a simple rule: if the first INST
 *    // of a chain of tokens is not a valid instruction, assume it's a label
 *    while(cur_tok != nullptr) {
 *        std::vector<IInstruction const *> candidates;
 *        // if the token was labeled an instruction and it's not even a candidate for an instruction,
 *        // assume it's a label
 *        if((cur_tok->type == INST && ! encoder.findInstruction(cur_tok, candidates) &&
 *                candidates.size() == 0) ||
 *            cur_tok->type == LABEL)
 *        {
 *            cur_tok->type = LABEL;
 *            if(cur_tok->opers != nullptr) {
 *                Token * upgrade_tok = cur_tok->opers;
 *                // if there is something after the label that the parser marked as an operand
 *                if(upgrade_tok->type == PSEUDO || encoder.findInstructionByName(upgrade_tok->str)) {
 *                    if(upgrade_tok->type != PSEUDO) {
 *                        upgrade_tok->type = INST;
 *                        // if it is a pseduo-op, then the opers are already correct because of parser behavior
 *                        // if not, opers is a nullptr because the opers are really in the next pointer
 *                        upgrade_tok->opers = upgrade_tok->next;
 *                        // recount operands (could just subtract 1, but do this just in case)
 *                        uint32_t num_opers = 0;
 *                        Token * cur_oper = upgrade_tok->opers;
 *                        while(cur_oper != nullptr) {
 *                            num_opers += 1;
 *                            cur_oper = cur_oper->next;
 *                        }
 *                        upgrade_tok->num_opers = num_opers;
 *                    }
 *                    // elevate the token to a proper token in the chain
 *                    upgrade_tok->next = cur_tok->next;
 *                    cur_tok->next = upgrade_tok;
 *                    cur_tok->opers = nullptr;
 *                    cur_tok->num_opers = 0;
 *                } else {
 *                    logger.printfMessage(lc3::utils::PrintType::WARNING, cur_tok,
 *                        "\'%s\' is being interpreted as a label, did you mean for it to be an instruction?",
 *                        cur_tok->str.c_str());
 *                    logger.newline();
 *                }
 *            }
 *        }
 *        cur_tok = cur_tok->next;
 *    }
 *}
 *
 */
/*
 *Token * lc3::core::Assembler::findOrig(Token * program, lc3::utils::AssemblerLogger & logger)
 *{
 *    Token * program_start = program;
 *    Token * cur_tok = program;
 *    bool found_valid_orig = false;
 *    uint32_t invalid_statement_count = 0;
 *    while(! found_valid_orig && cur_tok != nullptr) {
 *        if(! cur_tok->checkPseudoType("orig")) {
 *            if(cur_tok->type != LABEL) {
 *                invalid_statement_count += 1;
 *            }
 *        } else {
 *            found_valid_orig = true;
 *            if(cur_tok->num_opers != 1) {
 *                logger.printfMessage(lc3::utils::PrintType::ERROR, cur_tok, "incorrect number of operands");
 *                logger.newline();
 *                throw utils::exception("incorrect number of operands to .orig");
 *            }
 *
 *            if(cur_tok->opers->type != NUM) {
 *                logger.printfMessage(lc3::utils::PrintType::ERROR, cur_tok->opers, "illegal operand");
 *                logger.newline();
 *                throw utils::exception("illegal operand to .orig");
 *            }
 *
 *            // TODO: use encode function
 *            uint32_t oper_val = (uint32_t) cur_tok->opers->num;
 *            uint32_t trunc_oper_val =((uint32_t) oper_val) & 0xffff;
 *            if(oper_val > 0xffff) {
 *                logger.printfMessage(lc3::utils::PrintType::WARNING, cur_tok->opers, "truncating 0x%0.8x to 0x%0.4x",
 *                    oper_val, trunc_oper_val);
 *                logger.newline();
 *            }
 *            cur_tok->pc = trunc_oper_val;
 *            program_start = cur_tok;
 *        }
 *
 *        cur_tok = cur_tok->next;
 *    }
 *
 *    if(! found_valid_orig) {
 *        logger.printf(lc3::utils::PrintType::ERROR, true, "could not find valid .orig in program");
 *        throw utils::exception("could not find valid .orig");
 *    } else {
 *        if(invalid_statement_count > 0) {
 *            logger.printf(lc3::utils::PrintType::WARNING, true, "ignoring %d statements before .orig", invalid_statement_count);
 *        }
 *    }
 *
 *    return program_start;
 *}
 */

/*
 * precondition: first token is valid .orig
 *void lc3::core::Assembler::processStatements(Token * program, lc3::utils::AssemblerLogger & logger)
 *{
 *    uint32_t pc = program->pc;
 *    uint32_t pc_offset = 0;
 *    Token * cur_tok = program->next;
 *    while(cur_tok != nullptr) {
 *        cur_tok->pc = pc + pc_offset;
 *
 *        // assign pc to opernads too
 *        Token * cur_oper = cur_tok->opers;
 *        while(cur_oper != nullptr) {
 *            cur_oper->pc = cur_tok->pc;
 *            cur_oper = cur_oper->next;
 *        }
 *
 *        if(cur_tok->type == INST) {
 *            processInstOperands(cur_tok);
 *            pc_offset += 1;
 *        } else if(cur_tok->type == PSEUDO) {
 *            // don't do any error checking, just ignore the pseduo op if it doesn't meet the requirements
 *            // we'll halt the assembler anyway if there is something wrong
 *            if(cur_tok->str == "fill") {
 *                if(cur_tok->num_opers == 1) {
 *                    pc_offset += 1;
 *                }
 *            } else if(cur_tok->str == "stringz") {
 *                if(cur_tok->num_opers == 1) {
 *                    processStringzOperands(cur_tok);
 *                    pc_offset += cur_tok->opers->str.size() + 1;
 *                }
 *            } else if(cur_tok->str == "blkw") {
 *                if(cur_tok->num_opers == 1 && cur_tok->opers->type == NUM) {
 *                    pc_offset += cur_tok->opers->num;
 *                }
 *            } else if(cur_tok->str == "end") {
 *                cur_tok = cur_tok->next;
 *                break;
 *            }
 *        }
 *        cur_tok = cur_tok->next;
 *    }
 *
 *    // TODO: delete remaining tokens
 *    uint32_t leftover_statement_count = 0;
 *    while(cur_tok != nullptr) {
 *        leftover_statement_count += 1;
 *        cur_tok = cur_tok->next;
 *    }
 *
 *    if(leftover_statement_count > 0) {
 *        logger.printf(lc3::utils::PrintType::WARNING, true, "ignoring %d statements after .end", leftover_statement_count);
 *    }
 *}
 */

/*
 *void lc3::core::Assembler::processInstOperands(Token * inst)
 *{
 *    Token * oper = inst->opers;
 *    // reassign operand types
 *    while(oper != nullptr) {
 *        if(oper->type == STRING) {
 *            if(encoder.findReg(oper->str)) {
 *                oper->type = static_cast<int>(OperType::OPER_TYPE_REG);
 *            } else {
 *                oper->type = static_cast<int>(OperType::OPER_TYPE_LABEL);
 *            }
 *        } else if(oper->type == NUM) {
 *            oper->type = static_cast<int>(OperType::OPER_TYPE_NUM);
 *        }
 *
 *        oper = oper->next;
 *    }
 *}
 *
 *void lc3::core::Assembler::processStringzOperands(Token * stringz)
 *{
 *    Token * oper = stringz->opers;
 *    if(oper->type == STRING) {
 *        std::stringstream new_str;
 *        std::string value = oper->str;
 *
 *        if(value[0] == '"') {
 *            if(value[value.size() - 1] == '"') {
 *                value = value.substr(1, value.size() - 2);
 *            } else {
 *                value = value.substr(1);
 *            }
 *        }
 *
 *        for(uint32_t i = 0; i < value.size(); i += 1) {
 *            char char_value = value[i];
 *            if(char_value == '\\' && i + 1 < value.size()) {
 *                if(value[i + 1] == 'n') {
 *                    char_value = '\n';
 *                }
 *                i += 1;
 *            }
 *            new_str << char_value;
 *        }
 *        oper->str = new_str.str();
 *    } else {
 *        oper->type = STRING;
 *        oper->str = std::to_string(oper->num);
 *    }
 *}
 *
 *void lc3::core::Assembler::saveSymbols(Token * program, std::map<std::string, uint32_t> & symbols,
 *    lc3::utils::AssemblerLogger & logger)
 *{
 *    Token * cur_tok = program;
 *    while(cur_tok != nullptr) {
 *        if(cur_tok->type == LABEL) {
 *            std::string const & label = cur_tok->str;
 *
 *            if(symbols.find(label) != symbols.end()) {
 *                logger.printfMessage(lc3::utils::PrintType::WARNING, cur_tok, "redefining label \'%s\'", cur_tok->str.c_str());
 *                logger.newline();
 *            }
 *
 *            symbols[label] = cur_tok->pc;
 *
 *            logger.printf(lc3::utils::PrintType::DEBUG, true, "setting label \'%s\' to 0x%X", label.c_str(), cur_tok->pc);
 *        }
 *        cur_tok = cur_tok->next;
 *    }
 *}
 *
 * precondition: first token is orig
 *std::vector<lc3::core::Statement> lc3::core::Assembler::secondPass(Token * program,
 *    std::map<std::string, uint32_t> symbols, lc3::utils::AssemblerLogger & logger)
 *{
 *    bool success = true;
 *    std::vector<Statement> ret;
 *    ret.emplace_back(program->pc, true, logger.asm_blob[program->row_num]);
 *
 *    Token * cur_tok = program->next;
 *    while(cur_tok != nullptr) {
 *        try {
 *            if(cur_tok->type == INST) {
 *                uint32_t encoded = encodeInstruction(cur_tok, symbols, logger);
 *                ret.emplace_back(encoded, false, logger.asm_blob[cur_tok->row_num]);
 *            } else if(cur_tok->type == PSEUDO) {
 *                std::vector<Statement> encoded = encodePseudo(cur_tok, symbols, logger);
 *                ret.insert(ret.end(), encoded.begin(), encoded.end());
 *            }
 *        } catch(utils::exception const & e) {
 *            success = false;
 *        }
 *        cur_tok = cur_tok->next;
 *    }
 *
 *    if(! success) {
 *        throw utils::exception("second pass failed");
 *    }
 *
 *    return ret;
 *}
 *
 *uint32_t lc3::core::Assembler::encodeInstruction(Token * inst, std::map<std::string, uint32_t> symbols,
 *    lc3::utils::AssemblerLogger & logger)
 *{
 *    std::vector<IInstruction const *> candidates;
 *    if(encoder.findInstruction(inst, candidates)) {
 *        uint32_t encoding = encoder.encodeInstruction(candidates[0], inst, symbols, logger);
 *        logger.printf(lc3::utils::PrintType::DEBUG, true, "%s => %s", logger.asm_blob[inst->row_num].c_str(),
 *            utils::udecToBin(encoding, 16).c_str());
 *        return encoding;
 *    }
 *
 *    if(candidates.size() == 0) {
 *        // this shouldn't happen, because if there are no candidates it should've been retyped as a LABEL
 *        logger.printfMessage(lc3::utils::PrintType::ERROR, inst, "\'%s\' is not a valid instruction", inst->str.c_str());
 *        logger.newline();
 *        throw utils::exception("could not find a valid candidate for instruction");
 *    }
 *
 *    logger.printfMessage(lc3::utils::PrintType::ERROR, inst, "not a valid usage of \'%s\' instruction", inst->str.c_str());
 *    for(IInstruction const * candidate : candidates) {
 *        logger.printf(lc3::utils::PrintType::NOTE, false, "did you mean \'%s\'?", candidate->toFormatString().c_str());
 *    }
 *    logger.newline();
 *
 *    throw utils::exception("matched instruction with a candidate, but some operands were incorrect");
 *}
 *
 *std::vector<lc3::core::Statement> lc3::core::Assembler::encodePseudo(Token * pseudo,
 *    std::map<std::string, uint32_t> symbols, lc3::utils::AssemblerLogger & logger)
 *{
 *    std::vector<Statement> ret;
 *
 *    // TODO: is it worth making this like the instruction encoder?
 *    if(pseudo->str == "fill") {
 *        Token * oper = pseudo->opers;
 *        if(pseudo->num_opers != 1 || (oper->type != NUM && oper->type != STRING)) {
 *            logger.printfMessage(lc3::utils::PrintType::ERROR, pseudo, "not a valid usage of .fill pseudo-op");
 *            logger.printf(lc3::utils::PrintType::NOTE, false, "did you mean \'.fill num\'?");
 *            logger.printf(lc3::utils::PrintType::NOTE, false, "did you mean \'.fill label\'?");
 *            logger.newline();
 *            throw utils::exception("not a valid usage of .fill pseudo-op");
 *        }
 *
 *        if(oper->type == NUM) {
 *            ret.emplace_back(oper->num, false, logger.asm_blob[oper->row_num]);
 *        } else if(oper->type == STRING) {
 *            auto search = symbols.find(oper->str);
 *            if(search != symbols.end()) {
 *                ret.emplace_back((uint32_t) search->second, false, logger.asm_blob[oper->row_num]);
 *            } else {
 *                logger.printfMessage(lc3::utils::PrintType::ERROR, oper, "unknown label \'%s\'", oper->str.c_str());
 *                logger.newline();
 *                throw std::runtime_error("unknown label");
 *            }
 *        }
 *    } else if(pseudo->str == "stringz") {
 *        Token * oper = pseudo->opers;
 *        if(pseudo->num_opers != 1 || (oper->type != NUM && oper->type != STRING)) {
 *            logger.printfMessage(lc3::utils::PrintType::ERROR, pseudo, "not a valid usage of .stringz pseudo-op");
 *            logger.printf(lc3::utils::PrintType::NOTE, false, "did you mean \'.stringz string\'?");
 *            logger.newline();
 *            throw std::runtime_error("not a valid usage of .stringz pseudo-op");
 *        }
 *
 *        std::string value;
 *        if(oper->type == NUM) {
 *            value = std::to_string(oper->num);
 *            logger.printfMessage(lc3::utils::PrintType::WARNING, oper, "interpreting numeric value as decimal string \'%s\'",
 *                value.c_str());
 *            logger.newline();
 *        } else if(oper->type == STRING) {
 *            value = oper->str;
 *        }
 *
 *        bool first = true;
 *        for(char i : value) {
 *            ret.emplace_back(((uint32_t) i) & 0xff, false, first ? ("(start) " + logger.asm_blob[oper->row_num]) : "");
 *            first = false;
 *        }
 *        ret.emplace_back(0U, false, "(end) " + logger.asm_blob[oper->row_num]);
 *    } else if(pseudo->str == "blkw") {
 *        Token * oper = pseudo->opers;
 *        if(pseudo->num_opers != 1 || oper->type != NUM) {
 *            logger.printfMessage(lc3::utils::PrintType::ERROR, pseudo, "not a valid usage of .blkw pseudo-op");
 *            logger.printf(lc3::utils::PrintType::NOTE, false, "did you mean \'.blkw num\'?");
 *            logger.newline();
 *            throw std::runtime_error("not a valid usage of .blkw pseudo-op");
 *        }
 *
 *        for(uint32_t i = 0; i < (uint32_t) oper->num; i += 1) {
 *            ret.emplace_back(0U, false, (i == 0) ? logger.asm_blob[oper->row_num] : "");
 *        }
 *    }
 *
 *    return ret;
 *}
 */

