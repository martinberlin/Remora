
// Config.h and Animate.h need configuration
#include <Config.h>  // WiFi credentials, mDNS Domain
#include <Animate.h> // PixelCount, PixelPin
#include <ESPmDNS.h>
#include <U8g2lib.h> 
#include <WiFi.h>
// Starting BLE Setup
#include <nvs.h>
#include <nvs_flash.h>
// JSON object handling
#include <ArduinoJson.h>
// Includes for BLE
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <Preferences.h>

// HELTEC Board pins, update if you use another Board:
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
byte u8cursor = 1;

// Debug mode prints to serial
bool debugMode = true;
TimerHandle_t wifiReconnectTimer;
// Animation handling class
Animate animate;
const char* localDomain = MDNSDOMAIN; // mDNS: led.local (Config.h)
struct config {
  char chipId[20];
  int udpPort = 49161; // 49161 Default Orca UDP Port
} internalConfig;

/** Build time */
const char compileDate[] = __DATE__ " " __TIME__;

/** Unique device name */
char apName[] = "ESP32-xxxxxxxxxxxx";
/** Selected network 
    true = use primary network
		false = use secondary network
*/
bool usePrimAP = true;
/** Flag if stored AP credentials are available */
bool hasCredentials = false;
/** Connection status */
volatile bool isConnected = false;
/** Connection change status */
bool connStatusChanged = false;

/**
 * Create unique device name from MAC address
 **/
void createName() {
	uint8_t baseMac[6];
	// Get MAC address for WiFi station
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	// Write unique name into apName
	sprintf(apName, "ESP32-%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
}

// List of Service and Characteristic UUIDs
#define SERVICE_UUID  "0000aaaa-ead2-11e7-80c1-9a214cf093ae"
#define WIFI_UUID     "00005555-ead2-11e7-80c1-9a214cf093ae"

/** SSIDs of local WiFi networks */
String ssidPrim;
String ssidSec;
/** Password for local WiFi network */
String pwPrim;
String pwSec;

/** Characteristic for digital output */
BLECharacteristic *pCharacteristicWiFi;
/** BLE Advertiser */
BLEAdvertising* pAdvertising;
/** BLE Service */
BLEService *pService;
/** BLE Server */
BLEServer *pServer;

/** Buffer for JSON string */
StaticJsonBuffer<200> jsonBuffer;

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
 * MyServerCallbacks
 * Callbacks for client connection and disconnection
 */
class MyServerCallbacks: public BLEServerCallbacks {
	// TODO this doesn't take into account several clients being connected
	void onConnect(BLEServer* pServer) {
		Serial.println("BLE client connected");
	};

	void onDisconnect(BLEServer* pServer) {
		Serial.println("BLE client disconnected");
		pAdvertising->start();
	}
};

/**
 * MyCallbackHandler
 * Callbacks for BLE client read/write requests
 */
class MyCallbackHandler: public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic *pCharacteristic) {
		std::string value = pCharacteristic->getValue();
		if (value.length() == 0) {
			return;
		}
		Serial.println("Received over BLE: " + String((char *)&value[0]));

		// Decode data
		int keyIndex = 0;
		for (int index = 0; index < value.length(); index ++) {
			value[index] = (char) value[index] ^ (char) apName[keyIndex];
			keyIndex++;
			if (keyIndex >= strlen(apName)) keyIndex = 0;
		}

		/** Json object for incoming data */
		JsonObject& jsonIn = jsonBuffer.parseObject((char *)&value[0]);
		if (jsonIn.success()) {
			if (jsonIn.containsKey("ssidPrim") &&
					jsonIn.containsKey("pwPrim") && 
					jsonIn.containsKey("ssidSec") &&
					jsonIn.containsKey("pwSec")) {
				ssidPrim = jsonIn["ssidPrim"].as<String>();
				pwPrim = jsonIn["pwPrim"].as<String>();
				ssidSec = jsonIn["ssidSec"].as<String>();
				pwSec = jsonIn["pwSec"].as<String>();

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
			} else if (jsonIn.containsKey("erase")) {
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
			} else if (jsonIn.containsKey("reset")) {
				WiFi.disconnect();
				esp_restart();
			}
		} else {
			Serial.println("Received invalid JSON");
		}
		jsonBuffer.clear();
	};

	void onRead(BLECharacteristic *pCharacteristic) {
		Serial.println("BLE onRead request");
		String wifiCredentials;

		/** Json object for outgoing data */
		JsonObject& jsonOut = jsonBuffer.createObject();
		jsonOut["ssidPrim"] = ssidPrim;
		jsonOut["pwPrim"] = pwPrim;
		jsonOut["ssidSec"] = ssidSec;
		jsonOut["pwSec"] = pwSec;
		// Convert JSON object into a string
		jsonOut.printTo(wifiCredentials);

		// encode the data
		int keyIndex = 0;
		Serial.println("Stored settings: " + wifiCredentials);
		for (int index = 0; index < wifiCredentials.length(); index ++) {
			wifiCredentials[index] = (char) wifiCredentials[index] ^ (char) apName[keyIndex];
			keyIndex++;
			if (keyIndex >= strlen(apName)) keyIndex = 0;
		}
		pCharacteristicWiFi->setValue((uint8_t*)&wifiCredentials[0],wifiCredentials.length());
		jsonBuffer.clear();
	}
};

/**
 * initBLE
 * Initialize BLE service and characteristic
 * Start BLE server and service advertising
 */
void initBLE() {
	// Initialize BLE and set output power
	BLEDevice::init(apName);
	BLEDevice::setPower(ESP_PWR_LVL_P7);

	// Create BLE Server
	pServer = BLEDevice::createServer();

	// Set server callbacks
	pServer->setCallbacks(new MyServerCallbacks());

	// Create BLE Service
	pService = pServer->createService(BLEUUID(SERVICE_UUID),20);

	// Create BLE Characteristic for WiFi settings
	pCharacteristicWiFi = pService->createCharacteristic(
		BLEUUID(WIFI_UUID),
		// WIFI_UUID,
		BLECharacteristic::PROPERTY_READ |
		BLECharacteristic::PROPERTY_WRITE
	);
	pCharacteristicWiFi->setCallbacks(new MyCallbackHandler());

	// Start the service
	pService->start();

	// Start advertising
	pAdvertising = pServer->getAdvertising();
	pAdvertising->start();
}

/** Callback for receiving IP address from AP */
void gotIP(system_event_id_t event) {
	isConnected = true;
	connStatusChanged = true;
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  if (!MDNS.begin(localDomain)) {
    while(1) { 
    delay(100);
    }
  }
  MDNS.addService("http", "tcp", 80);
  printMessage(String(localDomain)+".local mDns started");

  animate.startUdpListener(WiFi.localIP(), internalConfig.udpPort);
}

/** Callback for connection loss */
void lostCon(system_event_id_t event) {
	isConnected = false;
	connStatusChanged = true;
    Serial.println("WiFi lost connection, restarting");
	ESP.restart();
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
	int8_t rssiPrim = 0;
	/** RSSI for secondary network */
	int8_t rssiSec = 0;
	/** Result of this function */
	bool result = false;

	Serial.println("Start scanning for networks");

	WiFi.disconnect(true);
	WiFi.enableSTA(true);
	WiFi.mode(WIFI_STA);

	// Scan for AP
	int apNum = WiFi.scanNetworks(false,true,false,1000);
	if (apNum == 0) {
		Serial.println("Found no networks");
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

/**
 * Start connection to AP
 */
void connectWiFi() {
	// Setup callback function for successful connection
	WiFi.onEvent(gotIP, SYSTEM_EVENT_STA_GOT_IP);
	// Setup callback function for lost connection
	WiFi.onEvent(lostCon, SYSTEM_EVENT_STA_DISCONNECTED);

	WiFi.disconnect(true);
	WiFi.enableSTA(true);
	WiFi.mode(WIFI_STA);

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
 * Convert the IP to string 
 */
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]);
}

void setup()
{
  Serial.begin(115200);

  u8x8.begin();
  u8x8.setCursor(0, u8cursor);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_u);
  u8x8.print("REMORA");
  delay(100);
  u8x8.print(" .");
  delay(100);
  u8x8.print(" .");
  delay(100);
  u8x8.print(" .");
  delay(100);
  u8x8.print(" .");
  delay(100);
  u8x8.print(" .");

 	// Create unique device name
	createName();

  	// Send some device info
	Serial.print("Build: ");
	Serial.println(compileDate);

	Preferences preferences;
	preferences.begin("WiFiCred", false);
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
		u8x8.clear();
		u8x8.println("BLE APP:");
		u8x8.println();
		u8x8.println("ESP32WIFIBLE");
		u8x8.println();
		u8x8.println("USE ANDROID APP");
		u8x8.println("TO CONFIG WIFI");
		Serial.println("Could not find preferences, need send data over BLE");
	}
	preferences.end();

	// Start BLE server
	initBLE();
	if (hasCredentials) {
		// Check for available AP's
		if (!scanWiFi) {
			Serial.println("Could not find any AP");
			
		} else {
			// If AP was found, start connection
			connectWiFi();
		}
	}
}

void loop() {
  animate.loop();
}