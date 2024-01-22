// Poe: OpenGL 4.5 Renderer
// Copyright (C) 2024 Omar Huseynov
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

#include "UI.hpp"

namespace Poe
{
    ////////////////////////////////////////
    bool DebugUI::mEnableWireframe{false};
    bool DebugUI::mEnableSkybox{true};
    bool DebugUI::mEnableGrid{true};
    bool DebugUI::mEnableVsync{true};
    std::vector<std::string> DebugUI::mCoutLogs{};
    std::vector<std::string> DebugUI::mCerrLogs{};
}
