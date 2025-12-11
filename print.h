
#pragma once

#include <iostream>

template <typename... Args>
void println(Args &&...args)
{
    (std::cout << ... << std::forward<Args>(args));
    std::cout << std::endl;
}