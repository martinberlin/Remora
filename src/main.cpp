// Remora Firmware WiFi Configuration over Bluetooth serial
#include <WiFi.h>
// Config.h and Animate.h need configuration
#include <Config.h>  // WiFi credentials, mDNS Domain - Not used for now with Blue config
#include <Animate.h> // PixelCount, PixelPin
#include <ESPmDNS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <nvs.h>
#include <nvs_flash.h>
// Includes for Bluetooth Serial
#include "BluetoothSerial.h"
#include <Preferences.h>
#include <ArduinoJson.h>

// Debug mode prints to serial
bool debugMode = true;
TimerHandle_t wifiReconnectTimer;
// Animation handling class
Animate animate;

struct config {
  char chipId[20];
  int udpPort = 49161; // 49161 Default Orca UDP Port
} internalConfig;
// WiFi credentials storage
Preferences preferences; 

char apName[] = "ESP-xxxxxxxxxxxx_49161";
bool usePrimAP = true;
/** Flag if stored AP credentials are available */
bool hasCredentials = false;
/** Connection status */
volatile bool isConnected = false;
bool connStatusChanged = false;

String ipAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]);
}
/**
 * Generic message printer. Modify this if you want to send this messages elsewhere (Display)
 */
void printMessage(String message, bool newline = true)
{
  if (debugMode) {
    if (newline) {
      Serial.println(message);
    } else {
      Serial.print(message);
    }
   }
}
/**
 * Create unique device name from MAC address
 **/
void createName() {
	uint8_t baseMac[6];
	// Get MAC address for WiFi station
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	// Write unique name into apName
	sprintf(apName, "ESP-%02X%02X%02X%02X%02X%02X_%d", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5], internalConfig.udpPort);
}

/** SSIDs of local WiFi networks */
String ssidPrim;
String ssidSec;
/** Password for local WiFi network */
String pwPrim;
String pwSec;

// SerialBT class
BluetoothSerial SerialBT;

/** Buffer for JSON string */
StaticJsonDocument<200> jsonBuffer;

/**
 * initBTSerial
 * Initialize Bluetooth Serial
 * Start BLE server and service advertising
 * @return <code>bool</code>
 * 			true if success
 * 			false if error occured
 */
bool initBTSerial() {
		if (!SerialBT.begin(apName)) {
			Serial.println("Failed to start BTSerial");
			return false;
		}
		Serial.println("BTSerial active. Device name: " + String(apName));
		return true;
}

/** Callback for receiving IP address from AP */
void gotIP(system_event_id_t event) {
	isConnected = true;
	connStatusChanged = true;

	MDNS.begin(apName);
	delay(100);
    MDNS.addService("http", "tcp", 80);
    printMessage(String(apName)+".local mDns started");

    animate.startUdpListener(WiFi.localIP(), internalConfig.udpPort);
}

/**
	 scanWiFi
	 Scans for available networks 
	 and decides if a switch between
	 allowed networks makes sense

	 @return <code>bool</code>
	        True if at least one allowed network was found
*/
bool scanWiFi() {
	/** RSSI for primary network */
	int8_t rssiPrim = -1;
	/** RSSI for secondary network */
	int8_t rssiSec = -1;
	/** Result of this function */
	bool result = false;

	Serial.println("Start scanning for networks");

	WiFi.disconnect(true);
	WiFi.enableSTA(true);
	WiFi.mode(WIFI_STA);

	// Scan for AP
	int apNum = WiFi.scanNetworks(false,true,false,1000);
	if (apNum == 0) {
		Serial.println("Found no networks?????");
		return false;
	}
	
	byte foundAP = 0;
	bool foundPrim = false;

	for (int index=0; index<apNum; index++) {
		String ssid = WiFi.SSID(index);
		Serial.println("Found AP: " + ssid + " RSSI: " + WiFi.RSSI(index));
		if (!strcmp((const char*) &ssid[0], (const char*) &ssidPrim[0])) {
			Serial.println("Found primary AP");
			foundAP++;
			foundPrim = true;
			rssiPrim = WiFi.RSSI(index);
		}
		if (!strcmp((const char*) &ssid[0], (const char*) &ssidSec[0])) {
			Serial.println("Found secondary AP");
			foundAP++;
			rssiSec = WiFi.RSSI(index);
		}
	}

	switch (foundAP) {
		case 0:
			result = false;
			break;
		case 1:
			if (foundPrim) {
				usePrimAP = true;
			} else {
				usePrimAP = false;
			}
			result = true;
			break;
		default:
			Serial.printf("RSSI Prim: %d Sec: %d\n", rssiPrim, rssiSec);
			if (rssiPrim > rssiSec) {
				usePrimAP = true; // RSSI of primary network is better
			} else {
				usePrimAP = false; // RSSI of secondary network is better
			}
			result = true;
			break;
	}
	return result;
}

/** Callback for connection loss */
void lostCon(system_event_id_t event) {
	isConnected = false;
	connStatusChanged = true;

    Serial.println("WiFi lost connection, trying to connect again");
	//ESP.restart();
	WiFi.begin(ssidPrim.c_str(), pwPrim.c_str());
}

/**
 * Start connection to AP
 */
void connectWiFi() {
	// Setup callback function for successful connection
	WiFi.onEvent(gotIP, SYSTEM_EVENT_STA_GOT_IP);
	// Setup callback function for lost connection
	WiFi.onEvent(lostCon, SYSTEM_EVENT_STA_DISCONNECTED);

	Serial.println();
	Serial.print("Start connection to ");
	if (usePrimAP) {
		Serial.println(ssidPrim);
		WiFi.begin(ssidPrim.c_str(), pwPrim.c_str());
	} else {
		Serial.println(ssidSec);
		WiFi.begin(ssidSec.c_str(), pwSec.c_str());
	}
}

/**
 * readBTSerial
 * read all data from BTSerial receive buffer
 * parse data for valid WiFi credentials
 */
void readBTSerial() {
	uint64_t startTimeOut = millis();
	String receivedData;
	int msgSize = 0;
	// Read RX buffer into String
	while (SerialBT.available() != 0) {
		receivedData += (char)SerialBT.read();
		msgSize++;
		// Check for timeout condition
		if ((millis()-startTimeOut) >= 5000) break;
	}
	SerialBT.flush();
	Serial.println("Received message " + receivedData + " over Bluetooth");

	// decode the message | No need to do this, since we receive it as string already
	if (receivedData[0] != '{') {
		int keyIndex = 0;
		for (int index = 0; index < receivedData.length(); index ++) {
			receivedData[index] = (char) receivedData[index] ^ (char) apName[keyIndex];
			keyIndex++;
			if (keyIndex >= strlen(apName)) keyIndex = 0;
		}
		Serial.println("Decoded message: " + receivedData); 
	}
	
	/** Json object for incoming data */
	auto error = deserializeJson(jsonBuffer, receivedData);
	if (!error)
	{
		if (jsonBuffer.containsKey("ssidPrim") &&
			jsonBuffer.containsKey("pwPrim") &&
			jsonBuffer.containsKey("ssidSec") &&
			jsonBuffer.containsKey("pwSec"))
		{
			ssidPrim = jsonBuffer["ssidPrim"].as<String>();
			pwPrim = jsonBuffer["pwPrim"].as<String>();
			ssidSec = jsonBuffer["ssidSec"].as<String>();
			pwSec = jsonBuffer["pwSec"].as<String>();

			Preferences preferences;
			preferences.begin("WiFiCred", false);
			preferences.putString("ssidPrim", ssidPrim);
			preferences.putString("ssidSec", ssidSec);
			preferences.putString("pwPrim", pwPrim);
			preferences.putString("pwSec", pwSec);
			preferences.putBool("valid", true);
			preferences.end();

			Serial.println("Received over bluetooth:");
			Serial.println("primary SSID: "+ssidPrim+" password: "+pwPrim);
			Serial.println("secondary SSID: "+ssidSec+" password: "+pwSec);
			connStatusChanged = true;
			hasCredentials = true;
			
			if (!scanWiFi()) {
				Serial.println("Could not find any AP");
			} else {
				// If AP was found, start connection
				connectWiFi();
			}

		}
		else if (jsonBuffer.containsKey("erase"))
		{ // {"erase":"true"}
			Serial.println("Received erase command");
			Preferences preferences;
			preferences.begin("WiFiCred", false);
			preferences.clear();
			preferences.end();
			connStatusChanged = true;
			hasCredentials = false;
			ssidPrim = "";
			pwPrim = "";
			ssidSec = "";
			pwSec = "";

			int err;
			err=nvs_flash_init();
			Serial.println("nvs_flash_init: " + err);
			err=nvs_flash_erase();
			Serial.println("nvs_flash_erase: " + err);
		}
		else if (jsonBuffer.containsKey("getip"))
		{ // {"getip":"true"}
			Serial.println("getip");
			String wifiCredentials;
			jsonBuffer.clear();
			jsonBuffer["status"] = 1;
			jsonBuffer["ip"] = ipAddress2String(WiFi.localIP());
			jsonBuffer["port"] = internalConfig.udpPort;
			serializeJson(jsonBuffer, wifiCredentials);
			Serial.println(wifiCredentials);
			Serial.println();
			if (SerialBT.available()) {
				SerialBT.print(wifiCredentials);
				SerialBT.flush();
			} else {
				Serial.println("Cannot send IP request: Serial Bluetooth is not available");
			}
			
		}
		else if (jsonBuffer.containsKey("read"))
		{ // {"read":"true"}
			Serial.println("BTSerial read request");
			String wifiCredentials;
			jsonBuffer.clear();

			/** Json object for outgoing data */
			jsonBuffer.clear();
			jsonBuffer["ssidPrim"] = ssidPrim;
			jsonBuffer["pwPrim"] = pwPrim;
			jsonBuffer["ssidSec"] = ssidSec;
			jsonBuffer["pwSec"] = pwSec;
			// Convert JSON object into a string
			serializeJson(jsonBuffer, wifiCredentials);

			// encode the data
			int keyIndex = 0;
			Serial.println("Stored settings: " + wifiCredentials);
			for (int index = 0; index < wifiCredentials.length(); index ++) {
				wifiCredentials[index] = (char) wifiCredentials[index] ^ (char) apName[keyIndex];
				keyIndex++;
				if (keyIndex >= strlen(apName)) keyIndex = 0;
			}
			Serial.println("Stored encrypted: " + wifiCredentials);

			delay(2000);
			SerialBT.print(wifiCredentials);
			SerialBT.flush();
		} else if (jsonBuffer.containsKey("reset")) {
			WiFi.disconnect();
			esp_restart();
		}
	} else {
		Serial.println("Received invalid JSON");
	}
	jsonBuffer.clear();
}

void setup()
{ 
	Serial.begin(115200);
	createName();

	// Start Bluetooth serial
	initBTSerial();
	delay(9000);
	preferences.begin("WiFiCred", false);
    //preferences.clear();

	bool hasPref = preferences.getBool("valid", false);
	if (hasPref) {
		ssidPrim = preferences.getString("ssidPrim","");
		ssidSec = preferences.getString("ssidSec","");
		pwPrim = preferences.getString("pwPrim","");
		pwSec = preferences.getString("pwSec","");

		if (ssidPrim.equals("") 
				|| pwPrim.equals("")
				|| ssidSec.equals("")
				|| pwPrim.equals("")) {
			Serial.println("Found preferences but credentials are invalid");
		} else {
			Serial.println("Read from preferences:");
			Serial.println("primary SSID: "+ssidPrim+" password: "+pwPrim);
			Serial.println("secondary SSID: "+ssidSec+" password: "+pwSec);
			hasCredentials = true;
		}
	} else {
		Serial.println("Could not find preferences, need send data over BLE");
	}
	preferences.end();

	if (hasCredentials) {
		// Check for available AP's
		if (!scanWiFi()) {
			Serial.println("Could not find any AP");
		} else {
			// If AP was found, start connection
			connectWiFi();
		}
	}


}

void loop() {
    if (isConnected) {
        animate.loop();
    }
	// Do an else on isConnected if you want to avoid having Bluetooth serial at the same time
	if (SerialBT.available() != 0) {
		readBTSerial();
	}
}