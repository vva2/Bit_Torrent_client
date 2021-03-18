#include "../include/log.h"

LOG::Log::Log() {
    level = INFO;
    toStdout = true;
    fp = stdout;
}

LOG::Log::~Log() {
    fclose(fp);
}

void LOG::Log::setLevel(Level level) {
    this->level = level;
}

void LOG::Log::setDebugFile(const char *debugFilePath) {
    fp = fopen(debugFilePath, "w");
}

void LOG::Log::info(const char *msg, int n) {
    if(level > INFO) return;
    
    if(n == -1) n = strlen(msg);
    printf("[%sINFO%s]::", DARK_GREEN, RESET);
    fwrite(msg, 1, n, fp);
}

void LOG::Log::warning(const char *msg, int n) {
    if(level > WARNING) return;

    if(n == -1) n = strlen(msg);
    printf("[%sWARNING%s]::", DARK_BLUE, RESET);
    fwrite(msg, 1, n, fp);
}

void LOG::Log::error(const char *msg, int n) {
    if(level > ERROR) return;

    if(n == -1) n = strlen(msg);
    printf("[%sERROR%s]::", DARK_RED, RESET);
    fwrite(msg, 1, n, fp);
}