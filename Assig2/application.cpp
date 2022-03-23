#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "ADXL345.h"
#include <unistd.h>
#include <pthread.h>
#include "MQTTClient.h"
#define CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"

using namespace std;
using namespace exploringRPi;

//Please replace the following address with the address of your server
#define ADDRESS "tcp://192.168.1.97:1883"
#define CLIENTID "rpi1"
#define AUTHMETHOD "Hooke915!"
#define AUTHTOKEN "Hooke915!"
#define TOPIC "ee513/CPUTemp"
#define QOS 2
#define TIMEOUT 10000L

float getCPUTemperature() {
// get the CPU temperature
int cpuTemp;
// store as an int 
fstream fs;
fs.open(CPU_TEMP, fstream::in); // read from the file 
fs >> cpuTemp;
fs.close();
return (((float)cpuTemp)/1000);
}

int main(int argc, char* argv[]) {
char str_payload[100];

struct timeval begin, end;
gettimeofday(&begin, 0);


ADXL345 sensor(1,0x1d);
sensor.setResolution(ADXL345::NORMAL);
sensor.setRange(ADXL345::PLUSMINUS_4_G);
int i=1;
int rc;

while (i<=10)
{
++i;

gettimeofday(&end, 0);
long seconds = end.tv_sec - begin.tv_sec;
long microseconds = end.tv_usec - begin.tv_usec;
double elapsed =seconds + microseconds*1e-6;
cout<<elapsed<<endl;

sensor.displayPitchAndRoll();

// Set your max message size here
MQTTClient client;
MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_willOptions willOpts = MQTTClient_willOptions_initializer;
MQTTClient_deliveryToken token;
MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

willOpts.topicName ="ee513/CPUTemp";
willOpts.message = "Last will triggered";
willOpts.retained = 0;
willOpts.qos =1;


opts.keepAliveInterval = 20;
opts.cleansession = 1;
opts.username = AUTHMETHOD;
opts.password = AUTHTOKEN;

//int rc;
if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
cout << "Failed to connect, return code " << rc << endl;

return -1;
}
sprintf(str_payload, "{\"CPUTemp\": %.2f, \"Pitch\": %.2f, \"Roll\": %.2f, \"Time\": %.2f}", getCPUTemperature(), sensor.getPitch(), sensor.getRoll(), (double)elapsed);
pubmsg.payload = str_payload;
pubmsg.payloadlen = strlen(str_payload);
pubmsg.qos = 1;
pubmsg.retained = 0;
MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
" seconds for publication of " << str_payload <<
" \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl;
rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
cout << "Message with token " << (int)token << " delivered." << endl;
MQTTClient_disconnect(client, 10000);
MQTTClient_destroy(&client);
//return rc;
}

return rc;
}
