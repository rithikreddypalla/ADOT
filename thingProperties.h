bool myVar;
// Secure version: Remove hardcoded credentials. Load from a separate, non-tracked config file or use build flags.
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// TODO: Store these in a separate config file (not committed to version control) or use build flags/macros
#ifndef DEVICE_LOGIN_NAME
#define DEVICE_LOGIN_NAME "YOUR_DEVICE_LOGIN_NAME" // Set via build flag or config
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID" // Set via build flag or config
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "YOUR_WIFI_PASSWORD" // Set via build flag or config
#endif

#ifndef DEVICE_KEY
#define DEVICE_KEY "YOUR_DEVICE_KEY" // Set via build flag or config
#endif

void onPasswordChange();
//void onPassChange();
//void onMyVarChange();

String password;
int pass;
bool myVar;

void initProperties(){
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(password, READWRITE, ON_CHANGE, onPasswordChange);
  //ArduinoCloud.addProperty(pass, READWRITE, ON_CHANGE, onPassChange);
  //ArduinoCloud.addProperty(myVar, READWRITE, ON_CHANGE, onMyVarChange);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(WIFI_SSID, WIFI_PASS);