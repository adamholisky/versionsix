#ifndef VIOS_RTC_INCLUDED
#define VIOS_RTC_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define RTC_PORT_ADDR 0x70
#define RTC_PORT_DATA 0x71

#define RTC_REG_YEAR 0x09
#define RTC_REG_MONTH 0x08
#define RTC_REG_DAY 0x07
#define RTC_REG_HOUR 0x04
#define RTC_REG_MIN 0x02
#define RTC_REG_SEC 0x00

typedef struct {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;

    char basic_datetime_string[25];
} vios_date_time;

void rtc_initalize( void );
uint8_t rtc_get_update_status( void );
vios_date_time *rtc_update_time( void );
char *rtc_get_datetime_string( void );

#define RTC_GET_DATA( reg, var ) \
    outportb( RTC_PORT_ADDR, reg ); \
    var = inportb( RTC_PORT_DATA )

#ifdef __cplusplus
}
#endif
#endif