#pragma once

struct DateTime final {
  unsigned day;
  unsigned month;
  unsigned year;
  unsigned hour;
  unsigned minute;

  [[nodiscard]] static DateTime now(void);
};
