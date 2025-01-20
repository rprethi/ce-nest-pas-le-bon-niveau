#define CHAN_KEYONE 0
#define CHAN_KEYTWO 1
#define CHAN_KEYTHREE 2

#include <VL53L0X.h>
VL53L0X myTOF;

#include "Unit_Encoder.h"
Unit_Encoder myEncoder;

int myEncoderPreviousRotation;

#include <M5Atom.h>

CRGB pixel;

unsigned long monChronoMessages;

#include <MicroOscSlip.h>
MicroOscSlip<128> monOsc(&Serial);

#include <M5_PbHub.h>
M5_PbHub myPbHub;

bool monEtatKey1;
bool monEtatKey2;
bool monEtatKey3;

int maLectureKeyPrecedenteOne;
int maLectureKeyPrecedenteTwo;
int maLectureKeyPrecedenteThree;
int etatPlay;

void setup() {
  // put your setup code here, to run once:
  M5.begin(false, false, false);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(&pixel, 1);  // Ajouter le pixel du M5Atom à FastLED
  Serial.begin(115200);

  unsigned long chronoDepart = millis();
  while (millis() - chronoDepart < 5000) {
    pixel = CRGB(255, 255, 255);
    FastLED.show();
    delay(100);

    pixel = CRGB(0, 0, 0);
    FastLED.show();
    delay(100);
  }

  pixel = CRGB(0, 0, 0);
  FastLED.show();

  Wire.begin();
  myPbHub.begin();
  myPbHub.setPixelCount(CHAN_KEYONE, 1);

  myTOF.init();
  myTOF.setTimeout(500);
  myTOF.startContinuous();
  myEncoder.begin();  // Démarrer la connexion avec l'encodeur
}

void maReceptionMessageOsc(MicroOscMessage& oscMessage) {

  if (oscMessage.checkOscAddress("/master/vu")) {
    float vu = oscMessage.nextAsFloat();
    int niveau = floor(vu * 255.0);
    pixel = CRGB(niveau, niveau, niveau);
    FastLED.show();
  }
}


void loop() {
  int monMin = 0;
  int monMax = 256;

  int monRouge = random(monMin, monMax);
  int monVert = random(monMin, monMax);
  int monBleu = random(monMin, monMax);
  // put your main code here, to run repeatedly:
  M5.update();

  uint16_t value = myTOF.readRangeContinuousMillimeters();
  int error = myTOF.timeoutOccurred();
  monOsc.onOscMessageReceived(maReceptionMessageOsc);

  int encoderRotation = myEncoder.getEncoderValue();

  int encoderRotationChange = encoderRotation - myEncoderPreviousRotation;
  myEncoderPreviousRotation = encoderRotation;

  int encoderButton = myEncoder.getButtonStatus();
  uint32_t myColorOn = monRouge;
  uint32_t myColorOff = 0x000000;

  // À CHAQUE 20 MS I.E. 50x PAR SECONDE
  if (millis() - monChronoMessages >= 20) {
    monChronoMessages = millis();

    // ENCODER
    if (encoderButton == 0) {
      myEncoder.setLEDColor(2, myColorOn);
      myEncoder.setLEDColor(1, myColorOn);
    } else {
      if (encoderRotationChange > 0) {
        myEncoder.setLEDColor(1, myColorOn);
        myEncoder.setLEDColor(2, myColorOff);
      } else if (encoderRotationChange < 0) {
        myEncoder.setLEDColor(1, myColorOff);
        myEncoder.setLEDColor(2, myColorOn);
      } else {
        myEncoder.setLEDColor(1, myColorOff);
        myEncoder.setLEDColor(2, myColorOff);
      }
    }

    //KEY

    int maLectureKeyOne = myPbHub.digitalRead(CHAN_KEYONE);  //digitalRead va de 0 à 1
    int maLectureKeyTwo = myPbHub.digitalRead(CHAN_KEYTWO);  //digitalRead va de 0 à 1
    int maLectureKeyThree = myPbHub.digitalRead(CHAN_KEYTHREE);  //digitalRead va de 0 à 1


    // LECTURE DE MA KEY
    if (maLectureKeyPrecedenteOne != maLectureKeyOne) {
      if (maLectureKeyOne == 0) {
        monEtatKey1 = !monEtatKey1;  // même chose que if (monEtatKey == false) { monEtatKey == true} else { monEtatKey == false}
      }
      if (monEtatKey1) {
        monOsc.sendInt("/key1", 0);
        myPbHub.setPixelColor(0, 0, 0, 0, 0);
      } else {
        monOsc.sendInt("/key1", 1);
        myPbHub.setPixelColor(0, 0, 255, 255, 255);
      }
      maLectureKeyPrecedenteOne = maLectureKeyOne;
    }

    // LECTURE DE MA KEY
    if (maLectureKeyPrecedenteTwo != maLectureKeyTwo) {
      if (maLectureKeyTwo == 0) {
        monEtatKey2 = !monEtatKey2;  // même chose que if (monEtatKey == false) { monEtatKey == true} else { monEtatKey == false}
      }
      if (monEtatKey2) {
        monOsc.sendInt("/key2", 0);
        myPbHub.setPixelColor(1, 0, 0, 0, 0);
      } else {
        monOsc.sendInt("/key2", 2);
        myPbHub.setPixelColor(1, 0, 255, 255, 255);
      }
      maLectureKeyPrecedenteTwo = maLectureKeyTwo;
    }

    if (maLectureKeyPrecedenteThree != maLectureKeyThree) {
      if (maLectureKeyThree == 0) {
        monEtatKey3 = !monEtatKey3;  // même chose que if (monEtatKey == false) { monEtatKey == true} else { monEtatKey == false}
      }
      if (monEtatKey3) {
        monOsc.sendInt("/key3", 0);
        myPbHub.setPixelColor(2, 0, 0, 0, 0);
      } else {
        monOsc.sendInt("/key3", 3);
        myPbHub.setPixelColor(2, 0, 255, 255, 255);
      }
      maLectureKeyPrecedenteThree = maLectureKeyThree;
    }
  }



  monOsc.sendInt("/encoder", encoderRotation);
  monOsc.sendInt("/encoderBtn", encoderButton);
  monOsc.sendInt("/tof", value);
}
