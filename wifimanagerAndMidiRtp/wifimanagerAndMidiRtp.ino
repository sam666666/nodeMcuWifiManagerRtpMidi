#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESPRotary.h>
#include <ESP8266mDNS.h>
#define SerialMon Serial
#define APPLEMIDI_DEBUG SerialMon
#include <AppleMIDI.h>

int8_t isConnected = 0;

//encoder
#define ROTARY_PIN1  14
#define ROTARY_PIN2 12
ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2);
int oldValue = 0;

//midi
APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

//wifi stuff
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager


void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  // put your setup code here, to run once:
  Serial.begin(115200);

  //WiFi.mode(WIFI_STA); // it is a good practice to make sure your code sets wifi mode how you want it.

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  //reset settings - wipe credentials for testing
  //wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("AutoConnect", "AutoConnect"); // password protected ap

  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();//either make that the flash btn resets the wifi settings or this resets the settings
  }
  else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }

  //put setupshit down here
  midiSetup();
  delay(50);
  r.setChangedHandler(rotate);
}

void loop() {
  // Listen to incoming notes
  MIDI.read();
  r.loop();
}

void rotate(ESPRotary& r) {
  byte note = 45;
  byte velocity = 55;
  byte channel = 1;
  int posValue = r.getPosition() / 4;
  if (posValue != oldValue) {
    oldValue = posValue;
    Serial.println(oldValue);
    MIDI.sendNoteOn(note, oldValue, channel);
  }
}

void midiSetup() {
  MIDI.begin();

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected++;
    DBG(F("Connected to session"), ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected--;
    DBG(F("Disconnected"), ssrc);
  });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOn"), note);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOff"), note);
  });
}
