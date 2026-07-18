/******************************************************************************
  OpenDAD Wi-Fi access point

  OpenDAD creates its own Wi-Fi network. A phone, tablet, or computer connects
  directly to this network to access the OpenDAD webpage.

  This file handles Wi-Fi only. The web server will be added separately in
  WebServer.ino.
******************************************************************************/

#include <WiFi.h>
#include <ESPmDNS.h>

/******************************************************************************
  Wi-Fi settings
******************************************************************************/

const char* WIFI_AP_NAME = "OpenDAD";
const char* WIFI_AP_PASSWORD = "opendad123";

// Standard ESP32 access-point address.
IPAddress openDAD_IP(192, 168, 4, 1);
IPAddress openDAD_GATEWAY(192, 168, 4, 1);
IPAddress openDAD_SUBNET(255, 255, 255, 0);

/******************************************************************************
  Start the OpenDAD Wi-Fi access point
******************************************************************************/

void setupWiFi()
{
  const char* ssid = "OpenDAD";
  const char* password = "opendad123";

  IPAddress localIP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_AP);

  if (!WiFi.softAPConfig(localIP, gateway, subnet))
  {
    Serial.println("SoftAP configuration failed");
  }

  if (!WiFi.softAP(ssid, password))
  {
    Serial.println("SoftAP startup failed");
    return;
  }

  Serial.println("OpenDAD Wi-Fi started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  if (MDNS.begin("opendad"))
  {
    Serial.println("mDNS started");
    Serial.println("Open: http://opendad.local");

    MDNS.addService("http", "tcp", 80);
  }
  else
  {
    Serial.println("mDNS startup failed");
    Serial.println("Use http://192.168.4.1");
  }
}






/******************************************************************************
  Wi-Fi status helpers
******************************************************************************/

int getConnectedDeviceCount()
{
  return WiFi.softAPgetStationNum();
}

bool isDeviceConnected()
{
  return getConnectedDeviceCount() > 0;
}

void printWiFiStatus()
{
  Serial.println();
  Serial.println("OpenDAD Wi-Fi status");
  Serial.print("  Network: ");
  Serial.println(WIFI_AP_NAME);

  Serial.print("  IP:      ");
  Serial.println(WiFi.softAPIP());

  Serial.print("  Devices: ");
  Serial.println(getConnectedDeviceCount());
}