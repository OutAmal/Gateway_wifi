#include <SPI.h>
#include <AIR430BoostFCC.h>
#include <WiFi.h>
#include <PubSubClient.h>


unsigned char rxData[60];           // Data to read from radio
char ssid[] = "network name";
char password[] = "network password ";
char server[] = "iot.eclipse.org";  // MQTTServer
WiFiClient wifiClient;              // Connecting to MQTT broker via Wi-Fi
PubSubClient client(server, 1883, callback, wifiClient);  // Initialize MQTT client

// -----------------------------------------------------------------------------
// RF packet received!

void printRxData()
{
  // If RF data received, print diagnostic info to Serial Monitor & Publish over MQTT
  Serial.print("RX (DATA, RSSI, LQI, CRCBIT): ");
  Serial.print("(");
  Serial.print((char*)rxData);
  Serial.print(", ");
  Serial.print(Radio.getRssi());
  Serial.print(", ");
  Serial.print(Radio.getLqi());
  Serial.print(", ");
  Serial.print(Radio.getCrcBit());
  Serial.println(")");

  // Publish latest RF payload to the cloud via MQTT, Blink Yellow LED if successful
  if(client.publish("Sensor_data",(char*)rxData)) {
    digitalWrite(YELLOW_LED, HIGH);
    Serial.println("MQTT Publish success!");
    digitalWrite(YELLOW_LED, LOW);
  } else {
    Serial.println("MQTT Publish failed!");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {

}


void setup()
{
  //Setup LED for example demonstration purposes.
  pinMode(RED_LED, OUTPUT);       // RED LED Notifier for subGHz RF Rx
  digitalWrite(RED_LED, LOW);
  pinMode(GREEN_LED, OUTPUT);     // GREEN LED Notifier for Wi-Fi connected
  digitalWrite(GREEN_LED, LOW);
  pinMode(YELLOW_LED, OUTPUT);    // YELLOW LED Notifier for MQTT Pub successful
  digitalWrite(YELLOW_LED, LOW);
  pinMode(PUSH1, INPUT);          // Configure PUSH1. Used to decide how we will connect to Wi-Fi

  // Setup serial for debug printing.
  Serial.begin(115200);



  // SMARTCONFIG if PUSH1 was pressed during start up
  if(digitalRead(PUSH1) == 1){
    Serial.print("SMARTCONFIG MODE - ");
    Serial.println("Starting WiFi SmartConfig...");
    Serial.println("Use SmartConfig app to pass Wi-Fi credentials to your LaunchPad.");
    WiFi.startSmartConfig();

  // else, ATTEMPT TO CONNECT TO DEFAULT WIFI NETWORK
  } else{
    Serial.print("Attempting to connect to Network named: ");
    // print the network name (SSID);
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ssid, password);
    while ( WiFi.status() != WL_CONNECTED) {
      // print dots while we wait to connect
      Serial.print(".");
      delay(300);
    }
  }

  // Connected to Wi-Fi!
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");

  // Wait for IP Address
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nIP Address obtained");

  // We are connected and have an IP address. Print the WiFi status.
  printWifiStatus();


  Radio.begin(0x01, CHANNEL_1, POWER_MAX);
}

void loop()
{
  if (!client.connected()) {
    Serial.println("Disconnected. Reconnecting to MQTT Broker");
    client.connect("myCC3200_gateway");
    Serial.println("Connected to MQTT Broker!");
  }

  // Turn on the receiver and listen for incoming data.
  // The receiverOn() method returns the number of bytes copied to rxData.
  if (Radio.receiverOn(rxData, 60, 1000) > 0)
  {

    digitalWrite(RED_LED, HIGH);
    printRxData();                  // RX debug information
    digitalWrite(RED_LED, LOW);
  }

  // Ping MQTT broker to maintain connection
  client.poll();
}

void printWifiStatus() {

  Serial.print("network name : ");
  Serial.println(WiFi.SSID());

  // print your WiFi IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  digitalWrite(GREEN_LED, HIGH);  // Connected to WiFi LED
}
