/*
 * Copyright (C) 2025  F_Error11
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ET_
#define ET_

#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace C {  // Color
    std::string 红 = "\033[31m";
    std::string 绿 = "\033[32m";
    std::string 黄 = "\033[33m";
    std::string 蓝 = "\033[34m";
    std::string 紫 = "\033[35m";
    std::string 青 = "\033[36m";
    std::string 无 = "\033[0m";
    std::string 灰 = "\033[90m";
}

void 分割线(const std::string& c = C::灰) {
    struct winsize w;
    int I = (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) ? w.ws_col : 24;
    for (int i = 0; i <= I; i++) std::cout << c << "-";
    std::cout << C::无 << std::endl;
}

#endif  // ET_
