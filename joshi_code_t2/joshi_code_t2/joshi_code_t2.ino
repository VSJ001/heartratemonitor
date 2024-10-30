#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
MAX30105 particleSensor; 
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;//Time at which the last beat occurred
#define  ST7735_BLACK   0x0000
#define ST7735_BLUE    0xF800
#define ST7735_RED     0x001F
#define ST7735_ORANGE  0x00AA  //TEST CREATING COLOR
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0xFFE0
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0x07FF
#define ST7735_WHITE   0xFFFF

float beatsPerMinute;
int beatAvg;                                  
#include <Adafruit_GFX.h>                                                 // Graphics library for drawin on screen
#include <Adafruit_ST7735.h>                                              // Hardware-specific library
#define SCALING 12                                                  // Scale height of trace, reduce value to make trace height                                                                // bigger, increase to make smaller
#define TRACE_SPEED 0.5                                             // Speed of trace across screen, higher=faster   
#define TRACE_MIDDLE_Y_POSITION 41                                  // y pos on screen of approx middle of trace
#define TRACE_HEIGHT 64                                             // Max height of trace in pixels    
#define HALF_TRACE_HEIGHT TRACE_HEIGHT/2                            // half Max height of trace in pixels (the trace amplitude)    
#define TRACE_MIN_Y TRACE_MIDDLE_Y_POSITION-HALF_TRACE_HEIGHT+1     // Min Y pos of trace, calculated from above values
#define TRACE_MAX_Y TRACE_MIDDLE_Y_POSITION+HALF_TRACE_HEIGHT-1     // Max Y pos of trace, calculated from above values
 
// Pins to use with the 7735 display
#define TFT_CS     10   // Chop select
#define TFT_RST    9    // Reset
#define TFT_RS     8    // Register select
 
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_RS, TFT_RST);
 
 
void setup()
{
  pinMode(3,INPUT);
    Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED


  //Setup to sense a nice looking saw tooth on the plotter
//  byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
//  byte sampleAverage = 8; //Options: 1, 2, 4, 8, 16, 32
//  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
//  int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
//  int pulseWidth = 411; //Options: 69, 118, 215, 411
//  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
//
//   particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

    tft.initR(INITR_GREENTAB);   // initialize a ST7735S chip, for 128x128 display
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
    tft.setTextColor(ST7735_BLACK);   
    tft.setTextSize(2);
    tft.setCursor(5,86);
    tft.print("BPM Status");
    tft.setCursor(92,86);
     tft.setTextSize(1);                          // The small subscriper 2 of O2
    tft.setCursor(5,0);   
    tft.print("Photoplethysmography");
    tft.setTextSize(2);
    tft.drawRect(0,TRACE_MIN_Y-1,128,TRACE_HEIGHT+2,ST7735_MAGENTA);     // The border box for the trace
        
}
 
void loop()
{
 
  int16_t Diff=0;                             // The difference between the Infra Red (IR) and Red LED raw results
  uint16_t ir=particleSensor.getIR(), red=particleSensor.getRed();                           // raw results returned in these
  static float lastx=1;                       // Last x position of trace
  static int lasty=TRACE_MIDDLE_Y_POSITION;   // Last y position of trace, default to middle
  static float x=1;                           // current x position of trace
  int32_t y;                                  // current y position of trace
  uint8_t BPM,O2;                             // BPM and O2 values
  static uint32_t tsLastReport = 0;           // Last time BMP/O2 were checked
  static int32_t SensorOffset=10000;       // Offset to lowest point that raw data does not go below, default 10000
                                              // Note that as sensors may be slightly different the code adjusts this
                                              // on the fly if the trace is off screen. The default was determined
                                              // By analysis of the raw data returned 
                                                
  long irfinger = particleSensor.getIR(); 
  if(irfinger< 50000){
    tone(3,450);
    
    Serial.print(" No finger");
    tft.setCursor(50,110); 
    tft.setTextSize(1);
     tft.setTextColor(ST7735_BLUE,ST7735_CYAN);
     tft.print("No finger");
    
    }
    else{
      noTone(3);
      Serial.print("Detected");
      tft.setCursor(50,110); 
    tft.setTextSize(1);
     tft.setTextColor(ST7735_BLUE,ST7735_CYAN);
     tft.print("Detected!");
      }                          
  if(particleSensor.getIR()&&particleSensor.getRed())          // If raw data available for IR and Red 
  {
    if(red<1000)
    {
      
      y=TRACE_MIDDLE_Y_POSITION;
    }
    else
    {
      // Plot our new point
      Diff=(ir-red);                          // Get raw difference between the 2 LEDS
      Diff=Diff-SensorOffset;                 // Adjust the baseline of raw values by removing the offset (moves into a good range for scaling)
      Diff=Diff/SCALING;                      // Scale the difference so that it appears at a good height on screen
      
      // If the Min or max are off screen then we need to alter the SensorOffset, this should bring it nicely on screen
      if(Diff<-HALF_TRACE_HEIGHT)
        SensorOffset+=(SCALING*(abs(Diff)-32));
      if(Diff>HALF_TRACE_HEIGHT)
        SensorOffset+=(SCALING*(abs(Diff)-32));
        
      y=Diff+(TRACE_MIDDLE_Y_POSITION-HALF_TRACE_HEIGHT);         // These two lines move Y pos of trace to approx middle of display area
      y+=TRACE_HEIGHT/4;                                              
    } 
    if(y>TRACE_MAX_Y) y=TRACE_MAX_Y;                              // If going beyond trace box area then crop the trace
    if(y<TRACE_MIN_Y) y=TRACE_MIN_Y;                              // so it stays within
    tft.drawLine(lastx,lasty,x,y,ST7735_RED);                  // Plot the next part of the trace
    lasty=y;                                                      // Save where the last Y pos was
    lastx=x;                                                      // Save where the last X pos was
    x+=TRACE_SPEED;                                               // Move trace along the display
    if(x>126)                                                     // If reached end of display then reset to statt
    {
//      tft.fillRect(1,TRACE_MIN_Y,126,TRACE_HEIGHT,ST7735_BLACK);  // Blank trace display area
//      x=1;                                                        // Back to start
//      lastx=x;
      heartrate_loop();
                                                          
    }
  }
  
  }
void heartrate_loop()
{
  bool heartBool = true;
  long irValue = particleSensor.getIR();
  while(heartBool)
  {
    long irValue = particleSensor.getIR();
    if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);
    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
   Serial.println();                  // Clear the old values                      
      if((beatAvg<60)||(beatAvg>110)){                                      // If too low or high for a resting heart rate then display in red
        tft.setTextColor(ST7735_RED);
        }
      else{
        tft.setTextColor(ST7735_BLACK);}// else display in green
        
      tft.setCursor(5,104);
      tft.setTextSize(2);
      tft.setTextColor(ST7735_BLUE,ST7735_CYAN);// Put BPM at this position
      tft.print(beatAvg);
      if(irValue< 50000){
     Serial.print(" No finger");
     tone(3,450);
    tft.setCursor(50,110); 
    tft.setTextSize(1);
     tft.setTextColor(ST7735_BLUE,ST7735_CYAN);
     tft.print("No finger");
    }
    else{
      noTone(3);
      Serial.print("Detected");
      tft.setCursor(50,110); 
    tft.setTextSize(1);
     tft.setTextColor(ST7735_BLUE,ST7735_CYAN);
     tft.print("Detected!");
      } 
   }

}
