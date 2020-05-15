#pragma once

#include "nvme.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

String AP_MAC = WiFi.macAddress();
String AP_SSID = "LandoAP-" + AP_MAC.substring(9);
String AP_PWD = "12345678";

NVME nvme = NVME();

class WIFI {
private:
	String GetUser() { return nvme.GetWiFiUser(); }
	String GetPwd() { return nvme.GetWiFiPwd(); }
	bool SetUser(String data) { return nvme.SetWiFiUser(data); }
	bool SetPwd(String data) { return nvme.SetWiFiPwd(data); }

	bool WiFiConnect(String ssid, String pass) {
		WiFi.mode(WIFI_STA);
		WiFi.begin(ssid, pass);

		byte res = 0, count = 0;
		while (true) {
			res = WiFi.status();
			switch (res) {
				case WL_CONNECTED:
					return true;
				case WL_CONNECT_FAILED:
					return false;
				default:
					if (count++ > 30)
						return false;
					break;
			}
			delay(1000);
		}
	}

	bool WiFiCreateAP() {
		ESP8266WebServer web_server(80);
		WiFi.mode(WIFI_AP);
		WiFi.softAP(AP_SSID, AP_PWD);

		bool isWaiting = true;

		web_server.on("/", HTTP_GET, [&](){
			web_server.send(200, "text/html", "<h1>You are connected</h1>");
		});
		web_server.on("/set", HTTP_GET, [&]() {
			if (web_server.hasArg("ssid") && web_server.hasArg("pass")) {
				String ssid = web_server.arg("ssid");
				String pass = web_server.arg("pass");
				if (nvme.SetWiFiUser(ssid) && nvme.SetWiFiPwd(pass)) {
					web_server.send(200, "text/plain", "Setting SSID: " + ssid);
					delay(1000);
					isWaiting = false;
				}
				else
					web_server.send(200, "text/plain", "Error setting SSID and PWD");
			}
			else
				web_server.send(200, "text/plain", "Error setting SSID and PWD");
		});
		web_server.on("/exit", HTTP_GET, [&]() {
			web_server.send(200, "text/html", "<h1>Exiting web server</h1>");
			isWaiting = false;
		});
		web_server.begin();

		double time = millis();
		while(isWaiting) {
			web_server.handleClient();
			if ((millis() - time) > (double)(5.0 * 60 * 1000))
				return false;
		}
		return true;
	}

public:
	WIFI(bool autoConnect);
	IPAddress localIP();
	bool Connect();
};

WIFI::WIFI(bool autoConnect = false) {
	WiFi.setAutoConnect(autoConnect);
}

IPAddress WIFI::localIP() {
	return WiFi.localIP();
}


bool WIFI::Connect() {
	if (WiFiConnect(GetUser(), GetPwd()))
		return true;

	if(WiFiCreateAP() && WiFiConnect(GetUser(), GetPwd()))
		return true;
	return false;
}

