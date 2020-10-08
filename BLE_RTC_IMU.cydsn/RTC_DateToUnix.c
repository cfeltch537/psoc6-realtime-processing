#include "stdio.h"
#include "time.h"

/* Number of days in month definitions */
#define RTC_DAYS_IN_JANUARY     (31u)
#define RTC_DAYS_IN_FEBRUARY    (28u)
#define RTC_DAYS_IN_MARCH       (31u)
#define RTC_DAYS_IN_APRIL       (30u)
#define RTC_DAYS_IN_MAY         (31u)
#define RTC_DAYS_IN_JUNE        (30u)
#define RTC_DAYS_IN_JULY        (31u)
#define RTC_DAYS_IN_AUGUST      (31u)
#define RTC_DAYS_IN_SEPTEMBER   (30u)
#define RTC_DAYS_IN_OCTOBER     (31u)
#define RTC_DAYS_IN_NOVEMBER    (30u)
#define RTC_DAYS_IN_DECEMBER    (31u)
#define RTC_MONTHS_PER_YEAR     (12uL)
#define RTC_HOURS_PER_DAY       (24uL)
#define RTC_SECONDS_PER_MINUTE  (60uL)
#define RTC_SECONDS_PER_HOUR    (3600uL)
#define RTC_SECONDS_PER_DAY     (24uL * 3600uL)
#define RTC_LEAP_YEAR(year)             ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))
#define RTC_SECONDS_PER_LEAP_YEAR    (366uL * 24uL * 3600uL)
#define RTC_SECONDS_PER_NONLEAP_YEAR (365uL * 24uL * 3600uL)


extern const uint16_t RTC_daysBeforeMonthTbl[RTC_MONTHS_PER_YEAR];


const uint16_t RTC_daysBeforeMonthTbl[12] = {
                                                 (0u), /* JANUARY */
                                                 (0u + 31u), /* FEBRUARY */
                                                 (0u + 31u + 28u), /* MARCH */
                                                 (0u + 31u + 28u + 31u), /* APRIL */
                                                 (0u + 31u + 28u + 31u + 30u), /* MAY */
                                                 (0u + 31u + 28u + 31u + 30u + 31u), /* JUNE */
                                                 (0u + 31u + 28u + 31u + 30u + 31u + 30u), /* JULY */
                                                 (0u + 31u + 28u + 31u + 30u + 31u + 30u + 31u), /* AUGUST */
                                                 (0u + 31u + 28u + 31u + 30u + 31u + 30u + 31u + 31u), /* SEPTEMBER */
                                                 (0u + 31u + 28u + 31u + 30u + 31u + 30u + 31u + 31u + 30u), /* OCTOBER */
                                                 (0u + 31u + 28u + 31u + 30u + 31u + 30u + 31u + 31u + 30u + 31u), /* NOVEMBER */
                                                 (0u + 31u + 28u + 31u + 30u + 31u + 30u + 31u + 31u + 30u + 31u + 30u)}; /* DECEMBER */


int unixtm;


int RTC_DateTimeToUnix(struct tm *time)
{
    uint32_t i;
    uint32_t tmpYear;
    int unixTime;

    unixTime = 0u;
    //Psoc 6 start the count from 1900's year
    tmpYear = time->tm_year + 1900;

    /* Calculate seconds from epoch start up to (but not including) current year */

    for(i = 1970; i < tmpYear; i++)
    {
        if((0u != RTC_LEAP_YEAR(i)))
        {
            unixTime += RTC_SECONDS_PER_LEAP_YEAR;
        }
        else
        {
            unixTime += RTC_SECONDS_PER_NONLEAP_YEAR;
        }
    }
    /* Calculates how many seconds have elapsed today from day of year */

    //unixTime += ((int)time->tm_yday - 1u) * RTC_SECONDS_PER_DAY;
    //Non so perchè viene sottratto un giorno...togliendolo ci si trova esattamente un giorno in meno
    unixTime += ((int)time->tm_yday) * RTC_SECONDS_PER_DAY;

    /* Calculates how many seconds have elapsed today up to the current hour */

    unixTime += (int)time->tm_hour * RTC_SECONDS_PER_HOUR;

    /* Calculates how many seconds have elapsed today up to the current minute */
    unixTime += ((int) time->tm_min) * RTC_SECONDS_PER_MINUTE;

    /* Add remaining seconds of current minute */
    unixTime += time->tm_sec;

    return(unixTime);
}

