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

#define SUPPRESS_WARNINGS() \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wsign-conversion\"") \
    _Pragma("GCC diagnostic ignored \"-Wold-style-cast\"") \
    _Pragma("GCC diagnostic ignored \"-Wfloat-equal\"") \
    _Pragma("GCC diagnostic ignored \"-Wcast-qual\"") \
    _Pragma("GCC diagnostic ignored \"-Wduplicated-branches\"") \
    _Pragma("GCC diagnostic ignored \"-Wconversion\"")

#define ENABLE_WARNINGS() \
    _Pragma("GCC diagnostic pop")
