#ifndef INSTRUCTION_DECODER_H
#define INSTRUCTION_DECODER_H

namespace core
{
    class InstructionDecoder : public InstructionHandler
    {
    public:
        InstructionDecoder(void);

        bool findInstructionByEncoding(uint32_t encoding, Instruction *& candidate) const;
        void decode(uint32_t encoded_inst, Instruction & inst);
    private:
        std::map<uint32_t, std::vector<Instruction const *>> instructions_by_opcode;
    };
};

#endif
