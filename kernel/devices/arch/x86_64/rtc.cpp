#include <kernel_common.h>
#include <rtc.h>

vios_date_time date_time;

void rtc_initalize( void ) {
	rtc_update_time();
	debugf( "Current date and time: %s\n", rtc_get_datetime_string() );
}

uint8_t rtc_get_update_status( void ) {
	outportb( RTC_PORT_ADDR, 0x0A );
	return inportb( RTC_PORT_DATA ) & 0x80;
}

vios_date_time *rtc_update_time( void ) {
	// hold until there isn't an update running
	// This seems to take a fair bit... doing without it for now
	// while( !rtc_get_update_status() ) { ; }

	RTC_GET_DATA( RTC_REG_YEAR, date_time.year );
	RTC_GET_DATA( RTC_REG_MONTH, date_time.month );
	RTC_GET_DATA( RTC_REG_DAY, date_time.day );
	RTC_GET_DATA( RTC_REG_HOUR, date_time.hour );
	RTC_GET_DATA( RTC_REG_MIN, date_time.min );
	RTC_GET_DATA( RTC_REG_SEC, date_time.sec );

	uint8_t regb = 0;
	RTC_GET_DATA( 0x0B, regb );

	if( !(regb & 0x04) ) {
		date_time.sec = (date_time.sec & 0x0F) + ((date_time.sec / 16) * 10);
		date_time.min = (date_time.min & 0x0F) + ((date_time.min / 16) * 10);
		date_time.hour = ( (date_time.hour & 0x0F) + (((date_time.hour & 0x70) / 16) * 10) ) | (date_time.hour & 0x80);
		date_time.day = (date_time.day & 0x0F) + ((date_time.day / 16) * 10);
		date_time.month = (date_time.month & 0x0F) + ((date_time.month / 16) * 10);
		date_time.year = (date_time.year & 0x0F) + ((date_time.year / 16) * 10);
	}

	if( !(regb & 0x02) && (date_time.hour & 0x80) ) {
		date_time.hour = ((date_time.hour & 0x7F) + 12) % 24;
	}

	snprintf( date_time.basic_datetime_string, 25, "%d/%d/%d %d:%02d:%02d", date_time.month, date_time.day, date_time.year, date_time.hour, date_time.min, date_time.sec );

	return &date_time;
}

char *rtc_get_datetime_string( void ) {
	rtc_update_time();
	
	return date_time.basic_datetime_string;
}