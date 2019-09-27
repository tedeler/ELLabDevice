#include "global.h"

#include "Arduino.h"



//...perTimer2_GW: Gewichteter Wert mit alpha an A8 (BNC1Pin)

/* Wichtige Anmerkung: Da das touch screen die Analogpins A0 bis A4 verwendet und
 * das Messen des Analogwertes an BNC1Pin (Analogpin A8) denselben vorhandenen ADC verwenden,
 * mussten diverse Anpassungen vorgenommen werden. Die Analogwerte an A8 werden durch einen
 * Timer ca. 44100 mal pro Sekunde gemessen. Ein Timer triggert per TIOA2 den ADC, der wiederum
 * einen Interrupt zum wandeln der anliegenden Spannung auslöst. Die gewandelten Spannungen
 * für das touch screen und den BNC1Pin liegen dann in den entsprechenden data register des
 * ADC. Grundlage für die Umsetzung war das Projekt: TFT_Due_Touch_test7_mit_ADC_perTimer4.
 */

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <math.h>
#include "MyTouchScreen.h"
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal) /*??*/
    #define F(string_literal) string_literal
#endif


#include "Labor.h"
#include "LaborLogAmplifier.h"
#include "LaborLogAmplifierTimeChoice.h"



// For the Arduino Due:
//   D0 connects to digital pin 8
//   D1 connects to digital pin 9
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7

#define debug 0
#define debug1 1

#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

//Min.-Koordinaten
#define TS_MINX 100
#define TS_MINY 220

//Max.-Koordinaten
#define TS_MAXX 907
#define TS_MAXY 880

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
// Gemessen hatte ich an einem TouchScreen 616 Ohm zwischen XP und XM;
// Versuch mit: TouchScreen ts = TouchScreen(XP, YP, XM, YM, 600); brachte
// keine Verbesserung im Touch-Verhalten
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
// Einstellen des minimalen und maximalen Drucks auf dem touch screen:
#define MINPRESSURE 10    //in TouchScreen.cpp pressureThreshold
#define MAXPRESSURE 1000

#define LCD_CS A3     // Chip Select goes to Analog 3
#define LCD_CD A2     // Command/Data goes to Analog 2
#define LCD_WR A1     // LCD Write goes to Analog 1
#define LCD_RD A0     // LCD Read goes to Analog 0
// optional
#define LCD_RESET A4  // Can alternately just connect to Arduino's reset pin


#define BUFFER_SIZE 44100

short oldcolor, currentcolor;

boolean z, start0=1, home0=1, labor1=0, labor2=0, labor21=0, labor3=0, paint0=1, paint1=0, paint11=0, paint2=1, paint21=0, paint22=0, paint3=1;
int counter, x,y, time_ms;
volatile uint16_t BNC1Pin = A8;
int BNC1Pin_sum;
volatile uint16_t BNC1Value;
volatile uint16_t ADC_xm, ADC_yp;
uint32_t buf[BUFFER_SIZE];
float BNC1average = 0;
float Temp = 0;
float Temp_old = 0;
int dig;  //digit für die Messwertdarstellung auf dem Display
int korr; //Korrekturwert, wenn der Messbereich 40 bis 80°C beträgt
unsigned short grafik;
bool initTemp = true;
unsigned long last=0;
unsigned long current=0;
int toggle = 31;
int pinState = LOW;
float alpha = 0.1;
float Usum, Ueff;
int zaehler;

//Für Interrupt0:
int CLK = 22;//CLK->
int DT = 23;//DT->
int SW = 24;//SW->
//+ auf 3,3V und GND
const int interrupt0 = 22;
int zaehler2;
bool aktualisieren = true;
unsigned short resolution;
int Rotary1Counter = 0;//Define the count
int lastCLK = 0;//CLK initial value

int MyAnalogRead(uint8_t pin);

//Test lookup table:
//float dB_lookup[37]={0.000316228, 0.000398107, 0.000501187, 0.000630957, 0.000794328, 0.001, 0.001258925, 0.001584893, 0.001995262, 0.002511886, 0.003162278, 0.003981072, 0.005011872, 0.006309573,
//0.007943282, 0.01, 0.012589254, 0.015848932, 0.019952623, 0.025118864, 0.031622777, 0.039810717, 0.050118723, 0.063095734, 0.079432823, 0.1, 0.125892541, 0.158489319, 0.199526231, 0.251188643,
//0.316227766, 0.398107171, 0.501187234, 0.630957344, 0.794328235, 1};
short i=0, zmax=0, zmax_old=0;

void setup(void) {
  //SerialUSB.begin(9600);
  //while (!SerialUSB) {
  //  ; // wait for Serial port to connect
  //}
    Serial.begin(115200);
 #if (debug)
    Serial.println(F("TFT LCD test"));
 #endif
  tft.reset();

  uint16_t identifier = tft.readID();
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  }else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  }else if(identifier == 0x7783) {
    Serial.println(F("Found ST7781 LCD driver"));
  }else if(identifier == 0x8230) {
    Serial.println(F("Found UC8230 LCD driver"));
  }
  else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if(identifier==0x0101)
  {
      identifier=0x9341;
       Serial.println(F("Found 0x9341 LCD driver"));
  }else if(identifier==0x9481)
  {
       Serial.println(F("Found 0x9481 LCD driver"));
  }
  else if(identifier==0x9486)
  {
       Serial.println(F("Found 0x9486 LCD driver"));
  }
  else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    identifier=0x9486;
  }
  tft.begin(identifier);
  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());
  tft.setRotation(1);
  tft.fillScreen(WHITE);
  pinMode(13, OUTPUT);
  #if (debug)
      Serial.print("ADC_xm: ");
      Serial.println(ADC_xm);
      Serial.print("ADC_yp: ");
      Serial.println(ADC_yp);
  #endif
  pinMode(toggle, OUTPUT);
  //rotary 1:
  pinMode(SW, INPUT_PULLUP);
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  attachInterrupt(interrupt0, ClockChanged, CHANGE);//Set the interrupt 0 handler, trigger level change
  tc_setup();
  adc_setup();
}

void DisplayStartScreen(){
     //tft.fillRect(x1, y1, x-Breite, y-Höhe, Farbe)
     tft.fillScreen(WHITE);
     for(int z=0; z<4; z++){
         tft.fillRect(20, 20+20*z, 60, 4, BLUEHAW1);  //Farbe: HAW hellblau
     }
     for(int z=0; z<4; z++){
         tft.fillRect(36, 30+20*z, 60, 4, BLUEHAW2);  //Farbe: HAW dunkelblau
     }
     tft.setCursor(114, 30);
     tft.setTextColor(BLUEHAW2);    tft.setTextSize(3);
     tft.println("HAW");

     tft.setCursor(390, 30);
     tft.setTextColor(BLUEHAW2);    tft.setTextSize(3);
     tft.println("V1.2");

     tft.setCursor(114, 63);
     tft.setTextColor(BLUEHAW2);    tft.setTextSize(3);
     tft.println("HAMBURG");

     tft.fillRect(SIDE_H, 120, 250, BOXSIZE, BLUEHAW2); //X1, Y1, X-Breite, Y-Höhe, Farbe
     tft.fillRect(SIDE_H, 120 + SIDE_V + BOXSIZE, 250, BOXSIZE, BLUEHAW2);
     tft.fillRect(SIDE_H, 120 + 2*SIDE_V + 2*BOXSIZE, 250, BOXSIZE, BLUEHAW2);

     tft.setCursor(175, 130);
     tft.setTextColor(WHITE);    tft.setTextSize(3);
     tft.println("Labor 1");

     tft.setCursor(175, 200);
     tft.setTextColor(WHITE);    tft.setTextSize(3);
     tft.println("Labor 2");

     tft.setCursor(175, 270);
     tft.setTextColor(WHITE);    tft.setTextSize(3);
     tft.println("Labor 3");
     paint0=0;
}


int touch(){
     digitalWrite(13, HIGH);
     TSPoint p = ts.getPoint();
     digitalWrite(13, LOW);

     //if sharing pins, you'll need to fix the directions of the touchscreen pins
     // pinMode(XP, OUTPUT);
     pinMode(XM, OUTPUT);
     pinMode(YP, OUTPUT);
     //pinMode(YM, OUTPUT);

     //we have some minimum pressure we consider 'valid'
    //pressure of 0 means no pressing!

    if(p.z > MINPRESSURE && p.z < MAXPRESSURE) {
//    #if (debug)
       Serial.print("p.x: ");
       Serial.println(p.x);
       Serial.print("p.y: ");
       Serial.println(p.y);
//    #endif
    //scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, 0,tft.width());     //tft.width for 480 pixel
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());   //tft.hight for 320 pixel
//    #if (debug)
       Serial.print("p.x_new: ");
       Serial.println(p.x);
       Serial.print("p.y_new: ");
       Serial.println(p.y);
//    #endif
    x = p.x;
    y = p.y;
    }
    return x, y;
}

TSPoint ReadDisplayTouch(){
	digitalWrite(13, HIGH);
	TSPoint p = ts.getPoint();
	digitalWrite(13, LOW);

	pinMode(XM, OUTPUT);
	pinMode(YP, OUTPUT);

	p.z = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
	p.x = map(p.x, TS_MINX, TS_MAXX, 0,tft.width());     //tft.width for 480 pixel
	p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());   //tft.hight for 320 pixel

	return p;
}


void ppm(uint16_t x, uint16_t y){
    //Darstellung PPM:
    tft.fillScreen(BLACK);    //Bildschirm schwarz
    tft.setCursor(x, y);
    tft.setTextColor(WHITE);    tft.setTextSize(3);
    tft.println("PPM");
}

void pt1000(uint16_t x, uint16_t y){
    //Darstellung PT1000:
    tft.fillScreen(BLACK);    //Bildschirm schwarz
    tft.setCursor(x, y);
    tft.setTextColor(YELLOW);    tft.setTextSize(3);
    tft.println("PT1000");
}

void temp_akt(uint16_t x, uint16_t y){
    //Darstellung: Aktuelle Temperatur:
    tft.setCursor(x, y);
    tft.setTextColor(WHITE);    tft.setTextSize(3);
    tft.print("Aktuelle Temperatur");
    tft.println(':');
}

void ms_time(){
    if(resolution >= 1 && resolution <= 3){
        tft.setTextColor(YELLOW);
        tft.setCursor(25, 121);
        tft.println("10ms");
        tft.setTextColor(WHITE);
        tft.setCursor(25, 157);
        tft.println("100ms");
        tft.setCursor(25, 193);
        tft.println("1000ms");
    }
    if(resolution >= 4 && resolution <= 6){
        tft.setTextColor(YELLOW);
        tft.setCursor(25, 157);
        tft.println("100ms");
        tft.setTextColor(WHITE);
        tft.setCursor(25, 121);
        tft.println("10ms");
        tft.setCursor(25, 193);
        tft.println("1000ms");
    }

    if(resolution >= 7 && resolution <= 9){
        tft.setTextColor(WHITE);
        tft.setCursor(25, 121);
        tft.println("10ms");
        tft.setCursor(25, 157);
        tft.println("100ms");
        tft.setTextColor(YELLOW);
        tft.setCursor(25, 193);
        tft.println("1000ms");
    }
}

void home_button(){
    tft.setCursor(35, 260);
    tft.setTextColor(WHITE);    tft.setTextSize(3);
    tft.println("Home");
    tft.drawRect(25, 250, 88,43,WHITE);
}

void back_button(){
    tft.setCursor(35, 260);
    tft.setTextColor(WHITE);    tft.setTextSize(3);
    tft.println("Back");
    tft.drawRect(25, 250, 88,43,WHITE);
}

void temp_grafik(){
    tft.setCursor(25, 160);
    tft.setTextColor(WHITE);    tft.setTextSize(3);
    tft.println("Grafik: ");
    tft.setCursor(180, 160);
    tft.println("0" "\xF7" "C" " bis " "40" "\xF7" "C");
    tft.drawRect(170, 149, 253, 43,WHITE);
    tft.setCursor(180, 220);
    tft.println("40" "\xF7" "C" " bis " "80" "\xF7" "C");
    tft.drawRect(170, 208, 253, 43,WHITE);
}

void KAchsen(uint16_t u){
    int i;
    float z=0;
    //Darstellung °C:
    tft.setCursor(111, 85);
    tft.setTextColor(WHITE);    tft.setTextSize(2);
    tft.println("\xF7" "C");
    tft.setCursor(111, 159);
    tft.setTextColor(WHITE);    tft.setTextSize(2);
    tft.println(u);
    //Darstellung sek:
    tft.setCursor(397, 247);
    tft.setTextColor(WHITE);    tft.setTextSize(2);
    tft.println("sec");
    //Darstellung 2:
    tft.setCursor(183, 247);
    tft.setTextColor(WHITE);    tft.setTextSize(2);
    tft.println("2");
    //Darstellung 4:
    tft.setCursor(223, 247);
    tft.setTextColor(WHITE);    tft.setTextSize(2);
    tft.println("4");
    //Darstellung 6:
    tft.setCursor(263, 247);
    tft.setTextColor(WHITE);    tft.setTextSize(2);
    tft.println("6");
    //Darstellung 8:
    tft.setCursor(303, 247);
    tft.setTextColor(WHITE);    tft.setTextSize(2);
    tft.println("8");
    //Darstellung 10:
    tft.setCursor(337, 247);
    tft.setTextColor(WHITE);    tft.setTextSize(2);
    tft.println("10");
    //Darstellung Einteilung Zeit-Achse:
    for(i=0; i<6; i++){
      tft.fillRect(187+40*i, 230, 3, 9, WHITE);
    }
    //Darstellung Einteilung Grad-Achse:
    for(i=0; i<4; i++){
      tft.fillRect(144, 98+33*i, 9, 3, WHITE);
    }
    //Darstellung Linienspitzen:
    for(i=0; i<5; i++){
      tft.fillRect(149-i, 72+2*i, 5+2*i, 2, WHITE);
      tft.fillRect(443+2*i, 225+i, 2, 13-2*i, WHITE);
    }
    tft.fillRect(150, 70, 3, 180, WHITE);   //vertikale Linie
    tft.fillRect(130, 230, 325, 3, WHITE);  //horizontale Linie
}

void under_construction(){
    tft.fillScreen(BLACK);    //Bildschirm schwarz
    tft.setCursor(75, 150);
    tft.setTextColor(WHITE);    tft.setTextSize(3);
    tft.println("under construction");
    tft.setCursor(35, 260);
}

//The interrupt0 handler: rotary1
void ClockChanged()
{
  delayMicroseconds(40);
  int clkValue = digitalRead(CLK);//Read the CLK pin level
  int dtValue = digitalRead(DT);//Read the DT pin level
  if (lastCLK != clkValue)
  {
    lastCLK = clkValue;
    Rotary1Counter += (clkValue != dtValue ? 1 : -1);//CLK and inconsistent DT + 1, otherwise - 1
    resolution = (abs(Rotary1Counter)%9)+1;
//    ms_time();
    #if(debug)
      Serial.print("count, resolution: ");//Vorsicht: Zeitliches Verhalten durch print-
      Serial.println(Rotary1Counter, resolution);  //Anweisungen anders!!
    #endif
  }
}



laborchoice_t getLaborChoiceFromTouchInput() {
	TSPoint p;
	int16_t x,y;

	p = ReadDisplayTouch();

	if(!p.z)
		return LC_None;

	x = p.x;
	y = p.y;

	Serial.print(x);
	Serial.print(" ");
	Serial.println(y);

    if(x > 114 && x < 364){
       if(y >120 && y < 160){
    	   return LC_Labor1;
       }
       if(y >190 && y < 230){
    	   return LC_Labor2;
       }
       if(y > 260 && y < 300){
    	   return LC_Labor3;
       }
    }

    x = 0; y = 0;
    return LC_None;
}


void loop() {
	enum state_t {S_WRITEDISPLAY, S_WAIT_TOUCHINPUT, S_EXECLAB, S_EXECLAB1_TIMECHOICE, S_EXECLAB1};
	static state_t state = S_WRITEDISPLAY;
	static Labor *currentLabor;
	laborchoice_t choice = LC_None;
	Labor::loopResult_t loopResult;
	TSPoint touchPoint;
	int Rotary1Switch = (digitalRead(SW) == LOW);

	if(currentLabor)
		loopResult = currentLabor->loop(Rotary1Counter, Rotary1Switch, 0, 0, touchPoint);


	switch(state) {
	case S_WRITEDISPLAY:
	    DisplayStartScreen();

	    state = S_EXECLAB1;
	    currentLabor = new LaborLogAmplifier(10);
	    break;

	    state = S_WAIT_TOUCHINPUT;
	    break;
	case S_WAIT_TOUCHINPUT:
		choice = getLaborChoiceFromTouchInput();
		Serial.println(choice);
		switch(choice){
		case LC_Labor1:
			currentLabor = new LaborLogAmplifier_TimeChoice();
			state = S_EXECLAB1_TIMECHOICE;
			break;
		case LC_Labor2:
			break;
		case LC_Labor3:
			break;
		default:
			break;
		}
		break;
	case S_EXECLAB1_TIMECHOICE:
		if(loopResult == Labor::LR_EXIT){
			delete currentLabor;
			currentLabor = 0;
			state = S_WRITEDISPLAY;
		}

		if(loopResult == Labor::LR_SWITCH){
			LaborLogAmplifier_TimeChoice *lab = reinterpret_cast<LaborLogAmplifier_TimeChoice *>(currentLabor);
			int16_t timems = lab->get_timems();
			delete currentLabor;
			currentLabor = new LaborLogAmplifier(timems);
			state = S_EXECLAB1;
		}
		break;

	case S_EXECLAB1:
		if(loopResult == Labor::LR_EXIT){
			delete currentLabor;
			currentLabor = 0;
			state = S_WRITEDISPLAY;
		}
		break;

	default:
		break;
	}
}

void loopx(){
    if(paint0){
        DisplayStartScreen();
    }
    touch();
    #if (debug)
        Serial.print("x1: ");
        Serial.println(x);
        Serial.print("y1: ");
        Serial.println(y);
    #endif

    if(x > 114 && x < 364){
       if(y >120 && y < 160){
           labor1 = 1;
           paint1 = 1;
           x = 0;
           y = 0;
       }
       if(y >190 && y < 230){
           labor2 = 1;
           paint2 = 1;
           tft.fillScreen(BLACK);
           x = 0;
           y = 0;
       }
       if(y > 260 && y < 300){
            labor3 = 1;
            paint3 = 1;
            tft.fillScreen(BLACK);
            x = 0;
            y = 0;
       }
    }

    #if (debug)
       Serial.print("Labor1: ");
       Serial.println(labor1);
       Serial.print("x2: ");
       Serial.println(x);
       Serial.print("y2: ");
       Serial.println(y);
    #endif


    //labor1:
    while(labor1){
       if(paint1){
          ppm(25, 38);
          tft.setCursor(25, 85);
          tft.setTextColor(WHITE);    tft.setTextSize(3);
          tft.println("Resolution time: ");
          //BNC1average = 0;
          counter = 0;
          home_button();
          paint1 = 0;
       }

       //choice of resolution per rotary:
       ms_time();

       touch();
          if((x > 25 && x < 113)&&(y > 250 && y < 293)){
            labor1 = 0;
            paint0 = 1;
            x = 0;
            y = 0;
          }

       if(digitalRead(SW) == LOW){
          z = 1;
          paint11 = 1;
          if(resolution >= 1 && resolution <=3)time_ms = 441;     //alle 10ms
          if(resolution >= 4 && resolution <=6)time_ms = 4410;    //alle 100ms
          if(resolution >= 7 && resolution <=9) time_ms = 44100; //alle 1000ms
       }
       while(z){
           if(paint11){
              x = 0;
              y = 0;
              #if (debug)
                 Serial.print("x: ");
                 Serial.println(x);
                 Serial.print("y: ");
                 Serial.println(y);
              #endif
              ppm(220, 38);

              //Anzeigelinien der dB-Werte:
              for(int z=0; z<47; z++){
                if(z!=4&&z!=14&&z!=24&&z!=34&&z!=44){
                     tft.fillRect(13+10*z, 155, 3, 10, WHITE);
                }
                else if(z==4||z==14||z==24) {
                    tft.fillRect(13+10*z, 155, 3, 10, BLUE1);
                }
                else if(z==34) {
                    tft.fillRect(13+10*z, 155, 3, 10, YELLOW);
                }
                else if(z==44) {
                    tft.fillRect(13+10*z, 155, 3, 10, RED);
                }
            }
            tft.fillRect(10, 120, 7, 30, BLUE1);   //ersten Balken blau anzeigen
            tft.setCursor(32, 175);
            tft.setTextColor(BLUE1);
            tft.setTextSize(2);
            tft.println("-30");

            tft.setCursor(132, 175);
            tft.setTextColor(BLUE1);    tft.setTextSize(2);
            tft.println("-20");

            tft.setCursor(232, 175);
            tft.setTextColor(BLUE1);    tft.setTextSize(2);
            tft.println("-10");

            tft.setCursor(349, 175);
            tft.setTextColor(YELLOW);    tft.setTextSize(2);
            tft.println("0");

            tft.setCursor(432, 175);
            tft.setTextColor(RED);    tft.setTextSize(2);
            tft.println("+10");

            tft.setCursor(13, 94);
            tft.setTextColor(WHITE);    tft.setTextSize(2);
            tft.println("dB");

            home_button();

            tft.setCursor(350, 250);
            tft.setTextColor(YELLOW);    tft.setTextSize(3);
            if(resolution >= 1 && resolution <=3) tft.println("10ms");
            if(resolution >= 4 && resolution <=6) tft.println("100ms");
            if(resolution >= 7 && resolution <=9) tft.println("1000ms");

            paint11 = 0;
          }

       //Drücken des home button:
       touch();
       #if (debug)
          Serial.print("x: ");
          Serial.println(x);
          Serial.print("y: ");
          Serial.println(y);
       #endif
       if((x > 25 && x < 113)&&(y > 250 && y < 293)){
          z = 0;
          labor1 = 0;
          paint0 = 1;
          x = 0;
          y = 0;
          #if (debug)
              Serial.print("paint0: ");
              Serial.println(paint0);
          #endif
       }

          //Hier: Test Timing Messwerte Teil a)
          //#if(debug)
            if(counter == 0){
            current = micros();
            }
          //#endif

            //Usum = Usum + pow(BNC1Pin * 3.3/1023, 2);
            Usum = Usum + BNC1Pin;
            #if(debug)
              Serial.print("Usum: ");
              Serial.println(Usum);
            #endif
            zaehler++;
//            current = micros();
//            Serial.print("time: ");
//            Serial.println(current - last);
//            Serial.print("zaehler: ");
//            Serial.println(zaehler);
//            Serial.print("counter: ");
//            Serial.println(counter);
            if(counter >= time_ms){
              last = micros();
              //Ueff = sqrt(Usum / zaehler);
              Ueff = Usum / zaehler;
              //zmax = round(20*(log(Ueff/0.775)/log(10)));
              zmax = ((BNC1Pin-744)*20/185.69 +2.21);
            //Hier: Test Timing Messwerte Teil b)
            //#if(debug)
                Serial.print("counter: ");
                Serial.println(counter);
                Serial.print("zaehler: ");
                Serial.println(zaehler);
//                Serial.print("Ueff: ");
//                Serial.println(Ueff);
//                Serial.print("zmax: ");
//                Serial.println(zmax);
                Serial.print("time: ");
                Serial.println(last - current);
            //#endif
              counter = 0;
              zaehler = 0;
              Usum = 0;
              PPM_Anzeige();
              zmax_old = zmax;
        #if(debug)
                current = micros();
        #endif
        }
      }
  } //while(labor1) end


  //Labor 2:
  while(labor2){
  //  int zaehler;
    int MP[120];
    if (initTemp) {
      for(i=0; i<120; i++) {
        MP[i] = -100;
      }
      initTemp = false;
    }
    if(paint2){
       pt1000(25, 38);
       temp_akt(25, 73);
       temp_grafik();
       BNC1average = 0;
       home_button();
       Temp = 0;
       counter = 0;
       paint2 = 0;
       Temp_old = 0;
       zaehler = 0;
       zaehler2 = 0;
       initTemp = true;
    }
    //Messwert in °C ausgeben:

    BNC1Pin_sum = BNC1Pin_sum + BNC1Pin;
    zaehler++;                            //Die Werte für counter und zaehler unterscheiden sich!!
     if(counter >= 4410){                  //4410 => alle 100ms
     //if(counter >= 22050){
        BNC1average = BNC1Pin_sum / (zaehler);
        //BNC1average = BNC1Pin_sum / (counter + 1);
        #if(debug)
          last = micros();
          Serial.print("time: ");
          Serial.println(last - current);
          Serial.print("BNC1average: ");
          Serial.println(BNC1average);
          Serial.print("zaehler: ");
          Serial.println(zaehler);
          Serial.print("BNC1Pin_sum: ");
          Serial.println(BNC1Pin_sum);
          Serial.print("counter: ");
          Serial.println(counter);
        #endif
        float tempvolt = BNC1average * 3.3 / 1023.;
      //  float tempmess = tempvolt/50+80e-3;

        float tempmess = tempvolt/60+59e-3;
        //float tempmess = tempvolt/57.1+59e-3;
        //tempmess = 3.5 * x/(47k + x)
        float rpt1000 = tempmess * 47e3 / (3.5-tempmess);
        Temp = (rpt1000 - 1000)/3.91;   //T = (R(T)-R0)/(R0*A)
        //Temp = -3.91e-3/(2*0.58e-6)-sq((pow(3.91e-3/(2*0.58e-6),2)-(1000-rpt1000)/0.58e-3));
        counter = 0;
        BNC1Pin_sum = 0;
        zaehler = 0;
        #if(debug)
          current = micros();
        #endif

        if(!labor21){
          if(Temp != Temp_old){
            tft.fillRect(25, 108, 130, 30, BLACK);
            tft.setCursor(25, 108);
            tft.setTextColor(WHITE);    tft.setTextSize(3);
            tft.print(Temp,1);
            tft.println("\xF7" "C");
            Temp_old = Temp;
          }
        }

        if(labor21){
          if(paint21){
            pt1000(25, 25);
            back_button();
            KAchsen(grafik);
            Temp_old = -120;  //damit im Grafikmodus sofort eine Temp angezeigt wird!
            paint21 = 0;
            if(grafik == 60) korr = 130;
            else korr = 0;
            paint22 = 1;
            i = 0;
          }
          if(paint22){
            if(aktualisieren){
              if(Temp != Temp_old){
                tft.fillRect(350, 38, 130, 30, BLACK);
                tft.setCursor(350, 38);
                if(korr == 0){
                  if(Temp <= 0 || Temp >= 40){
                    tft.setTextColor(RED);
                  }
                  else tft.setTextColor(YELLOW);
                }
                else{
                  if(Temp <= 40 || Temp >= 80){
                    tft.setTextColor(RED);
                  }
                  else tft.setTextColor(YELLOW);
                }
                tft.setTextSize(3);
                tft.print(Temp,1);
                tft.println("\xF7" "C");
                Temp_old = Temp;
              }
            }
            currentcolor = YELLOW;
            //Umrechnung Messpunkte auf Display:
            dig = int(Temp / 0.303)-korr;
            #if(debug)
              Serial.print("BNC1average: ");
              Serial.println(BNC1average);
              Serial.print("Temp: ");
              Serial.println(Temp);
              Serial.print("dig: ");
              Serial.println(dig);
            #endif

            if(aktualisieren){
              if(zaehler2 <120){
                MP[zaehler2] = (230 - dig); //230: y-Wert der Zeit-Achse
                if (MP[zaehler2] != -100){
                    if(MP[zaehler2] >= 228){
                      currentcolor = RED;
                      MP[zaehler2] = 228;
                    }
                    else if(MP[zaehler2] <= 98){
                      currentcolor = RED;
                      MP[zaehler2] = 98;
                    }
                tft.fillRect(153+2*zaehler2, MP[zaehler2], 2, 2, currentcolor);
                }
                zaehler2++;
              }
              else{
                //Messpunkte loeschen:
                MP[119] = (230 - dig);
                currentcolor = BLACK;
                for (i=0;i<119;i++){
                  tft.fillRect(153+2*i, MP[i], 2, 2, currentcolor);
                  MP[i] = MP[i+1];  //Messpunkte verschieben im Array
                  //verschobene Messpunkte zeichnen:
                  if (MP[i] != -100){
                    if(MP[i] >= 228){
                      currentcolor = RED;
                      MP[i] = 228;
                    }
                    else if(MP[i] <= 98){
                      currentcolor = RED;
                      MP[i] = 98;
                    }
                    else currentcolor = YELLOW;
                    tft.fillRect(153+2*i, MP[i], 2, 2, currentcolor);
                    currentcolor = BLACK;
                    tft.fillRect(153+2*(i+1), MP[i], 2, 2, currentcolor);
                  }
                }
              }
            }
          }
        }
      }


      if(digitalRead(SW) == LOW){
        aktualisieren = false;
      }

      touch();
      #if (debug)
         Serial.print("x: ");
         Serial.println(x);
         Serial.print("y: ");
         Serial.println(y);
      #endif
      //press 0 bis 40°C button:
      if((x > 170 && x < 405)&&(y > 140 && y < 193)){
         labor21 = 1;
         paint21 = 1;
         grafik = 20;
         x = 0;
         y = 0;
      }
      //press 40 bis 80°C button:
      if((x > 170 && x < 405)&&(y > 200 && y < 253)){
         labor21 = 1;
         paint21 = 1;
         grafik = 60;
         x = 0;
         y = 0;

      }
      //press home/back button:
      if((x > 25 && x < 113)&&(y > 250 && y < 293)){
         aktualisieren = true;
         x = 0;
         y = 0;
         //home:
         if(paint22 == 1){
           labor21 = 0;
           paint2 = 1;
           paint22 = 0;
         }
         //back:
         else{
           paint0 = 1;
           labor2 = 0;
           labor21 = 0;
           zaehler2 = 0;
           #if (debug)
              Serial.print("paint0: ");
              Serial.println(paint0);
           #endif
         }
      }
} //while labor2 end


    //Labor 3:
    while(labor3){
      if(paint3){
         under_construction();
         home_button();
         paint3 = 0;
      }
      touch();
      #if (debug)
          Serial.print("x: ");
          Serial.println(x);
          Serial.print("y: ");
          Serial.println(y);
      #endif
      if((x > 25 && x < 113)&&(y > 250 && y < 293)){
          labor3 = 0;
          paint0 = 1;
          x = 0;
          y = 0;
          #if (debug)
              Serial.print("paint0: ");
              Serial.println(paint0);
          #endif
      }
    }
}

