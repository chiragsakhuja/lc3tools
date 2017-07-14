#ifndef CONSOLE_INPUTTER_H
#define CONSOLE_INPUTTER_H

namespace utils {
    class ConsoleInputter : public IInputter
    {
    public:
        ConsoleInputter(void) = default;
        ~ConsoleInputter(void) = default;

        virtual void beginInput(void) override
        {
            struct termios ttystate;

            tcgetattr(STDIN_FILENO, &ttystate);
            ttystate.c_lflag &= ~ICANON;
            ttystate.c_lflag &= ~ECHO;
            ttystate.c_cc[VMIN] = 1;

            tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
        }

        virtual bool getChar(char & c) override
        {
            if(kbhit() != 0) {
                c = fgetc(stdin);
                return true;
            }

            return false;
        }

        virtual void endInput(void) override
        {
            struct termios ttystate;
            tcgetattr(STDIN_FILENO, &ttystate);
            ttystate.c_lflag |= ICANON;
            ttystate.c_lflag |= ECHO;
            tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
        }

    private:
        struct termios old_tio;

        int kbhit(void)
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

    };
};

#endif
