#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

namespace core
{
    class InstructionEncoder : public InstructionHandler
    {
    public:
        InstructionEncoder(void);

        bool findInstructionByName(std::string const & search) const;
        bool findReg(std::string const & search) const;
        bool findInstruction(Token const * search, std::vector<IInstruction const *> & candidates) const;

        // precondition: the instruction is of type pattern and is valid (no error checking)
        void encodeInstruction(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, IInstruction const * pattern, Token const * inst,
            uint32_t & encoded_instruction, std::map<std::string, uint32_t> const & labels) const;
    private:
        std::map<std::string, std::vector<IInstruction const *>> instructions_by_name;
    };
};

#endif
