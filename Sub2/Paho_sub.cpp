#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include "MQTTClient.h"
#include <wiringPi.h>
#include <unistd.h>

#define ADDRESS "tcp://192.168.1.97:1883"
#define CLIENTID "rpi2"  //comment this out for second subscriber
//#define CLIENTID "rpi3" // uncomment for second subscriber
#define AUTHMETHOD "Hooke915!"
#define AUTHTOKEN "Hooke915!"
#define TOPIC "ee513/CPUTemp"
#define PAYLOAD "Hello World!"
#define QOS 1
#define TIMEOUT 10000L

#define GPIO_ROLL 0   //this is the roll GPIO
#define GPIO_PITCH 2  //this is the pitch GPIO 

using namespace std;
char buffer[1000];
float float_array [10];

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
printf("Message with token value %d delivery confirmed\n", dt);
deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
int i;
char* payloadptr;
printf("Message arrived\n");
printf("topic: %s\n", topicName);
printf("message: ");
payloadptr = (char*) message->payload;
printf("The payload: %s",payloadptr);
sprintf(buffer, "individual: %s",payloadptr);


int n = sizeof(buffer);
char char_array[n+1];
int a;

stringstream str_strm;
str_strm.precision(2);
str_strm << buffer;
string temp_str;
float temp_int;
int z=0;
for (int d =0; d<sizeof(buffer); d++){

str_strm>> temp_str; //take wordsinto temp_str one by one
if(stringstream(temp_str)>>temp_int){//try to convert string to int

float_array[z]=temp_int;
z++;
}
temp_str = " "; //clear temp string
}

putchar('\n');
printf("Pitch: %f\n", float_array[1]);
printf("Roll: %f\n",float_array[2]);
if (float_array[1]>10){  //comment out for second subscriber
digitalWrite(GPIO_ROLL, HIGH);} //comment out for second subscriber
else {digitalWrite(GPIO_ROLL, LOW);} //comment out for second subscriber
//if(float_array[2]>10){ //uncomment for second subscriber
//digitalWrite(GPIO_PITCH, HIGH);} //uncomment for second subscriber
//else {digitalWrite(GPIO_PITCH, LOW);} //uncomment for second subscriber

MQTTClient_freeMessage(&message);
MQTTClient_free(topicName);
return 1;
}

void connlost(void *context, char *cause) {
printf("\nConnection lost\n");
printf("cause: %s\n", cause);
}

void getNumberFromString (std::string s) {
stringstream str_strm;
str_strm<< s; //convert sting into stringstream
string temp_str;
int temp_int;
while (str_strm.eof() != 1){
str_strm >> temp_str;//take words into temp str one by one
if(stringstream(temp_str) >> temp_int){ //try to convert string to int
cout <<temp_int<<" ";
}
temp_str = " "; //clear temp string
}
}

int main(int argc, char* argv[]) {
//MQTTClient client;

wiringPiSetup();
pinMode(GPIO_PITCH, OUTPUT);
pinMode(GPIO_ROLL, OUTPUT);
digitalWrite(GPIO_ROLL, LOW);
digitalWrite(GPIO_PITCH, LOW);


MQTTClient client;
MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
int rc;
int ch;
MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
opts.keepAliveInterval = 20;
opts.cleansession = 0;
opts.username = AUTHMETHOD;
opts.password = AUTHTOKEN;
MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
printf("Failed to connect, return code %d\n", rc);
exit(-1);
}
printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
"Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
MQTTClient_subscribe(client, TOPIC, QOS);
do {
ch = getchar();
} while(ch!='Q' && ch != 'q');
MQTTClient_disconnect(client, 10000);
MQTTClient_destroy(&client);
return rc;
}
