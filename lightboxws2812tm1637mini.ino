//Created by Giuseppe Tamanini 22/01/2022 with license cc-by-sa 4.0

#include <TM1637.h>
#include <DS3231.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#define PIN 2 // pin LEDs WS2812
#define NUMPIXELS 83 // Number of LEDs WS2812

DS3231 clock1;
bool h12Flag;
bool pmFlag;

// Instantiation and pins configurations
// Pin 2 - > DIO
// Pin 0 - > CLK
TM1637 tm1637(4, 3);
boolean dp;
int hh; // hour
int mm;  // minute
int ss;  // second
int oldss; // old second
String Sore; // String variables used for display on the TM1637
char ore[5];
String Sscena;
char Cscena[5];
int buttonPin = 7; // button pin
unsigned long timepressedButton; // button reading variable
unsigned long myTime;
unsigned long myTime1;
unsigned long previousMillis;
boolean buttonState;
boolean oldbuttonState;
boolean cambiato;
boolean modifica = false; // true if the program is in time edit mode
int sezioni[10][2] = {{1, 10},{11, 18},{19, 23},{24,25},{26,27},{28,36},{37,44},{45,56},{57,66},{67,80}}; // 83 LED divided in blocks
byte scene[4][10][3] = {
                        {{0,   0,  0},{  0,   0, 0},{64,  32,  16},{  0,   0,   0},{ 0,  0, 0},{ 0,  0, 0},{  0,   0,   0},{127,  64,  32},{127,  64,  32},{170, 255, 0}},
                        {{0, 255,  0},{128, 192, 0},{ 0, 128, 128},{192, 192,   0},{ 0,  0, 0},{64, 64, 0},{  0, 128, 128},{  0, 128, 127},{  0, 128, 128},{170, 255, 0}},
                        {{0, 255,  0},{127, 192, 0},{ 0, 255, 255},{192, 192,   0},{ 0,  0, 0},{64, 64, 0},{127,   0, 128},{  0,   0, 255},{  0,   0, 255},{255,   0, 0}},
                        {{0,   0,128},{  0,   0, 0},{ 0,  64,  64},{  0,   0,   0},{64, 96, 0},{ 0,  0, 0},{  0,   0,   0},{  0,   0, 128},{  0,   0, 128},{128, 192, 0}}
                      }; // defined colors sections in scenes
int R; // rosso
int G; // verde
int B; // blu
int sezione; // section number
int scena; // scene number
int oldscena = 99; // previous scene
int scenadissolve; // scene fade
int oldscenadissolve = 99; // previous scene fade
int n; // scene step number while fade
int fadeTime = 50; // milliseconds of step 
boolean dissolve; // true when fade
boolean fixed; // true when fixed scene
int sceneTime = 5000; // millisecond fixed scene
unsigned long stepdissolveTime; // used to measure the duration of the fade
unsigned long fixedTime; // it is used to measure the duration of the fixed scene
float stepvalue[10][3]; // matrix of color variation steps during fading

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  tm1637.init();
  tm1637.setBrightness(5);
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)  
  pixels.clear(); // Set all pixel colors to 'off'
  previousMillis = millis();
  for (int i = 0; i < 10; i++) {
    colora(i, scene[scena][i][0], scene[scena][i][1], scene[scena][i][2]);
  }
}

void loop() {
  hh = clock1.getHour(h12Flag, pmFlag);
  mm = clock1.getMinute();
  ss = clock1.getSecond();
  if (ss == oldss) { // Every second shows the time on the TM1637 display
    if (millis() - previousMillis < 500 && dp == false) {
      sprintf(ore, "%02d%02d", hh, mm);
      Sore = ore;
      tm1637.display(Sore); // Display the time
      Serial.println(Sore);
      dp = true;
    }
    if (millis() - previousMillis > 500 && dp == true) { // Every half second makes the colon flash
      sprintf(ore, "%02d.%02d", hh, mm);
      Sore = ore;
      tm1637.display(Sore);
      Serial.println(":");
      dp = false;
    }
  } else {
    oldss = ss;
    previousMillis = millis(); 
  }
  buttonState = digitalRead(buttonPin);
  if (buttonState && oldbuttonState == false) { // se è premuto il pulsante avvia il conto del tempo di premuta
    oldbuttonState = true;
    timepressedButton = millis();
    Serial.print("i");
    Serial.println(timepressedButton);
  }
  if (buttonState == LOW && oldbuttonState) { // quando il pulsante viene rilasciato calcola il tempo di premuta
    timepressedButton = millis() - timepressedButton;
    oldbuttonState = false;
    //Serial.print("p");
    //Serial.println(timepressedButton);
  }
  if (buttonState == false && oldbuttonState == false && timepressedButton > 3000) { // If the button has been pressed for 3 seconds execute moficaora function
    //Serial.print("pm");
    //Serial.println(timepressedButton);
    modifica = true;
    myTime1 = millis();
    //Serial.println("modifica ora");
    modificaora();
    timepressedButton = 0;
  }
  if (buttonState == false && oldbuttonState == false && (timepressedButton > 100 && timepressedButton < 3000)) { // If the button has been pressed for 0.1 second increase the value increase the value scena
    //Serial.print("ps");
    //Serial.println(timepressedButton);
    timepressedButton = 0;
    scena = scena + 1;
    if (scena == 5) scena = 0;
    Serial.print("Scena:");
    Serial.println(scena);
    if (scena != oldscena) {
      oldscena = scena;
      if (scena < 4) {
        for (int i = 0; i < 10; i++) {
          colora(i, scene[scena][i][0], scene[scena][i][1], scene[scena][i][2]);
        }
      }
    }
    sprintf(Cscena, "   %d", scena);
    Sscena = Cscena;
    tm1637.display(Sscena); // Display the time
    delay(500);
  }
  if (scena == 4) fade();
}

void modificaora() {
  while (modifica) {
    sprintf(ore, "%02d.%02d", hh, mm);
    Sore = ore;
    tm1637.display(Sore); // Display the time
    buttonState = digitalRead(buttonPin);
    if (buttonState && !oldbuttonState) {
      myTime = millis();
      oldbuttonState = true;
      cambiato = true;
    } else if (buttonState == false && oldbuttonState) {
      oldbuttonState = false;  // sets the old button state to false
      cambiato = false; // the state of the button action is false 
      myTime1 = millis();
    }
    if (buttonState && oldbuttonState && millis() - myTime > 100) {
      myTime = millis();
      //Serial.println("funziona");
      oldbuttonState = true;
      mm = mm + 1;
      if (mm == 60) {
        mm = 0;
        hh = hh + 1;
        if (hh == 24) hh = 0;
      }
    }
    if (buttonState == false && oldbuttonState == false && cambiato == false && millis() - myTime1 > 3000) {
      //Serial.println("salvato");
      clock1.setHour(hh);
      clock1.setMinute(mm);
      clock1.setSecond(ss);
      modifica = false;
    }
  }
}

void colora(int sezione, int R, int G, int B) { // Program the colors of the WS2812 LEDs
  for (int i = sezioni[sezione][0] - 1; i < sezioni[sezione][1]; i++) {
    pixels.setPixelColor(i, pixels.Color(R, G, B));
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
}

void fade() { // Function that fades between scenes
  if (scenadissolve != oldscenadissolve && dissolve == false) {
    oldscenadissolve = scenadissolve;
    dissolve = true;
    for (int i = 0; i < 10; i++) { // i è il numero della sezione
      for (int k = 0; k < 3; k++) { // k è il numero del colore 0 - R, 1 - G, 2 - B
        if (scenadissolve < 3) {
          stepvalue[i][k] = ((float)scene[scenadissolve + 1][i][k] - (float)scene[scenadissolve][i][k]) / 100.00; // fa la differenza fra il valore del canale i della scena successiva (j + 1) e quella corrente (j) / 100
        } else {
          stepvalue[i][k] = ((float)scene[0][i][k] - (float)scene[scenadissolve][i][k]) / 100.00;
        }
      }
    }
    stepdissolveTime = millis();
  }
  if (dissolve) {    
    if (n < 100) {
      if (millis() - stepdissolveTime > fadeTime) {     
        for (int i = 0; i < 10; i++) {
          colora(i, scene[scenadissolve][i][0] + stepvalue[i][0] * n, scene[scenadissolve][i][1] + stepvalue[i][1] * n, scene[scenadissolve][i][2] + stepvalue[i][2] * n);
        }
        n = n + 1;
        stepdissolveTime = millis();
      }
    } else {
      n = 0;
      dissolve = false;
      fixed = true;
      fixedTime = millis();
    }
  }
  if (fixed && millis() - fixedTime > sceneTime) {
    fixed = false;
    scenadissolve = scenadissolve + 1;
    if (scenadissolve == 4) scenadissolve = 0;
  }
}
