# GPS HAL Driver based on interrupts

### UART SETUP

- 9600 baud
- 8N1
- NVIC global interrupt


### How to use:
```
#include "GPS.h"

int main(void){
  gps_init();
  extern NMEAData gps;  //for struct elements access only
  while(1)
  {
    if(gps_is_data_ready()){
      gps_parse_data();
      printf("%s\n", gps_complete_location_string());
      printf("Alt: %.2f m\n", gps.altitude);
    }
  }
}
```


Parsing can happen inside interrupt, which wont require any code in while loop.

GPS.c
```
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  if(huart == GPS_UART){
    if(rx_buffer_index < RX_BUFFER_SIZE){
      rx_buffer[rx_buffer_index++] = received_byte;
      }
    else{
      rx_buffer_index = 0;
    }
    if(received_byte == '\n'){
      buffer_size = rx_buffer_index;
      rx_buffer_index = 0;
      if(gps_validate_data()){
        gps_data_ready_flag = 1;
        gps_parse_data();  //parsing inside interrupt
      }
    }
    HAL_UART_Receive_IT(GPS_UART, (uint8_t *)&received_byte, 1);
  }
}
```


Inside GPS.h and GPS.c, additional functions can be deleted for memory optimization.

These are functions that return variable values without need for externing struct, or parse variables into readable strings.

gps_complete_location_string();  - 52.2292 N, 21.0066 E

gps_complete_date_string();    - 04/03/2025

gps_complete_time_string(1);    - 14:23:20

Do not delete core functions.

ADITIONAL FUNCTIONS:
```
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
```

