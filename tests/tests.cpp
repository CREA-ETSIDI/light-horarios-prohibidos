#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// Code to be tested
#define tmDow2secs(t) (t.hour * 3600UL + t.mins * 60UL + t.secs)
// Time.h used definitions
typedef enum {
    dowInvalid, dowSunday, dowMonday, dowTuesday, dowWednesday, dowThursday, dowFriday, dowSaturday
} timeDayOfWeek_t;

typedef struct { // Custom time struct, only defined within a week
  uint8_t wDay;
  uint8_t hour;
  uint8_t mins;
  uint8_t secs;
} timeAndDOW_t;

bool isBetween_timeIgnoreDOW(timeAndDOW_t start, timeAndDOW_t end_tm, timeAndDOW_t compare) {
  long startSecs = tmDow2secs(start);
  long cmpSecs   = tmDow2secs(compare);
  long endSecs   = tmDow2secs(end_tm);
  return (startSecs <= cmpSecs && cmpSecs <= endSecs) ? true : false;
}

// Tests
TEST_CASE("isBetween_timeIgnoreDOW()") {
	timeAndDOW_t td;

	SUBCASE("Should be true") {
		td = {dowInvalid, 21, 50, 0};
		CHECK(isBetween_timeIgnoreDOW(
		  {dowInvalid, 21, 30, 0 },
		  {dowInvalid, 23, 59, 59},
		  td) == true);
	}
	SUBCASE("Should be false") {
		td = {dowInvalid, 21, 20, 0};
		CHECK(isBetween_timeIgnoreDOW(
		  {dowInvalid, 21, 30, 0 },
		  {dowInvalid, 23, 59, 59},
		  td) == false);
	}

}

