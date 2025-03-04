#ifndef GPS_H
#define GPS_H

#include <stdint.h>

#define GSA_COMA_COUNT			14
#define CHECKSUM_START_POINT	4
#define NMEA_START_POINT		1
#define NMEA_END_POINT			5

/*
 * GPS SETTINGS
 */
#define	GPS_UART	&huart1
#define NMEA_SENTENCES	6
#define MIN_BUFFER_SIZE	16
#define RX_BUFFER_SIZE  128

/*
 * PRINTING SETTINGS
 */
//#define PRINT_DEBUGGER
//#define PRINT_BUFFER
//#define PRINT_PARSED_DATA
//#define PRINT_COMPLETE_DATA

/*
 * PARSING SETTINGS
 */
#define PARSE_RMC
#define PARSE_VTG
#define PARSE_GGA
#define PARSE_GSA
#define PARSE_GSV
#define PARSE_GLL

typedef struct {
    float latitude;
    char lat_direction;
    float longitude;
    char lon_direction;
    float altitude;
    float geoid_sep;
    float time;
    int date;
    int  satellites_visible;
    int  satellites_in_use;
    int fix_mode;
    int gps_quality;
    float pdop;				/* precision | <1 - excellent | 1-2 - good | 2-5 - moderate | >5 - bad | */
    float hdop;				/* precision | <1 - excellent | 1-2 - good | 2-5 - moderate | >5 - bad | */
    float vdop;				/* precision | <1 - excellent | 1-2 - good | 2-5 - moderate | >5 - bad | */
    float ground_speed_knots;
    float ground_speed_kph;
} NMEAData;

/*
 * CORE FUNCTIONS
 */
void gps_init(void);
int gps_is_data_ready(void);
int gps_parse_data(void);
/*
 * ADITIONAL FUNCTIONS
 */
void gps_print_rx_buffer(void);
char* gps_complete_location_string(void);
float gps_latitude(void);
char gps_lat_direction(void);
float gps_longitude(void);
char gps_lon_direction(void);
int gps_altitude(void);
char* gps_complete_date_string(void);
int gps_day(void);
int gps_month(void);
int gps_year(void);
int gps_year_long_format(void);
char* gps_complete_time_string(int offset);
int gps_hour(int offset);
int gps_minute(void);
int gps_second(void);
int gps_satellites_visible(void);
int gps_satellites_in_use(void);
char* gps_fix_mode_string(void);
char* gps_quality_string(void);
float gps_pdop(void);
float gps_hdop(void);
float gps_vdop(void);
float gps_speed_knots(void);
float gps_speed_kph(void);

#endif /* GPS_H */
