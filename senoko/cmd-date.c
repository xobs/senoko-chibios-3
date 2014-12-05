/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "rtc.h"
#include "chprintf.h"

#include "bionic.h"
#include "senoko.h"

#if HAL_USE_RTC
const char *dow[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char *mon[] = { "Non", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", };
static const char *ampm[] = { "am", "pm" };

static void print_time(BaseSequentialStream *chp, int usage) {
  RTCDateTime ts;
  uint32_t hour, minute, second, millisecond;
  const char *ap = ampm[0];

  chThdSleepMilliseconds(100);
  rtcGetTime(&RTCD1, &ts);

  millisecond = ts.millisecond;
  
  second = millisecond / 1000;
  millisecond -= second * 1000;

  minute = second / 60;
  second -= minute * 60;

  hour = minute / 60;
  minute -= hour * 60;

  if (hour == 0)
    hour += 12;
  else if (hour >= 12) {
    ap = ampm[1];
    if (hour > 12)
      hour -= 12;
  }

  chprintf(chp, "%s %s %d  %d:%02d:%02d:%04d %s  UTC  %d\r\n",
      dow[ts.dayofweek - 1], mon[ts.month], ts.day,
      hour, minute, second, millisecond, ap,
      ts.year + 1980);

  if (usage) {
    chprintf(chp, "Usage:\r\n");
    chprintf(chp, "    date day [d]    Set day-of-month to [d]\r\n");
    chprintf(chp, "    date month [m]  Set month to [m]\r\n");
    chprintf(chp, "    date ms [m]     Set milliseconds-since-midnight to [m]\r\n");
    chprintf(chp, "    date year [y]   Set the year to [y]\r\n");
  }
}
#endif /* HAL_USE_RTC */

void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]) {
#if HAL_USE_RTC
  int usage = 1;

  if (argc >= 1 && !strcasecmp(argv[0], "day")) {
    RTCDateTime ts;
    rtcGetTime(&RTCD1, &ts);
    int day;

    day = strtoul(argv[1], NULL, 0);
    chprintf(chp, "Setting day-of-month: %d\r\n", day);
    ts.day = day;
    rtcSetTime(&RTCD1, &ts);
    usage = 0;
  }

  else if (argc >= 1 && !strcasecmp(argv[0], "month")) {
    RTCDateTime ts;
    rtcGetTime(&RTCD1, &ts);
    int month;

    month = strtoul(argv[1], NULL, 0);
    if (!month) {
      size_t i;
      for (i = 0; (!month) && (i < ARRAY_SIZE(mon)); i++)
        if (!strcasecmp(argv[1], mon[i]))
          month = i;
    }

    if (month) {
      chprintf(chp, "Setting month: %s\r\n", mon[month]);
      ts.month = month;
      rtcSetTime(&RTCD1, &ts);
      usage = 0;
    }
    else {
      chprintf(chp, "Invalid month: %s\r\n", argv[1]);
      return;
    }
  }

  else if (argc >= 1 && !strcasecmp(argv[0], "ms")) {
    RTCDateTime ts;
    rtcGetTime(&RTCD1, &ts);
    int ms;

    ms = strtoul(argv[1], NULL, 0);

    chprintf(chp, "Setting milliseconds-since-midnight: %d\r\n", ms);
    ts.millisecond = ms;
    rtcSetTime(&RTCD1, &ts);
    usage = 0;
  }

  else if (argc >= 1 && !strcasecmp(argv[0], "year")) {
    RTCDateTime ts;
    rtcGetTime(&RTCD1, &ts);
    int year;

    year = strtoul(argv[1], NULL, 0);

    chprintf(chp, "Party like it's %d\r\n", year);
    ts.year = year - 1980;
    rtcSetTime(&RTCD1, &ts);
    usage = 0;
  }

  print_time(chp, usage);
#else
  (void)argc;
  (void)argv;
  chprintf(chp, "RTC not enabled\r\n");
#endif /* HAL_USE_RTC */
  return;
}
