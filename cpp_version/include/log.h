#pragma once
#include <stdio.h>
#include <string.h>

    

namespace LOG {
    // source : https://bluesock.org/~willg/dev/ansi.html#ansicodes
    const char ESC[] =  "\e[";

    // text colors
    const char RESET[] =  "\e[m";
    const char BLACK[] =  "\e[0;30m";
    const char BLUE[] =  "\e[0;34m";
    const char GREEN[] =  "\e[0;32m";
    const char CYAN[] =  "\e[0;36m";
    const char RED[] =  "\e[0;31m";
    const char PURPLE[] =  "\e[0;35m";
    const char BROWN[] =  "\e[0;33m";
    const char GRAY[] =  "\e[0;37m";
    const char DARK_BLUE[] =  "\e[1;34m";
    const char DARK_GRAY[] =  "\e[1;30m";
    const char DARK_RED[] =  "\e[1;31m";
    const char DARK_GREEN[] =  "\e[1;32m";
    const char LIGHT_BLUE[] =  "\e[1;34m";
    const char LIGHT_CYAN[] =  "\e[1;36m";
    const char LIGHT_PURPLE[] =  "\e[1;35m";
    const char YELLOW[] =  "\e[1;33m";
    const char WHITE[] =  "\e[1;37m";

    // text attributes
    const char NORMAL[] =  "\e[0m";
    const char BOLD[] =  "\e[1m";
    const char UNDERLINE[] =  "\e[4m";
    const char BLINK_ON[] =  "\e[5m";
    const char REVERSE_VIDEO_ON[] =  "\e[7m";
    const char NON_DISPLAYED[] =  "\e[8m";  // invisible

    // foreground colors
    const char F_BLACK[] =  "\e[30m";
    const char F_RED[] =  "\e[31m";
    const char F_GREEN[] =  "\e[32m";
    const char F_YELLOW[] =  "\e[33m";
    const char F_BLUE[] =  "\e[34m";
    const char F_MAGENTA[] =  "\e[35m";
    const char F_CYAN[] =  "\e[36m";
    const char F_WHITE[] =  "\e[37m";

    // background colors
    const char B_BLACK[] =  "\e[40m";
    const char B_RED[] =  "\e[41m";
    const char B_GREEN[] =  "\e[42m";
    const char B_YELLOW[] =  "\e[43m";
    const char B_BLUE[] =  "\e[44m";
    const char B_MAGENTA[] =  "\e[45m";
    const char B_CYAN[] =  "\e[46m";
    const char B_WHITE[] =  "\e[47m";

    class Log {
        public:
            enum Level {
                INFO,
                WARNING,
                ERROR
            };
        
        private:
            Level level;
            bool toStdout;  // true initially
            FILE* fp;

        public:
            Log();
            ~Log();
            void setLevel(Level level);
            void setDebugFile(const char *debugFilePath);  // if not called prints directly to stdout
            
            // prints c-string buffers
            void info(const char *msg, int n=-1);
            void warning(const char *msg, int n=-1);
            void error(const char *m, int n=-1);
    };
};




