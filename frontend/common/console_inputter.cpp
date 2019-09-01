/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <iostream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

#include "console_inputter.h"

void lc3::ConsoleInputter::beginInput(void)
{
#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32))
    struct termios ttystate;

    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag &= ~ICANON;
    ttystate.c_lflag &= ~ECHO;
    ttystate.c_cc[VMIN] = 1;

    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
#endif
}

bool lc3::ConsoleInputter::getChar(char & c)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    if(_kbhit() != 0) {
        c = _getch();
        return true;
    }
#else
    if(kbhit() != 0) {
        c = fgetc(stdin);
        return true;
    }
#endif

    return false;
}

void lc3::ConsoleInputter::endInput(void)
{
#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32))
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag |= ICANON;
    ttystate.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
#endif
}

#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32))
int lc3::ConsoleInputter::kbhit(void)
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}
#endif
