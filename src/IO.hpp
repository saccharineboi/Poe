// Poe: OpenGL Renderer
// Copyright (C) 2023 saccharineboi
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

#pragma once

#include <fstream>
#include <string>

namespace Poe::IO
{
    ////////////////////////////////////////
    inline std::string ReadTextFile(const std::string& filePath)
    {
        std::string all;
        if (std::ifstream fp{filePath}) {
            for (std::string current; std::getline(fp, current); all += current + '\n')
                ;
        }
        return all;
    }
}
