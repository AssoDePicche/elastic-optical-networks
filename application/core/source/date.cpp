#include "date.h"

#include <chrono>
#include <ctime>

DateTime DateTime::now(void) {
  const auto time = std::time(nullptr);

  const auto localtime = std::localtime(&time);

  DateTime datetime;

  datetime.day = localtime->tm_mday;

  datetime.month = localtime->tm_mon + 1;

  datetime.year = localtime->tm_year + 1900;

  datetime.hour = localtime->tm_hour;

  datetime.minute = localtime->tm_min;

  return datetime;
}
