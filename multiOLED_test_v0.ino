/* original info and libraries from https://github.com/bitbank2/Multi_OLED
* v.0 - niq_ro tested first on Arduino Nano (just text)
*/

#include <Multi_BitBang.h>
#include <Multi_OLED.h>

#define NUM_DISPLAYS 4
#define NUM_BUSES 4
// I2C bus info for Arduino (Nano)
uint8_t scl_list[NUM_BUSES] = {9, 9, 9, 9}; //{9,9,9,9};
uint8_t sda_list[NUM_BUSES] = {5, 6, 7, 8}; //{5,6,7,8};
int32_t speed_list[NUM_BUSES] = {400000L, 400000L, 400000L, 400000L};
// OLED display info
uint8_t bus_list[NUM_DISPLAYS] = {0,1,2,3}; // can be multiple displays per bus
uint8_t addr_list[NUM_DISPLAYS] = {0x3c, 0x3c, 0x3c, 0x3c};
uint8_t type_list[NUM_DISPLAYS] = {OLED_128x64, OLED_128x64, OLED_128x64, OLED_128x64};
uint8_t flip_list[NUM_DISPLAYS] = {0,0,0,0};
uint8_t invert_list[NUM_DISPLAYS] = {0,0,0,0};

void setup() {
  // put your setup code here, to run once:
  Multi_I2CInit(sda_list, scl_list, speed_list, NUM_BUSES);
  Multi_OLEDInit(bus_list, addr_list, type_list, flip_list, invert_list, NUM_DISPLAYS);
} // setup

void loop() {
  // put your main code here, to run repeatedly:
uint8_t i;
char szTemp[16];

  for (i=0; i<NUM_DISPLAYS; i++)
  {
    Multi_OLEDFill(i, 0);
    Multi_OLEDSetContrast(i, 20);
    Multi_OLEDWriteString(i, 0, 0, (char *)"Display", FONT_NORMAL, 0);
    sprintf(szTemp, "Num: %d", i);
    Multi_OLEDWriteString(i, 0, 2, szTemp, FONT_NORMAL, 0);
    Multi_OLEDDrawLine(1, 0, 0, 60, 50);
  }
  delay(5000);  
} // loop
