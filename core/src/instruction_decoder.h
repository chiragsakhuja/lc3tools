#ifndef INSTRUCTION_DECODER_H
#define INSTRUCTION_DECODER_H

namespace core
{
    class InstructionDecoder : public InstructionHandler
    {
    public:
        InstructionDecoder(void);

        bool findInstructionByEncoding(uint32_t encoding, IInstruction *& candidate) const;
    private:
        std::map<uint32_t, std::vector<IInstruction const *>> instructions_by_opcode;
    };
};

#endif
