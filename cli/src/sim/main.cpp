#include <iostream>
#include <string>
#include <unistd.h>
#include <termios.h>
#include <thread>

#include "core.h"
#include "console_printer.h"
#include "console_inputter.h"

#define NB_ENABLE 10
#define NB_DISABLE 11


void nonblock(int state)
{
    struct termios ttystate;

    tcgetattr(STDIN_FILENO, &ttystate);

    if(state == NB_ENABLE) {
        ttystate.c_lflag &= ~ICANON;
        ttystate.c_cc[VMIN] = 1;
    } else if(state == NB_DISABLE) {
        ttystate.c_lflag |= ICANON;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int main(int argc, char * argv[])
{
    utils::IPrinter * printer = new utils::ConsolePrinter();
    utils::IInputter * inputter = new utils::ConsoleInputter();
    core::lc3 interface(*printer, *inputter);

/*
 *    inputter->beginInput();
 *    char c = 0;
 *    while(c != 'q') {
 *        char value = 0;
 *        if(inputter->getChar(value)) {
 *            c = value;
 *        }
 *        std::this_thread::sleep_for(std::chrono::milliseconds(10));
 *    }
 *
 */
    /*
     *char c;
     *int i = 0;
     *nonblock(NB_ENABLE);
     *while(!i) {
     *    usleep(1);
     *    i = kbhit();
     *    if(i != 0) {
     *        c = fgetc(stdin);
     *        if(c == 'q') { i = 1; }
     *        else { i = 0; }
     *    }
     *}
     *printf("\n you hit %c. \n", c);
     *nonblock(NB_DISABLE);
     */

    interface.initializeSimulator();

    for(int i = 1; i < argc; i += 1) {
        try {
            interface.loadSimulatorWithFile(std::string(argv[i]));
        } catch (utils::exception const & e) {
            printer->print(e.what());
            printer->newline();
        }
    }

    try {
        interface.simulate();
    } catch(utils::exception const & e) {
        printer->print(e.what());
        printer->newline();
    }

    delete printer;
    delete inputter;

    return 0;
}
