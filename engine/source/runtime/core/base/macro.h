#pragma once

#include <iostream>
#include <stdexcept>

#define LOG_INFO(...) std::cout << __VA_ARGS__

// #define LOG_ERROR(...) throw std::runtime_error(__VA_ARGS__) //...表示可变参数，__VA_ARGS__就是将...的值复制到这里
#define LOG_ERROR(...) std::cerr << __VA_ARGS__

#define LOG_FATAL(...) std::cerr << __VA_ARGS__