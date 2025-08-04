#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <unistd.h>
#include <string>

class FileSystem {
public:
    static inline std::string getPath(const std::string& l_path) {
        return FileSystem::getWorkingDirectory() + l_path;
    }

    static inline std::string getWorkingDirectory() {
        char cwd[1024];

        if (getcwd(cwd, 1024) != nullptr) {
            return std::string(cwd) + std::string("/");
        }
        return "";
    }

    static void chDir() {
        int i = chdir("..");
        if (i < 0) {
            printf("couldn't change the directory\n");
        }
    }
};

#endif