#include <Wire.h>//i2c communication
#include "MAX30100_PulseOximeter.h"//heart rate library
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h>// Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> 
#include <SPI.h>//spi communication
//////////display defination//////////
#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8
#define  ST7735_BLACK   0x0000
#define ST7735_BLUE    0xF800
#define ST7735_RED     0x001F
#define ST7735_ORANGE  0x00AA  //TEST CREATING COLOR
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0xFFE0
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0x07FF
#define ST7735_WHITE   0xFFFF
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
/////////end display defination//////
/////heart rate defination//////////
#define REPORTING_PERIOD_MS     2000
PulseOximeter pox;
uint32_t tsLastReport = 0;
void onBeatDetected() {
    Serial.println("Beat!");
}
///////end heart rate defination////////
void setup() {
    Serial.begin(9600);
    //////display statements///////
    Serial.print(F("Hello! ST77xx TFT Test"));
    //////heart statements/////////
    
    tft.initR(INITR_GREENTAB);  
    

    
  // Configure sensor to use 7.6mA for LED drive
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback routine
    pox.setOnBeatDetectedCallback(onBeatDetected);
    Serial.println(F("Initialized"));
    tft.fillScreen(ST7735_CYAN);
    tft.setTextWrap(false);
    tft.fillScreen(ST7735_CYAN);
    tft.setCursor(5, 60);
    tft.setTextColor(ST7735_BLACK);
    tft.setTextSize(1);
    tft.println("Photoplethysmography");
    tft.setCursor(5, 70);
    tft.println("With Simple Arduino");
    tft.setCursor(5, 80);
    tft.println("Setup");
    delay(3000);
    tft.fillScreen(ST7735_CYAN);
        // Initialize sensor
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
    
    
   
}

void loop() {
//     tft.setTextWrap(false);
//        tft.fillScreen(ST77XX_CYAN);
//        tft.setCursor(30, 10);
//        tft.setTextColor(ST7735_RED);
//         tft.setTextSize(2);
//         tft.println(pox.getHeartRate());
//         tft.setTextColor(ST7735_RED);
        
   pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        tft.fillScreen(ST7735_CYAN);
        Serial.print("Heart rate:");
         tft.setCursor(30, 10);
         tft.setTextColor(ST7735_RED);
         tft.setTextSize(2);
         tft.println(pox.getHeartRate());
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
        tsLastReport = millis();  
    }
        
}
/////////////////////funtion///////////
