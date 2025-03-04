#include "GPS.h"
#include "main.h"
#include <usart.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t rx_buffer[RX_BUFFER_SIZE];
char nmea_buffer[RX_BUFFER_SIZE];
volatile uint8_t received_byte;
volatile uint16_t rx_buffer_index = 0;
volatile uint16_t buffer_size = 0;
volatile uint8_t gps_data_ready_flag = 0;

const char *nmea_id[] = {"RMC", "VTG", "GGA", "GSA", "GSV", "GLL"};

NMEAData gps;

#ifdef	PARSE_RMC
static void parse_rmc(void);
#endif
#ifdef	PARSE_VTG
static void parse_vtg(void);
#endif
#ifdef	PARSE_GGA
static void parse_gga(void);
#endif
#ifdef	PARSE_GSA
static void parse_gsa(void);
#endif
#ifdef	PARSE_GSV
static void parse_gsv(void);
#endif
#ifdef	PARSE_GLL
static void parse_gll(void);
#endif

void gps_init(void){
	HAL_UART_Receive_IT(GPS_UART, (uint8_t *)&received_byte, 1);
}

static uint8_t gps_calculate_checksum(void){
	uint8_t checksum = 0;
	for(int i = NMEA_START_POINT; i < buffer_size - NMEA_END_POINT; i++){
		checksum ^= rx_buffer[i];
	}
	return checksum;
}

static uint8_t gps_parse_checksum(void){
	return (uint8_t)strtol((char*)&rx_buffer[buffer_size - CHECKSUM_START_POINT], NULL, 16);
}

static uint8_t gps_nmea_id(void){
	for(int i = 0; i < NMEA_SENTENCES; i++){
		if(strstr((char*)rx_buffer, nmea_id[i]))	return i;
	}
	return 0xFF;
}

static uint8_t gps_validate_data(void){
	return (gps_calculate_checksum() == gps_parse_checksum());
}

int gps_is_data_ready(void){
	return gps_data_ready_flag;
}

int gps_parse_data(void){
	if(gps_is_data_ready()){
		switch(gps_nmea_id()){
		case 0:
#ifdef	PARSE_RMC
			parse_rmc();	/* populates following variables with data: | time | latitude | lat_direction | longitude | lon_direction | ground_speed_knots | date | */
#endif
			break;
		case 1:
#ifdef	PARSE_VTG
			parse_vtg();	/* populates following variables with data: | ground_speed_knots | ground_speed_kph | */
#endif
			break;
		case 2:
#ifdef	PARSE_GGA
			parse_gga();	/* populates following variables with data: | time | latitude | lat_direction | longitude | lon_direction | GPS_quality | satellites_in_use | HDOP | altitude | */
#endif
			break;
		case 3:
#ifdef	PARSE_GSA
			parse_gsa();	/* populates following variables with data: | fix_mode | PDOP | HDOP | VDOP | */
#endif
			break;
		case 4:
#ifdef	PARSE_GSV
			parse_gsv();	/* populates following variables with data: | satellites_visible | */
#endif
			break;
		case 5:
#ifdef	PARSE_GLL
			parse_gll();	/* populates following variables with data: | latitude | lat_direction | longitude | lon_direction | time | */
#endif
			break;
		case 0xFF:
			//error
			break;
		}
	}

//		printf("calculated checksum: 0x%.2X\n", gps_calculate_checksum());
//		printf("parsed checksum: 0x%.2X\n", gps_parse_checksum());
//		printf("NMEA id: %d\n", gps_nmea_id());
//		printf("%.*s\n", buffer_size, rx_buffer);


#ifdef PRINT_BUFFER
	gps_print_rx_buffer();
#endif


#ifdef PRINT_PARSED_DATA
	printf("LOC:  %s\n", gps_complete_location_string());
	printf("TIME: %s\n", gps_complete_time_string(0));
	printf("DATE: %s\n", gps_complete_date_string());
#endif
#ifdef PRINT_COMPLETE_DATA
	printf("Lat: %f %c\n", gps.latitude, gps.lat_direction);
	printf("Lon: %f %c\n", gps.longitude, gps.lon_direction);
	printf("Alt: %f\n", gps.altitude);
	printf("Geo sep: %f\n", gps.geoid_sep);
	printf("Time: %f\n", gps.time);
	printf("Date: %d\n", gps.date);
	printf("SV: %d\n", gps.satellites_visible);
	printf("SU: %d\n", gps.satellites_in_use);
	printf("FIX: %d\n", gps.fix_mode);
	printf("Quality: %d\n", gps.gps_quality);
	printf("PDOP: %f\n", gps.pdop);
	printf("HDOP: %f\n", gps.hdop);
	printf("VDOP: %f\n", gps.vdop);
	printf("kts: %f\n", gps.ground_speed_knots);
	printf("kph: %f\n", gps.ground_speed_kph);
#endif
	gps_data_ready_flag = 0;
	return 1;
}

void gps_print_rx_buffer(){
	if(gps_is_data_ready()) {
		printf("\nRX buffer size: %d\n", buffer_size - 1);
		printf("%.*s\n\n", buffer_size - 1, rx_buffer);
	} else {
		printf("----data_not_ready----\n\n");
	}
}

#ifdef	PARSE_RMC
void parse_rmc(void) {
	char *token;
	strncpy(nmea_buffer, (char*)rx_buffer, buffer_size);
#ifdef PRINT_DEBUGGER
	printf("RMC: %.*s length(%d)\n", buffer_size-2, nmea_buffer, buffer_size);
#endif
	token = strtok(nmea_buffer, ",");
	token = strtok(NULL, ",");
	if (token) gps.time = (float) atof(token);
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	if (token) gps.latitude = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.lat_direction = token[0];
	token = strtok(NULL, ",");
	if (token) gps.longitude = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.lon_direction = token[0];
	token = strtok(NULL, ",,");
	if (token) gps.ground_speed_knots = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.date = atoi(token);
#ifdef PRINT_DEBUGGER
	printf("Time: %f\n", gps.time);
	printf("Latitude: %f\n", gps.latitude);
	printf("Lat_dir: %c\n", gps.lat_direction);
	printf("longitude: %f\n", gps.longitude);
	printf("Long_dir: %c\n", gps.lon_direction);
	printf("Speed knots: %f\n", gps.ground_speed_knots);
	printf("Date: %d\n", gps.date);
	printf("\n");
#endif
}
#endif

#ifdef	PARSE_VTG
void parse_vtg(void) {
	char *token;
	strncpy(nmea_buffer, (char*)rx_buffer, buffer_size);
#ifdef PRINT_DEBUGGER
	printf("VTG: %.*s length(%d)\n", buffer_size-2, nmea_buffer, buffer_size);
#endif
	token = strtok(nmea_buffer, ",");
	token = strtok(NULL, ",,");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	if (token) gps.ground_speed_knots = (float) atof(token);
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	if (token) gps.ground_speed_kph = (float) atof(token);
#ifdef PRINT_DEBUGGER
	printf("Speed knots: %f\n", gps.ground_speed_knots);
	printf("Speed km/h: %f\n", gps.ground_speed_kph);
	printf("\n");
#endif
}
#endif

#ifdef	PARSE_GGA
void parse_gga(void) {
	char *token;
	strncpy(nmea_buffer, (char*)rx_buffer, buffer_size);
#ifdef PRINT_DEBUGGER
	printf("GAA: %.*s length(%d)\n", buffer_size-2, nmea_buffer, buffer_size);
#endif
	token = strtok(nmea_buffer, ",");
	token = strtok(NULL, ",");
	if (token) gps.time = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.latitude = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.lat_direction = token[0];
	token = strtok(NULL, ",");
	if (token) gps.longitude = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.lon_direction = token[0];
	token = strtok(NULL, ",");
	if (token) gps.gps_quality = atoi(token);
	token = strtok(NULL, ",");
	if (token) gps.satellites_in_use = atoi(token);
	token = strtok(NULL, ",");
	if (token) gps.hdop = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.altitude = (float) atof(token);
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	if (token) gps.geoid_sep = (float) atof(token);
#ifdef PRINT_DEBUGGER
	printf("Time: %f\n", gps.time);
	printf("Latitude: %f\n", gps.latitude);
	printf("Lat_dir: %c\n", gps.lat_direction);
	printf("longitude: %f\n", gps.longitude);
	printf("Long_dir: %c\n", gps.lon_direction);
	printf("GPS quality: %d\n", gps.gps_quality);
	printf("satelites_in_use: %d\n", gps.satellites_in_use);
	printf("Precision (HDOP): %f\n", gps.hdop);
	printf("Altitude: %f\n", gps.altitude);
	printf("Geoid separation: %f\n", gps.geoid_sep);
	printf("\n");
#endif
}
#endif

#ifdef	PARSE_GSA
void parse_gsa(void) {
	char *token;
	int coma_counter = 0;
	int temp_index = 0;
	strncpy(nmea_buffer, (char*)rx_buffer, buffer_size);
	for(int i = 0; i < RX_BUFFER_SIZE; i++){
		if(nmea_buffer[temp_index++] == ','){
			coma_counter++;
			if(coma_counter == GSA_COMA_COUNT){
				break;
			}
		}
	}
#ifdef PRINT_DEBUGGER
	printf("GSA: %.*s length(%d)\n", buffer_size-2, nmea_buffer, buffer_size);
#endif
	token = strtok(nmea_buffer, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	if (token) gps.fix_mode = atoi(token);
	token = strtok(&nmea_buffer[temp_index], ",");
	if (token) gps.pdop = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.hdop = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.vdop = (float) atof(token);
#ifdef PRINT_DEBUGGER
	printf("Fix mode: %d\n", gps.fix_mode);	//1 - no fix | 2 - 2D | 3 - 3D |
	printf("PDOP: %f\n", gps.pdop);
	printf("HDOP: %f\n", gps.hdop);
	printf("VDOP: %f\n", gps.vdop);
	printf("\n");
#endif
}
#endif
#ifdef	PARSE_GSV
void parse_gsv(void) {
	char *token;
	strncpy(nmea_buffer, (char*)rx_buffer, buffer_size);
#ifdef PRINT_DEBUGGER
	printf("GSV: %.*s length(%d)\n", buffer_size-2, nmea_buffer, buffer_size);
#endif
	token = strtok(nmea_buffer, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	token = strtok(NULL, ",");
	if (token) gps.satellites_visible = atoi(token);
#ifdef PRINT_DEBUGGER
	printf("satellites_visible: %d\n", gps.satellites_visible);
	printf("\n");
#endif
}
#endif

#ifdef	PARSE_GLL
void parse_gll(void) {
	char *token;
	strncpy(nmea_buffer, (char*)rx_buffer, buffer_size);
#ifdef PRINT_DEBUGGER
	printf("GLL: %.*s length(%d)\n", buffer_size-2, nmea_buffer, buffer_size);
#endif
	token = strtok(nmea_buffer, ",");
	token = strtok(NULL, ",");
	if (token) gps.latitude = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.lat_direction = token[0];
	token = strtok(NULL, ",");
	if (token) gps.longitude = (float) atof(token);
	token = strtok(NULL, ",");
	if (token) gps.lon_direction = token[0];
	token = strtok(NULL, ",");
	if (token) gps.time = (float) atof(token);
#ifdef PRINT_DEBUGGER
	printf("Latitude: %f\n", gps.latitude);
	printf("Lat_dir: %c\n", gps.lat_direction);
	printf("longitude: %f\n", gps.longitude);
	printf("Long_dir: %c\n", gps.lon_direction);
	printf("Time: %f\n", gps.time);
	printf("\n");
#endif
}
#endif

char* gps_complete_location_string() {
	static char buffer[32];
	snprintf(buffer, sizeof(buffer), "%.6g %c, %.6g %c", gps_latitude(), gps_lat_direction(), gps_longitude(), gps_lon_direction());
	return buffer;
}

float gps_latitude(){
	int minutes = gps.latitude/100;
	float seconds = (gps.latitude-minutes*100)/60;
	return minutes+seconds;
}

char gps_lat_direction(){
	return	gps.lat_direction;
}

float gps_longitude(){
	int minutes = gps.longitude/100;
	float seconds = (gps.longitude-minutes*100)/60;
	return minutes+seconds;
}

char gps_lon_direction(){
	return	gps.lon_direction;
}

int gps_altitude(void){
	return gps.altitude;
}

//Time offset doesn't affect date. Date is always for UTC+0
char* gps_complete_date_string(void){
	static char buffer[20];
	if(gps_day() < 10)	sprintf(buffer, "0%d/", gps_day());
	else	sprintf(buffer, "%d/", gps_day());
	if(gps_month() < 10)	sprintf(buffer + strlen(buffer), "0%d/", gps_month());
	else	sprintf(buffer + strlen(buffer), "%d/", gps_month());
	sprintf(buffer + strlen(buffer), "%d", gps_year_long_format());
	return buffer;
}

int gps_day(void){
	return gps.date/10000;
}

int gps_month(void){
	return (gps.date-gps_day()*10000)/100;
}

int gps_year(void){
	return (gps.date-(gps_day()*10000)-(gps_month()*100));
}

int gps_year_long_format(void){
	return (2000 + gps_year());
}

char* gps_complete_time_string(int offset){
	static char buffer[20];
	if(gps_hour(offset) < 10)	sprintf(buffer, "0%d:", gps_hour(offset));
	else	sprintf(buffer, "%d:", gps_hour(offset));
	if(gps_minute() < 10)	sprintf(buffer + strlen(buffer), "0%d:", gps_minute());
	else	sprintf(buffer + strlen(buffer), "%d:", gps_minute());
	if(gps_second() < 10)	sprintf(buffer + strlen(buffer), "0%d", gps_second());
	else	sprintf(buffer + strlen(buffer), "%d", gps_second());
	return buffer;
}

int gps_hour(int offset){
	int hour = gps.time / 10000;
	hour += offset;
	if (hour < 0) hour += 24;
	else if (hour >= 24) hour -= 24;
	return hour;
}

int gps_minute(void){
	int hour = gps.time / 10000;
	return (gps.time - hour * 10000) / 100;
}

int gps_second(void){
	int hour = gps.time / 10000;
	int minute = (gps.time - hour * 10000) / 100;
	return gps.time - hour * 10000 - minute * 100;
}

int gps_satellites_visible(void){
	return gps.satellites_visible;
}

int gps_satellites_in_use(void){
	return gps.satellites_in_use;
}

char* gps_fix_mode_string(void){
	static char buffer[10];
	switch(gps.fix_mode) {
	case 1:
		strcpy(buffer, "NO FIX");
		break;
	case 2:
		strcpy(buffer, "2D FIX");
		break;
	case 3:
		strcpy(buffer, "3D FIX");
		break;
	default:
		strcpy(buffer, "FIX ERROR");
		break;
	}
	return buffer;
}

char* gps_quality_string(void){
	static char buffer[16];
	switch(gps.gps_quality) {
	case 0:
		strcpy(buffer, "FIX NOT VALID");
		break;
	case 1:
		strcpy(buffer, "GPS FIX");
		break;
	case 2:
		strcpy(buffer, "3D FIX");
		break;
	case 3:
		strcpy(buffer, "FIX NOT VALID");
		break;
	case 4:
		strcpy(buffer, "GPS FIX");
		break;
	case 5:
		strcpy(buffer, "3D FIX");
		break;
	case 6:
		strcpy(buffer, "3D FIX");
		break;
	default:
		strcpy(buffer, "FIX ERROR");
		break;
	}
	return buffer;
}

float gps_pdop(void){
	return gps.pdop;
}

float gps_hdop(void){
	return gps.hdop;
}

float gps_vdop(void){
	return gps.vdop;
}

float gps_speed_knots(void){
	return gps.ground_speed_knots;
}

float gps_speed_kph(void){
	return gps.ground_speed_kph;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == GPS_UART)
	{
		if(rx_buffer_index < RX_BUFFER_SIZE) {
			rx_buffer[rx_buffer_index++] = received_byte;
		}
		else{
			rx_buffer_index = 0;
		}
		if(received_byte == '\n'){
			HAL_GPIO_TogglePin(test_1_GPIO_Port, test_1_Pin);
			buffer_size = rx_buffer_index;
			rx_buffer_index = 0;
			if(gps_validate_data()){
				gps_data_ready_flag = 1;
				gps_parse_data();
			}
			HAL_GPIO_TogglePin(test_1_GPIO_Port, test_1_Pin);
		}
		HAL_UART_Receive_IT(GPS_UART, (uint8_t *)&received_byte, 1);
	}
}

