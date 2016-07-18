#!/usr/bin/env python3

import json
import random
import math

# Generate a complete coverage test case based on core/res/encodings.json

def construct(opname, regs, gen_list):
    output = []
    # Recurse downward until there is just one field in gen_list. Use that to generate a base
    # encoding, and then step out. Now a 2nd field in gen_list will apply all of its encodings
    # to the array, in addition to adding many more entries.
    partial = [] if len(gen_list) == 1 else construct(opname, regs, gen_list[0:-1])

    hi = gen_list[-1]['hi']
    lo = gen_list[-1]['lo']
    values = gen_list[-1]['values']
    is_reg = gen_list[-1]['is_reg']
    display = gen_list[-1]['display']

    for value in values:
        bits = (value & ((1 << (hi - lo + 1)) - 1)) << lo
        if len(gen_list) == 1:
            encoded = bits
            decoded = opname
            output.append((encoded, decoded))
        else:
            for part in partial:
                encoded = part[0] | bits
                decoded = part[1]
                if display: decoded += ' ' + (regs[value] if is_reg else '%d' % value)
                output.append((encoded, decoded))

    return output

def main():
    with open('../core/res/encodings.json') as config_file:
        config = json.load(config_file)

    # Find the instructions and register listing
    regs = list(filter(lambda x: x['type'] == 'REGS', config))[0]['data']['reglist']
    insts = filter(lambda x: x['type'] == 'INST', config)

    with open('comprehensive_test.asm', 'w') as output:
        # Instruction permutations are independent of other instructions
        for inst in insts:
            opname = inst['data']['label']
            enc_parts = inst['data']['enc']
            # gen_list contains the target values for each of the fields in the instruction.
            # For example, for a register add instruction, the core of the gen_list array would look like:
            #    [{display: True , hi: 11, lo: 9 , pos: 0 , is_reg: True , values: [0, 1, ..., 7]},
            #     {display: True , hi: 8 , lo: 6 , pos: 1 , is_reg: True , values: [0, 1, ..., 7]},
            #     {display: True , hi: 2 , lo: 0 , pos: 2 , is_reg: True , values: [0, 1, ..., 7]},
            #     {display: False, hi: 15, lo: 12, pos: -1, is_reg: False, values: [1]},
            #     {display: False, hi: 5 , lo: 3 , pos: -1, is_reg: False, values: [0]}]
            #
            # Once the target values have been generated, the construct function generates all permutations
            # of the values to get complete coverage of the instruction.
            gen_list = []

            # Go through the encoding parts
            for enc_part in inst['data']['enc']:
                # Check if the field is predetermined (OPCODE and FIXED types) or not
                if enc_part['type'] != 'OPCODE' and enc_part['type'] != 'FIXED':
                    size = enc_part['hi'] - enc_part['lo'] + 1
                    is_reg = enc_part['type'] == 'REG'
                    pos = enc_part['data']['pos']
                    values = []

                    # Iterate over all possible values of the field and append them to a list
                    # This assumes all immediates can be signed
                    for i in range(0, 1 << size):
                        values.append(i - (1 << (size - 1)))
                    gen_list.append({'display': True, 'hi': enc_part['hi'], 'lo': enc_part['lo'], 'pos': pos, 'is_reg': is_reg, 'values': values})
                else:
                    gen_list.append({'display': False, 'hi': enc_part['hi'], 'lo': enc_part['lo'], 'pos': -1, 'is_reg': False, 'values': [int(enc_part['data']['value'], 2)]})
            gen_list.sort(key=lambda x: x['pos'])
            instructions = construct(opname, regs, gen_list)
            for (encoded, decoded) in instructions:
                output.write('0x%04x %s\n' % (encoded, decoded))

if __name__ == '__main__':
    main()
