#include "UbidotsEsp32Mqtt.h"
#include "MQUnifiedsensor.h"
#include "Wire.h" 
/****************************Define Constants****************************************/

const char *UBIDOTS_TOKEN = "ADD UBIDOTS TOKEN";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "ADD WIFI SSID";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "ADD WIFI PASSWORD";      // Put here your Wi-Fi password

const char *DEVICE_LABEL = "esp32";   // Put here your Device label to which data  will be published

const char *VARIABLE_LABEL1 = "CH4"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL2 = "CO";
const char *VARIABLE_LABEL3 = "CO2";
const char *VARIABLE_LABEL4 = "NO2";
const char *VARIABLE_LABEL5 = "H2S"; 
const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds
unsigned long timer;
/************************Hardware Related Macros************************************/
#define         Board                   ("ESP-32") // Wemos ESP-32 or other board, whatever have ESP32 core.
#define         Pin_MQ2                 (34)       //IO for MQ2
#define         Pin_MQ135               (12)       //IO for MQ135 "ACTUAL 12"
#define         Pin_MQ136               (33)       //IO for MQ136 CHANGE THIS PIN //CONN
#define         LED_R                   (5)       //IO for RED LED
#define         LED_Y                   (4)        //IO for YELLOW LED 
#define         LED_G                   (15)        //IO for GREEN LED 
/***********************Software Related Macros************************************/
#define         Type_MQ2                ("MQ-2") 
#define         Type_MQ135              ("MQ-135") 
#define         Type_MQ136              ("MQ-136")
#define         RatioMQ135CleanAir        (3.6) //RS / R0 = 3.6 ppm (or 3.6 ppm?) 
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10)
/*****************************Globals***********************************************/

Ubidots ubidots(UBIDOTS_TOKEN);
//Declare Sensor
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin_MQ2, Type_MQ2);
MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin_MQ135, Type_MQ135);
MQUnifiedsensor MQ136(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin_MQ136, Type_MQ136);

/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

/***********************************Main Functions**********************************/

/*****************************Globals***********************************************/
  int ref_Voltage = 5;
  float mq2_analog_value;
  float mq135_analog_value;
  float mq136_analog_value;
  
  float CH4;
  float CO;
  float CO2;
  float NO2;
  float H2S; 

void setup() 
{
  //Init the serial port communication - to debug the library
  Serial.begin(115200); //Init serial port
  //ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  
  timer = millis();
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);

  MQ2.init();

  MQ135.init();
  
  MQ136.init(); 
 /*****************************  MQ CAlibration ********************************************
  Serial.print("Calibrating please wait.");
  float  //MQ2calcR0 =0,
         MQ135calcR0 = 0;
         //MQ136calcR0 = 0,
  for (int i = 1; i <= 10; i ++)
  {
    //Update the voltage lectures
    //MQ2.update();
    MQ135.update();
    //MQ136.update();
    
    //MQ2calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    MQ135calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    //MQ136calcR0 += MQ136.calibrate(RatioMQ136CleanAir);
    
    Serial.print(".");
  }

  //MQ2.setR0(MQ2calcR0 / 10);
  MQ135.setR0(MQ135calcR0 / 10);
  //MQ136.setR0(MQ135calcR0 / 10);
  
  Serial.println("  done!.");

  Serial.print("(MQ2, MQ135 & MQ136):");
  
  //Serial.print(MQ2calcR0 / 10); Serial.print(" | ");
  Serial.print(MQ135calcR0 / 10); Serial.print(" | ");
  //Serial.print(MQ136calcR0 / 10); Serial.print(" | ");
  
  /*****************************  MQ CAlibration ********************************************/ 
}

void loop() 
{
  
  MQ2.update();
  MQ135.update();  
  MQ136.update(); 
  
  // put your main code here, to run repeatedly:
  if (!ubidots.connected())
  {
    ubidots.reconnect();
    Serial.print("ubidots conneting");
  }
  if ((millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    
    mq2_analog_value = analogRead(Pin_MQ2);
    int Voltage_mq2 = mq2_analog_value * ref_Voltage; //(Rs/R0) - mq2
    mq135_analog_value = analogRead(Pin_MQ135);
    int Voltage_mq135 = mq135_analog_value * ref_Voltage; //(Rs/R0) - mq135
    mq136_analog_value = analogRead(Pin_MQ136); 
    int Voltage_mq136 = mq136_analog_value * ref_Voltage;

    CH4 = 1000 * (2.6 * pow(Voltage_mq2, -1.2));
   
    CO = 1000 * (9.95 * pow(Voltage_mq135, -1.22));
   
    CO2 = 1000 * (4.095 * pow(Voltage_mq135, -1.01));
    
    NO2 = 1000 * (1.8 * pow(Voltage_mq135*5, -1.08));
    
    H2S = 10.0 * pow(10.0, ((2.41 - Voltage_mq136) / 0.53));
    
    //Send gas conc to Ubidots
    ubidots.add(VARIABLE_LABEL1, CH4); // Insert your variable Labels and the value to be sent
    ubidots.add(VARIABLE_LABEL2, CO); // Insert your variable Labels and the value to be sent
    ubidots.add(VARIABLE_LABEL3, CO2); // Insert your variable Labels and the value to be sent
    ubidots.add(VARIABLE_LABEL4, NO2); // Insert your variable Labels and the value to be sent
    ubidots.add(VARIABLE_LABEL5, H2S); // Insert your variable Labels and the value to be sent // CONN
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  
  ubidots.loop();
    
  delay(500); //Sampling frequency
  
  //LED Management
  if ((CH4 <= 800) && (CO <= 70) && (CO2 <= 4000) && (NO2 < 20) && (H2S < 20))
  {
    digitalWrite(15, HIGH);    //GREEN LED is ON  
    digitalWrite(4, LOW);     //YELLOW LED is OFF
    digitalWrite(5, LOW);    //RED LED is OFF
    Serial.println("Green LED ON");
    delay(10000);
  }
  else if ((CH4 >= 1200) && (CO <= 150) || (CO2 >= 5000) || (NO2 > 50) || (H2S > 300))
  {
    digitalWrite(15, LOW);     //GREEN LED is OFF  
    digitalWrite(4, LOW);     //YELLOW LED is OFF
    digitalWrite(5, HIGH);   //RED LED is ON
    Serial.println("Red LED ON");
    delay(10000);//5,4,21
  }
  else
  {
    digitalWrite(15, LOW);     //GREEN LED is OFF   
    digitalWrite(4, HIGH);    //YELLOW LED is ON 
    digitalWrite(5, LOW);    //RED LED is OFF 
    Serial.println("Yellow LED ON");
    delay(10000);
  }
  
  //Serial.print("Smoke:    "); Serial.println(smoke);
  Serial.print("Methane:  "); Serial.println(CH4);
  Serial.print("CO:       "); Serial.println(CO);
  Serial.print("CO2:      "); Serial.println(CO2);
  Serial.print("NO2:      "); Serial.println(NO2);
  Serial.print("H2S:      "); Serial.println(H2S);
  Serial.println("--------------------------------------------------------");
}



/*
// Threshold value

int CO2Thres_healthy = 28000; //MNDOLI (Minnesota Department of Labour & Industry) - workplace safety standard: 30,000PPM for 15 minutes
int COThres_healthy = 85; //WHO (World Health Organisation) - workplace safety standard: 90-100PPM for 15 minutes
int NH4Thres_healthy = 30; //OSHA (Occupational Safety and Health Administration) - workplace safety standard: 50PPM causes irritation
int CH4Thres_healthy = 950; //OSHA (Occupational Safety and Health Administration) - workplace safety standard: 1000PPM
int SmokeThres_healthy = 1;

int CO2Thres_warning = 30000; //MNDOLI (Minnesota Department of Labour & Industry) - workplace safety standard: 30,000PPM for 15 minutes
int COThres_warning = 90; //WHO (World Health Organisation) - workplace safety standard: 90-100PPM for 15 minutes
int NH4Thres_warning = 50; //OSHA (Occupational Safety and Health Administration) - workplace safety standard: 50PPM causes irritation
int CH4Thres_warning = 1000; //OSHA (Occupational Safety and Health Administration) - workplace safety standard: 1000PPM
int SmokeThres_warning = 1.001;*/
