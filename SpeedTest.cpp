#include "LiquidCrystal_I2C_Spark.h"
#include <string>

LiquidCrystal_I2C *lcd;
int GATE1 = D7;
int GATE2 = D6;
volatile unsigned long Time1, Time2;
float Timing, Speed, averageSpeed;
double newSpeed, publishSpeed, Speeding;
char speedStorage[]={};
int flag1 = 0;
int flag2 = 0;
int SpeedFlag = 0;
int carCounter = 0;
float conversionFactor = 22.37e-3;
double distance = 10.00;//cm

void setup() {
    Serial.begin(9600);
    lcd = new LiquidCrystal_I2C(0x27, 16, 2);
    lcd->init();
    lcd->backlight();
    lcd->clear();
    pinMode(GATE1,INPUT_PULLDOWN);
    pinMode(GATE2,INPUT_PULLUP);
    attachInterrupt(GATE1,GO_1,RISING);
    attachInterrupt(GATE2,GO_2,FALLING);
    Particle.function("speedlimit",speedLimit);//get speed from UI
    Particle.subscribe("hook-response/Speed", myHandler, MY_DEVICES);//publish to thinkspeak
    Particle.variable("Speeding Car", &Speeding, DOUBLE);//will send notifiction via iftt when speed is above threshhold
    lcd->clear();
    lcd->print("SPEED MEASURMENT");
}

void myHandler(const char *event, const char *data) {
  // Handle the integration response
}
void resetData(){
    delay(3000);
    Time1 = 0;
    Time2 = 0;
    Speed = 0;
    flag1 = 0;
    flag2 = 0;
}
void isSpeeding(){
    if(Speed > newSpeed){
        SpeedFlag = 1;
    }
    else 
    SpeedFlag = 0; 
}
void inputSpeed(){
    for(int n = 0; n < carCounter; n++){
        speedStorage[n] = Speed;
    }
}
void getAverage(){
    for(int i=0; i < carCounter; i++){
        int total;
        total = total + speedStorage[i];
        averageSpeed = (total/carCounter);
    }
}
double GO_1(){
    Time1 = millis();
    // if(Time1 > 0){Time2 = 0;}
    flag1 = 1;
    // while(GATE2 == HIGH);
    return Time1;
}

double GO_2()
{
    Time2 = millis();
    // if(Time2 > 0){Time1 = 0;}
    flag2 = 1;
    // while(GATE1 == LOW);
    return Time2;
}
double sendSpeeding(){
    if(SpeedFlag == 1)
    {
        char* pEnd;
        String transferVal(Speed,(uint8_t)10);
        String publishSpeed = transferVal.format("%.2f",Speed);
        Speeding = strtod(publishSpeed,&pEnd);
        Particle.publish("Ex.Speed",publishSpeed,PRIVATE);
        return Speeding;
    }
    else
        SpeedFlag = 0;
}
int speedLimit(String command)
{
    if(command != NULL)//is a string check
    { 
        char stringValues[8]; //Create a character array
        command.toCharArray(stringValues,8); //Convert command toCharArray stringValues
        newSpeed = atoi(stringValues);
        return 1;
    }
    return -1;
}

void loop() {
//System.sleep(60);

    if (flag1 == 1 && flag2 == 1)
    {
        if(Time1 > Time2) {
        Timing = Time1 - Time2;  
        Speed = (((distance*1000)/Timing)*conversionFactor);
        if(Speed > 10){carCounter++;}
        }
        else if (Time2 > Time1) {
        Timing = Time2 - Time1;  
        Speed = (((distance*1000)/Timing)*conversionFactor);
        if(Speed > 10){carCounter++;}
        }
        else {
        Speed = 0;
        }   
    }
  
  if (Speed == 0) { 
        lcd->setCursor(0, 1); 
        lcd->print(".....READY!....."); 
    }
    else{
    inputSpeed();
    isSpeeding();
    sendSpeeding();
    lcd->setCursor(2, 1); 
    lcd->print(Speed);
    lcd->print(" mph");
    resetData();
    //System.sleep(D1, RISING, 30);
    }
    }
