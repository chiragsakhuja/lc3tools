/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "isa.h"
#include "logger.h"

using namespace lc3::core;

ADDRegInstruction::ADDRegInstruction(void) : IInstruction("add", {
    std::make_shared<FixedOperand>(4, 0x1),
    std::make_shared<RegOperand>(3),
    std::make_shared<RegOperand>(3),
    std::make_shared<FixedOperand>(3, 0x0),
    std::make_shared<RegOperand>(3)
}) { }

ADDImmInstruction::ADDImmInstruction(void) : IInstruction("add", {
    std::make_shared<FixedOperand>(4, 0x1),
    std::make_shared<RegOperand>(3),
    std::make_shared<RegOperand>(3),
    std::make_shared<FixedOperand>(1, 0x1),
    std::make_shared<NumOperand>(5, true)
}) { }

ANDRegInstruction::ANDRegInstruction(void) : IInstruction("and", {
    std::make_shared<FixedOperand>(4, 0x5),
    std::make_shared<RegOperand>(3),
    std::make_shared<RegOperand>(3),
    std::make_shared<FixedOperand>(3, 0x0),
    std::make_shared<RegOperand>(3)
}) { }

ANDImmInstruction::ANDImmInstruction(void) : IInstruction("and", {
    std::make_shared<FixedOperand>(4, 0x5),
    std::make_shared<RegOperand>(3),
    std::make_shared<RegOperand>(3),
    std::make_shared<FixedOperand>(1, 0x1),
    std::make_shared<NumOperand>(5, true)
}) { }

BRInstruction::BRInstruction(void) : IInstruction("br", {
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(3, 0x7),
    std::make_shared<LabelOperand>(9)
}) { }

BRnInstruction::BRnInstruction(void) : BRInstruction("brn", {
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(3, 0x4),
    std::make_shared<LabelOperand>(9)
}) { }

BRzInstruction::BRzInstruction(void) : BRInstruction("brz", {
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(3, 0x2),
    std::make_shared<LabelOperand>(9)
}) { }

BRpInstruction::BRpInstruction(void) : BRInstruction("brp", {
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(3, 0x1),
    std::make_shared<LabelOperand>(9)
}) { }

BRnzInstruction::BRnzInstruction(void) : BRInstruction("brnz", {
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(3, 0x6),
    std::make_shared<LabelOperand>(9)
}) { }

BRzpInstruction::BRzpInstruction(void) : BRInstruction("brzp", {
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(3, 0x3),
    std::make_shared<LabelOperand>(9)
}) { }

BRnpInstruction::BRnpInstruction(void) : BRInstruction("brnp", {
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(3, 0x5),
    std::make_shared<LabelOperand>(9)
}) { }

BRnzpInstruction::BRnzpInstruction(void) : BRInstruction("brnzp", {
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(3, 0x7),
    std::make_shared<LabelOperand>(9)
}) { }

NOP0Instruction::NOP0Instruction(void) : BRInstruction("nop", {
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(3, 0x0),
    std::make_shared<FixedOperand>(9, 0x0)
}) { }

NOP1Instruction::NOP1Instruction(void) : BRInstruction("nop", {
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(3, 0x0),
    std::make_shared<LabelOperand>(9)
}) { }

JMPInstruction::JMPInstruction(void) : IInstruction("jmp", {
    std::make_shared<FixedOperand>(4, 0xc),
    std::make_shared<FixedOperand>(3, 0x0),
    std::make_shared<RegOperand>(3),
    std::make_shared<FixedOperand>(6, 0x0)
}) { }

JSRInstruction::JSRInstruction(void) : IInstruction("jsr", {
    std::make_shared<FixedOperand>(4, 0x4),
    std::make_shared<FixedOperand>(1, 0x1),
    std::make_shared<LabelOperand>(11)
}) { }

JSRRInstruction::JSRRInstruction(void) : IInstruction("jsrr", {
    std::make_shared<FixedOperand>(4, 0x4),
    std::make_shared<FixedOperand>(1, 0x0),
    std::make_shared<FixedOperand>(2, 0x0),
    std::make_shared<RegOperand>(3),
    std::make_shared<FixedOperand>(6, 0x0)
}) { }

LDInstruction::LDInstruction(void) : IInstruction("ld", {
    std::make_shared<FixedOperand>(4, 0x2),
    std::make_shared<RegOperand>(3),
    std::make_shared<LabelOperand>(9)
}) { }

LDIInstruction::LDIInstruction(void) : IInstruction("ldi", {
    std::make_shared<FixedOperand>(4, 0xa),
    std::make_shared<RegOperand>(3),
    std::make_shared<LabelOperand>(9)
}) { }

LDRInstruction::LDRInstruction(void) : IInstruction("ldr", {
    std::make_shared<FixedOperand>(4, 0x6),
    std::make_shared<RegOperand>(3),
    std::make_shared<RegOperand>(3),
    std::make_shared<NumOperand>(6, true)
}) { }

LEAInstruction::LEAInstruction(void) : IInstruction("lea", {
    std::make_shared<FixedOperand>(4, 0xe),
    std::make_shared<RegOperand>(3),
    std::make_shared<LabelOperand>(9)
}) { }

NOTInstruction::NOTInstruction(void) : IInstruction("not", {
    std::make_shared<FixedOperand>(4, 0x9),
    std::make_shared<RegOperand>(3),
    std::make_shared<RegOperand>(3),
    std::make_shared<FixedOperand>(6, 0x3f)
}) { }

RETInstruction::RETInstruction(void) : JMPInstruction("ret", {
    std::make_shared<FixedOperand>(4, 0xc),
    std::make_shared<FixedOperand>(3, 0x0),
    std::make_shared<FixedOperand>(3, 0x7),
    std::make_shared<FixedOperand>(6, 0x0)
}) { }

RTIInstruction::RTIInstruction(void) : IInstruction("rti", {
    std::make_shared<FixedOperand>(4, 0x8),
    std::make_shared<FixedOperand>(12, 0x0)
}) { }

STInstruction::STInstruction(void) : IInstruction("st", {
    std::make_shared<FixedOperand>(4, 0x3),
    std::make_shared<RegOperand>(3),
    std::make_shared<LabelOperand>(9)
}) { }

STIInstruction::STIInstruction(void) : IInstruction("sti", {
    std::make_shared<FixedOperand>(4, 0xb),
    std::make_shared<RegOperand>(3),
    std::make_shared<LabelOperand>(9)
}) { }

STRInstruction::STRInstruction(void) : IInstruction("str", {
    std::make_shared<FixedOperand>(4, 0x7),
    std::make_shared<RegOperand>(3),
    std::make_shared<RegOperand>(3),
    std::make_shared<NumOperand>(6, true)
}) { }

TRAPInstruction::TRAPInstruction(void) : IInstruction("trap", {
    std::make_shared<FixedOperand>(4, 0xf),
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<NumOperand>(8, false)
}) { }

GETCInstruction::GETCInstruction(void) : TRAPInstruction("getc", {
    std::make_shared<FixedOperand>(4, 0xf),
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(8, 0x20)
}) { }

OUTInstruction::OUTInstruction(void) : TRAPInstruction("out", {
    std::make_shared<FixedOperand>(4, 0xf),
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(8, 0x21)
}) { }

PUTCInstruction::PUTCInstruction(void) : TRAPInstruction("putc", {
    std::make_shared<FixedOperand>(4, 0xf),
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(8, 0x21)
}) { }

PUTSInstruction::PUTSInstruction(void) : TRAPInstruction("puts", {
    std::make_shared<FixedOperand>(4, 0xf),
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(8, 0x22)
}) { }

INInstruction::INInstruction(void) : TRAPInstruction("in", {
    std::make_shared<FixedOperand>(4, 0xf),
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(8, 0x23)
}) { }

PUTSPInstruction::PUTSPInstruction(void) : TRAPInstruction("putsp", {
    std::make_shared<FixedOperand>(4, 0xf),
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(8, 0x24)
}) { }

HALTInstruction::HALTInstruction(void) : TRAPInstruction("halt", {
    std::make_shared<FixedOperand>(4, 0xf),
    std::make_shared<FixedOperand>(4, 0x0),
    std::make_shared<FixedOperand>(8, 0x25)
}) { }
