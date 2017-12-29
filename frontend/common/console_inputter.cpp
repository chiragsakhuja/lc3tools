#include <unistd.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    #include <conio.h>
#else
    #include <termios.h>
#endif

#include "core.h"
#include "console_inputter.h"

void utils::ConsoleInputter::beginInput(void)
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

bool utils::ConsoleInputter::getChar(char & c)
{
    if(kbhit() != 0) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
        c = getch();
#else
        c = fgetc(stdin);
#endif
        return true;
    }

    return false;
}

void utils::ConsoleInputter::endInput(void)
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
int utils::ConsoleInputter::kbhit(void)
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
