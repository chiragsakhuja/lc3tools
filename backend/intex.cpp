#include "intex.h"

uint8_t lc3::core::getExceptionVector(ExceptionType type)
{
    switch(type) {
        case ExceptionType::PRIVILEGE_MODE: return 0x00;
        case ExceptionType::ILLEGAL_OPCODE: return 0x01;
        case ExceptionType::ACV: return 0x02;
        default: return 0x7F;
    }
}

uint8_t lc3::core::getInterruptVector(InterruptType type)
{
    switch(type) {
        case InterruptType::KEYBOARD: return 0x80;
        default: return 0x00;
    }
}

uint8_t lc3::core::getInterruptPriority(InterruptType type)
{
    switch(type) {
        case InterruptType::KEYBOARD: return 0x04;
        default: return 0x00;
    }
}

std::string lc3::core::interruptTypeToString(InterruptType type)
{
    switch(type) {
        case InterruptType::KEYBOARD: return "keyboard";
        default: return "invalid";
    }
}
