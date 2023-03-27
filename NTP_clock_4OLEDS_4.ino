/* original info and libraries from https://github.com/bitbank2/Multi_OLED
* v.0a - niq_ro tested first on Arduino Nano (just text)
* v.0a1 - niq_ro tested on Wemos D1 Mini (ESP8266)
* v.1 - niq_ro defined 7-segment number as https://github.com/tehniq3/NTP_clock_4_OLED_displays 
*       based on // https://ta-laboratories.com/blog/2018/09/07/recreating-a-7-segment-display-with-adafruit-gfx-ssd1306-oled/
* v.1a - thick 7-segment characters (show 0,1,2,3)
* v.1b - show all characters 
* v.4 - combined with NTP clock from https://github.com/tehniq3/NTP_clock_4_OLED_displays/blob/main/NTP_clock_TCA9584A_4OLEDs_3.ino
*       but used analog input (no free stable pins), second is static, not blink
*/

#include <Multi_BitBang.h>
#include <Multi_OLED.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid     = "niq_ro";
const char *password = "berelaburtica";

const long timezoneOffset = 2*3600; // GMT + seconds  (Romania)
#define DSTpin  A0 // ADC0, see https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timezoneOffset);

int ora, minut, secunda;
byte DST = 0;
byte DST0;
byte oz1, ou1, mz1, mu1;
byte oz0 = 10;
byte ou0 = 10;
byte mz0 = 10;
byte mu0 = 10;
byte secunda0 = 60;

uint8_t n1 = 38;
uint8_t n3 = 8;
uint8_t n2 = 51;
uint8_t r = 4;

#define NUM_DISPLAYS 4
#define NUM_BUSES 4
// I2C bus info
// Wemos D1 - https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
uint8_t scl_list[NUM_BUSES] = {5, 5, 5, 5}; //{D1,D1,D1,D1}; Wemos D1 (ESP8266)
uint8_t sda_list[NUM_BUSES] = {4, 14, 12, 13}; //{D2,D5,D6,D7}; Wemos D1 (ESP8266)

int32_t speed_list[NUM_BUSES] = {400000L, 400000L, 400000L, 400000L};
// OLED display info
uint8_t bus_list[NUM_DISPLAYS] = {0,1,2,3}; // can be multiple displays per bus
uint8_t addr_list[NUM_DISPLAYS] = {0x3c, 0x3c, 0x3c, 0x3c};
uint8_t type_list[NUM_DISPLAYS] = {OLED_128x64, OLED_128x64, OLED_128x64, OLED_128x64};
uint8_t flip_list[NUM_DISPLAYS] = {0,0,0,0};
uint8_t invert_list[NUM_DISPLAYS] = {0,0,0,0};
int ics1, igrec1, ics2, igrec2;

// define segment truth table for each digit
/*
 *      --a-- 
 *     |     | 
 *     f     b
 *     |     |
 *      --g--
 *     |     |
 *     e     c
 *     |     |
 *      --d--  dp
 *   
 */

static const int digit_array[14][7] = {
 //a, b, c, d, e, f, g 
  {1, 1, 1, 1, 1, 1, 0},  // 0
  {0, 1, 1, 0, 0, 0, 0},  // 1
  {1, 1, 0, 1, 1, 0, 1},  // 2
  {1, 1, 1, 1, 0, 0, 1},  // 3
  {0, 1, 1, 0, 0, 1, 1},  // 4
  {1, 0, 1, 1, 0, 1, 1},  // 5
  {1, 0, 1, 1, 1, 1, 1},  // 6
  {1, 1, 1, 0, 0, 0, 0},  // 7
  {1, 1, 1, 1, 1, 1, 1},  // 8
  {1, 1, 1, 1, 0, 1, 1},  // 9
  {1, 1, 0, 0, 0, 1, 1},  // degree
  {0, 0, 1, 1, 1, 0, 1},  // o
  {1, 0, 0, 1, 1, 1, 0},  // C
  {0, 0, 0, 0, 0, 0, 1}   // - 
};

uint8_t cifra = 0;
byte cnt =0;
byte caractere = 0;

void setup() {
  // put your setup code here, to run once:
  Multi_I2CInit(sda_list, scl_list, speed_list, NUM_BUSES);
  Multi_OLEDInit(bus_list, addr_list, type_list, flip_list, invert_list, NUM_DISPLAYS);

//  pinMode (DSTpin, INPUT);
  
  Serial.begin(115200);
  Serial.println (" ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay (500);
    Serial.print (".");
  }
    Serial.println (" ");
    
  if (analogRead(DSTpin) < 100)
   DST = 0;
  else
   DST = 1;
  timeClient.setTimeOffset(timezoneOffset + DST*3600);
  timeClient.begin();
  DST0 = DST;

} // end setup

void loop() {
  // put your main code here, to run repeatedly:
if (analogRead(DSTpin) < 100)
   DST = 0;
  else
   DST = 1;
  
timeClient.update();
 ora = timeClient.getHours();
 minut = timeClient.getMinutes();
/*
 ora = ora%12;  // 12-hour format
 if (ora == 0) ora = 12;  // 12-hour format
*/
oz1 = ora/10;
ou1 = ora%10;
mz1 = minut/10;
mu1 = minut%10;
Serial.print(ora);
Serial.print(";");
Serial.println(minut);

if (oz1 != oz0)
{
  Multi_OLEDFill(3, 0);
  Multi_OLEDSetContrast(3, 20);
  render_digit (3, oz1);  
}

//if ((ou1 != ou0) or (secunda != secunda0))
if (ou1 != ou0)
{
  Multi_OLEDFill(2, 0);
  Multi_OLEDSetContrast(2, 20);
  render_digit (2, ou1); 
  has(2); 
}
if (mz1 != mz0)
{
  Multi_OLEDFill(1, 0);
  Multi_OLEDSetContrast(1, 20);
  render_digit (1, mz1);  
}
if (mu1 != mu0)
{
  Multi_OLEDFill(0, 0);
  Multi_OLEDSetContrast(0, 20);
  render_digit (0, mu1);  
}
if (DST != DST0)
{
  timeClient.setTimeOffset(timezoneOffset + DST*3600);
  timeClient.begin();
DST0 = DST;
}

oz0 = oz1;
ou0 = ou1;
mz0 = mz1;
mu0 = mu1;
secunda0 = secunda;
secunda = (millis()/500)%2;
delay(500);

} // end main loop


// RENDER DIGIT
// don't use this unless you only need a single digit
// use render_digits() func above instead
void render_digit(uint8_t afisaj, uint8_t numar) {
  // loop through 7 segments
  for (uint8_t k = 0; k < 7; k++) {
    bool seg_on = digit_array[numar][k];
    // if seg_on is true draw segment
    if (seg_on) {
      switch (k) {
        case 0:  // SEG a
          Multi_OLEDDrawLine(afisaj, 127,   0, 127,  55);
          Multi_OLEDDrawLine(afisaj, 126,   1, 126,  54);
          Multi_OLEDDrawLine(afisaj, 125,   2, 125,  53);
          Multi_OLEDDrawLine(afisaj, 124,   3, 124,  52);
          Multi_OLEDDrawLine(afisaj, 123,   4, 123,  51); 
          Multi_OLEDDrawLine(afisaj, 122,   5, 122,  50);
          Multi_OLEDDrawLine(afisaj, 121,   6, 121,  49); 
          Multi_OLEDDrawLine(afisaj, 120,   7, 120,  48);
          Multi_OLEDDrawLine(afisaj, 119,   8, 119,  47);              
          break;
        case 1:  // SEG b
          Multi_OLEDDrawLine(afisaj, 127,  55,  63,  55);
          Multi_OLEDDrawLine(afisaj, 126,  54,  63,  54);
          Multi_OLEDDrawLine(afisaj, 125,  53,  64,  53);
          Multi_OLEDDrawLine(afisaj, 124,  52,  65,  52);
          Multi_OLEDDrawLine(afisaj, 123,  51,  65,  51);
          Multi_OLEDDrawLine(afisaj, 122,  50,  66,  50);
          Multi_OLEDDrawLine(afisaj, 121,  49,  66,  49); 
          Multi_OLEDDrawLine(afisaj, 120,  48,  67,  48);
          Multi_OLEDDrawLine(afisaj, 119,  47,  67,  47);              
          break;
        case 2: // SEG c
          Multi_OLEDDrawLine(afisaj,  63,  55,   0,  55);
          Multi_OLEDDrawLine(afisaj,  63,  54,   1,  54);
          Multi_OLEDDrawLine(afisaj,  62,  53,   2,  53);
          Multi_OLEDDrawLine(afisaj,  61,  52,   3,  52);
          Multi_OLEDDrawLine(afisaj,  61,  51,   4,  51); 
          Multi_OLEDDrawLine(afisaj,  60,  50,   5,  50); 
          Multi_OLEDDrawLine(afisaj,  60,  49,   6,  49); 
          Multi_OLEDDrawLine(afisaj,  59,  48,   7,  48); 
          Multi_OLEDDrawLine(afisaj,  59,  47,   8,  47);      
          break;
        case 3: // SEG d
          Multi_OLEDDrawLine(afisaj,    0,   0,   0,  55);
          Multi_OLEDDrawLine(afisaj,    1,   1,   1,  54);
          Multi_OLEDDrawLine(afisaj,    2,   2,   2,  53);
          Multi_OLEDDrawLine(afisaj,    3,   3,   3,  52);
          Multi_OLEDDrawLine(afisaj,    4,   4,   4,  51); 
          Multi_OLEDDrawLine(afisaj,    5,   5,   5,  50);
          Multi_OLEDDrawLine(afisaj,    6,   6,   6,  49);
          Multi_OLEDDrawLine(afisaj,    7,   7,   7,  48);
          Multi_OLEDDrawLine(afisaj,    8,   8,   8,  47);                      
          break;
        case 4: // SEG e
          Multi_OLEDDrawLine(afisaj,    0,   0,   63,  0);
          Multi_OLEDDrawLine(afisaj,    1,   1,   63,  1);
          Multi_OLEDDrawLine(afisaj,    2,   2,   62,  2);
          Multi_OLEDDrawLine(afisaj,    3,   3,   61,  3);
          Multi_OLEDDrawLine(afisaj,    4,   4,   61,  4);
          Multi_OLEDDrawLine(afisaj,    5,   5,   60,  5);
          Multi_OLEDDrawLine(afisaj,    6,   6,   60,  6);
          Multi_OLEDDrawLine(afisaj,    7,   7,   59,  7);
          Multi_OLEDDrawLine(afisaj,    8,   8,   59,  8);               
          break;
        case 5: // SEG f
          Multi_OLEDDrawLine(afisaj, 127,    0,  63,   0);
          Multi_OLEDDrawLine(afisaj, 126,    1,  63,   1);
          Multi_OLEDDrawLine(afisaj, 125,    2,  64,   2);
          Multi_OLEDDrawLine(afisaj, 124,    3,  65,   3);
          Multi_OLEDDrawLine(afisaj, 123,    4,  65,   4);
          Multi_OLEDDrawLine(afisaj, 122,    5,  66,   5);
          Multi_OLEDDrawLine(afisaj, 121,    6,  66,   6);
          Multi_OLEDDrawLine(afisaj, 120,    7,  67,   7);
          Multi_OLEDDrawLine(afisaj, 119,    8,  67,   8);        
          break;
        case 6: // SEG g
          Multi_OLEDDrawLine(afisaj,  67,   8,   67,  47);
          Multi_OLEDDrawLine(afisaj,  66,   6,   66,  49);      
          Multi_OLEDDrawLine(afisaj,  65,   4,   65,  51);
          Multi_OLEDDrawLine(afisaj,  64,   2,   64,  53);
          Multi_OLEDDrawLine(afisaj,  63,   0,   63,  55);
          Multi_OLEDDrawLine(afisaj,  62,   2,   62,  53);
          Multi_OLEDDrawLine(afisaj,  61,   4,   61,  51);
          Multi_OLEDDrawLine(afisaj,  60,   6,   60,  49);
          Multi_OLEDDrawLine(afisaj,  59,   8,   59,  47);                  
          break;
      }
      seg_on = false;
    }
  }
}

void has (uint8_t afisaj1) {
    Multi_OLEDDrawLine(afisaj1,  0,  60,  0,  61);
    Multi_OLEDDrawLine(afisaj1,  1,  59,  1,  62);
    Multi_OLEDDrawLine(afisaj1,  2,  58,  2,  63);
    Multi_OLEDDrawLine(afisaj1,  3,  58,  3,  63);
    Multi_OLEDDrawLine(afisaj1,  4,  58,  4,  63);
    Multi_OLEDDrawLine(afisaj1,  5,  58,  5,  63);
    Multi_OLEDDrawLine(afisaj1,  6,  58,  6,  63);
    Multi_OLEDDrawLine(afisaj1,  7,  58,  7,  63);
    Multi_OLEDDrawLine(afisaj1,  8,  58,  8,  63); 
    Multi_OLEDDrawLine(afisaj1,  9,  58,  9,  63);
    Multi_OLEDDrawLine(afisaj1, 10,  58, 10,  63);
    Multi_OLEDDrawLine(afisaj1, 11,  59, 11,  62);
    Multi_OLEDDrawLine(afisaj1, 12,  60, 12,  61);         
}
