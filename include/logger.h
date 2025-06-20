#pragma once

#ifdef ALLOWLOGS

#include <cstring>
#include <format>
#include <iostream>

#define __FILENAME__ \
  (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG(LEVEL, STR)                                               \
  do {                                                                \
    std::clog << std::format("[{}] {}@{}: {}\n", LEVEL, __FILENAME__, \
                             __func__, STR);                          \
  } while (0)

#define ERROR(STR)    \
  do {                \
    LOG("ERROR", STR) \
  } while (0);

#define INFO(STR)     \
  do {                \
    LOG("INFO", STR); \
  } while (0);

#define WARNING(STR)  \
  do {                \
    LOG("WARN", STR); \
  } while (0);
#else

#define ERROR(STR) (void)(STR)

#define INFO(STR) (void)(STR)

#define WARNING(STR) (void)(STR)

#endif
