// Poe: OpenGL Renderer
// Copyright (C) 2022 saccharineboi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "Demo.hpp"
#include "Constants.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

////////////////////////////////////////
static void printHelp(const char* progName)
{
    std::printf("Usage: %s [OPTION]...\n", progName);
    std::printf("Poe is an OpenGL 4.5 Renderer\n\n");
    std::printf("Options:\n");
    std::printf("\t-h, --help\t\tDisplay this help message\n");
    std::printf("\t-v, --version\t\tDisplay version information\n");
    std::printf("\t-l, --license\t\tDisplay license information\n");
}

////////////////////////////////////////
static void printVersion()
{
    std::printf("Poe OpenGL 4.5 Renderer Version %d.%d\n", Poe::POE_VERSION_MAJOR, Poe::POE_VERSION_MINOR);
}

////////////////////////////////////////
static void printLicense()
{
    static const char* license = "Poe: OpenGL Renderer\n"
    "Copyright (C) 2022 saccharineboi\n\n"
    "This program is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program.  If not, see <https://www.gnu.org/licenses/>.\n";
    std::printf("%s", license);
}

////////////////////////////////////////
int main(int argc, char** argv)
{
    constexpr int strMaxSize = 20;
    for (int i = 1; i < argc; ++i) {
        if (!std::strncmp(argv[i], "-h", strMaxSize) ||
            !std::strncmp(argv[i], "--help", strMaxSize)) {
            printHelp(argv[0]);
            std::exit(EXIT_SUCCESS);
        }
        else if (!std::strncmp(argv[i], "-v", strMaxSize) ||
                 !std::strncmp(argv[i], "--version", strMaxSize)) {
            printVersion();
            std::exit(EXIT_SUCCESS);
        }
        else if (!std::strncmp(argv[i], "-l", strMaxSize) ||
                 !std::strncmp(argv[i], "--license", strMaxSize)) {
            printLicense();
            std::exit(EXIT_SUCCESS);
        }
        else {
            std::fprintf(stderr, "ERROR: %s is an unknown option\n", argv[i]);
            std::exit(EXIT_FAILURE);
        }
    }

    return Poe::Demo::Run(argc, argv);
}
