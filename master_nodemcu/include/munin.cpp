// stolen here: https://github.com/alejandroscf/esp8266-munin-node

#define FS(X) String(F(X))

// Change as your needs!
String nodename = "esp8266";
#define MAX_SRV_CLIENTS 1

// Initialize the esp8266 server library
// with the IP address and port you want to use
// (port 4949 is default for munin):
WiFiServer server(4949);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void munin_setup() {
	// start the server:
	server.begin();
	server.setNoDelay(true);
}

void munin_server() {
   WiFiClient client = server.available();
   if (client) {
	 client.print(FS("# munin node at ") + nodename + '\n');
     // store time to enable timeout detect
     unsigned long started_waiting_at = micros();

     while (client.connected()) {
      if (micros() - started_waiting_at > 5000000 ){
        // timeout at 5 sec (5 000 000µs)
         Serial.println("Client timed out after 5sec");
         client.stop();
         break;

      }

      if (client.available()) {
        Serial.println("readString...");
        String command = client.readString();
        Serial.println(command);
        // A command has been received, reinit timeout counter
        started_waiting_at = micros();
        if (command.startsWith(FS("quit"))) break;
        if (command.startsWith(FS("version"))) {
          client.print(FS("munin node on ") + nodename + FS(" version: 1.0.0\n"));
          continue;
        }
        if (command.startsWith(FS("list"))) {
          client.print(FS("esp_w1_temp\n"));
          continue;
        }
        if (command.startsWith(FS("config esp_w1_temp"))) {
          client.print(FS("graph_title ESP8266 OneWire Temperature\n"));
          client.print(FS("graph_vlabel Temperature in Celsius\n"));
          client.print(FS("graph_category Sensors\n"));
          for (int val = 0; val < MAXNODES*2; val++) {
            if ((int)values[val] != (int)NOSENSOR) {
              client.print(FS("temp") + (val + 1) + ".warning 30\n");
              client.print(FS("temp") + (val + 1) + ".critical 40\n");
              client.print(FS("temp") + (val + 1) + ".label temp" + (val + 1) + "\n"); 
            }
          }
          client.print(FS(".\n"));
          continue;
        }
        if (command.startsWith(FS("fetch esp_w1_temp"))) {
          for (int val = 0; val < MAXNODES*2; val++) {
            char STemp[12];
			if ( values[val] != NOSENSOR ) {
				sprintf(STemp, "%f", values[val]);
	            client.print(FS("temp") + (val + 1) + ".value " + STemp + "\n");
			}
          }
          client.print(FS(".\n"));
          continue;
        }
        // no command catched
        client.print(FS("# Unknown command. Try list, config, fetch, version or quit\n"));
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("Client disconnected");
//digitalWrite(13, LOW); // turn the LED off



	}


}
