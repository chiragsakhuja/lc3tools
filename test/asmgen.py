#!/usr/bin/python

from copy import copy
import random as rd

class Inst:
    def __init__(S, asm, bit_str):
        S.asm = asm
        S.bit_str = bit_str
    def setVal(S):
        return True
    def getAsmStr(S):
        return S.asm
    def getBitStr(S):
        return S.bit_str
    def __str__(S):
        return S.getAsmStr()

class Fixed:
    def __init__(S, bit_str):
        S.bit_str = bit_str
    def setVal(S):
        return True
    def getAsmStr(S):
        pass
    def getBitStr(S):
        return S.bit_str
    def __str__(S):
        return S.getAsmStr()

class Reg:
    def __init__(S):
        S.ready = False
    def setVal(S):
        S.val = rd.randint(0, 7)
        S.ready = True
        return S.ready
    def getAsmStr(S):
        if not S.ready: return None
        return 'r' + str(S.val)
    def getBitStr(S):
        if not S.ready: return None
        return '{:03b}'.format(S.val)
    def __str__(S):
        ret = S.getAsmStr()
        if not ret: return ''
        return ret

class Imm:
    def __init__(S, signed, bits, *constraints):
        if(signed):
            S.min = -pow(2, bits - 1)
            S.max =  pow(2, bits - 1) - 1
        else:
            S.min = 0
            S.max = pow(2, bits - 1) - 1
        S.bits = bits
        S.constraints = constraints
        S.ready = False
    def setVal(S):
        S.ready = False
        attempt_count = 0
        while not S.ready and attempt_count < 100:
            attempt_count += 1
            S.val = rd.randint(S.min, S.max)
            if S.constraints:
                for constraint in S.constraints:
                    S.ready |= not constraint(S.val)
                S.ready = not S.ready
            else:
                S.ready = True
        return S.ready
    def getAsmStr(S):
        if not S.ready: return None
        style = rd.uniform(0, 1)
        if style < 0.5:
            style = rd.uniform(0, 1)
            if style < 0.5:
                return 'x{:x}'.format(S.val)
            else:
                mask = (1 << S.bits) - 1
                return 'x{:x}'.format(S.val & mask)
        else:
            style = rd.uniform(0, 1)
            if style < 0.5:
                return '#{:d}'.format(S.val)
            else:
                return '{:d}'.format(S.val)
    def getBitStr(S):
        if not S.ready: return None
        mask = (1 << S.bits) - 1
        return ('{:0' + str(S.bits) + 'b}').format(S.val & mask)
    def __str__(S):
        ret = S.getAsmStr()
        if not ret: return ''
        return ret

def getAsmStr(ops):
    ret = ''
    for i, x in enumerate(ops):
        if isinstance(x, Fixed): continue
        ret += x.getAsmStr()
        if i < len(ops) - 1:
            ret += ' '
    return ret

def getBitStr(ops):
    ret = ''
    for x in ops:
        ret += x.getBitStr()
    return ret

def pickRandom(node):
    ret = list()
    if isinstance(node, tuple):
        for x in node:
            op = pickRandom(x)
            if not op: return None
            ret += op
    elif isinstance(node, list):
        idx = rd.randint(0, len(node) - 1)
        op = pickRandom(node[idx])
        if not op: return None
        ret += op
    else:
        node_inst = copy(node)
        res = node_inst.setVal()
        if not res: return None
        ret.append(node_inst)
    return ret

valid_inst_types = [(Inst('add', '0001'),
                     [(Reg(), Reg(), Fixed('000'), Reg()),
                      (Reg(), Reg(), Fixed('1'), Imm(True, 5))
                     ]
                    ),
                    (Inst('and', '0001'),
                     [(Reg(), Reg(), Fixed('000'), Reg()),
                      (Reg(), Reg(), Fixed('1'), Imm(True, 5))
                     ]
                    ),
                    ([Inst('br'   , '0000111'),
                      Inst('brn'  , '0000100'),
                      Inst('brz'  , '0000010'),
                      Inst('brp'  , '0000001'),
                      Inst('brnz' , '0000110'),
                      Inst('brnp' , '0000101'),
                      Inst('brzp' , '0000011'),
                      Inst('brnzp', '0000111')
                     ],
                     Imm(True, 9)
                    ),
                    ([(Inst('jmp', '1100'), Fixed('000'), Reg(), Fixed('00000')),
                      (Inst('ret', '1100000111000000'))
                     ]
                    ),
                    ([(Inst('jsr' , '0100'), Fixed('1')  , Imm(True, 11)),
                      (Inst('jsrr', '0100'), Fixed('000'), Reg(), Fixed('00000'))
                     ]
                    ),
                    (Inst('ld', '0010'), Reg(), Imm(True, 9)),
                    (Inst('ldi', '1010'), Reg(), Imm(True, 9)),
                    (Inst('ldr', '0110'), Reg(), Reg(), Imm(True, 6)),
                    (Inst('lea', '1110'), Reg(), Imm(True, 9)),
                    (Inst('not', '1001'), Reg(), Reg(), Fixed('111111')),
                    (Inst('rti', '1000000000000000')),
                    (Inst('st', '0011'), Reg(), Imm(True, 9)),
                    (Inst('sti', '1011'), Reg(), Imm(True, 9)),
                    (Inst('str', '0111'), Reg(), Reg(), Imm(True, 6)),
                    ([(Inst('trap', '1111'), Fixed('0000'), Imm(False, 8)),
                      (Inst('getc', '1111000000100000')),
                      (Inst('out' , '1111000000100001')),
                      (Inst('puts', '1111000000100010')),
                      (Inst('in'  , '1111000000100011')),
                      (Inst('halt', '1111000000100101'))
                     ]
                    )
                   ]

valid_orig = (Inst('.orig', ''), Imm(False, 16))
valid_end = (Inst('.end', ''))

opts = {'inst-count': 100,
        'orig-prob': 0.01}

program = list()
current_orig = pickRandom(valid_orig)
program.append(current_orig)
for i in range(opts['inst-count']):
    toss = rd.uniform(0, 1)
    if toss < opts['orig-prob']:
        current_orig = pickRandom(valid_orig)
        program.append(pickRandom(valid_end))
        program.append(current_orig)
    else:
        program.append(pickRandom(valid_inst_types))
program.append(pickRandom(valid_end))

for line in program:
    print('{} => {}'.format(getAsmStr(line), getBitStr(line)))

