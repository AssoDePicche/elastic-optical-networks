#pragma once

#ifdef ALLOWLOGS

#include <cstring>
#include <iostream>

#define __FILENAME__                                                           \
  (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG(LEVEL, STR)                                                        \
  do {                                                                         \
    std::clog << '[' << LEVEL << "] " << __FILENAME__ << '@' << __func__       \
              << ": " << STR << std::endl;                                     \
  } while (0);

#define INFO(STR)                                                              \
  do {                                                                         \
    LOG("INFO", STR);                                                          \
  } while (0);

#define WARNING(STR)                                                           \
  do {                                                                         \
    LOG("WARN", STR);                                                          \
  } while (0);
#else

#define INFO(STR) (void)(STR)

#endif
