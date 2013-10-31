#include <TimedAction.h>

#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>

const int transmit_pin = 0;
const int receive_pin = 1;

//pins
#define DOOR1_TRIGGER 2
#define DOOR2_TRIGGER 3
#define LIGHT1_TRIGGER 4
#define LIGHT2_TRIGGER 5
#define DOOR1_SWITCH_CLOSED 6
#define DOOR2_SWITCH_CLOSED 7
#define BUZZER 8
#define LEDGREEN 9
#define LEDRED 10

#define THERM1_PIN   0
#define PHOTO1_PIN   1
#define PHOTO2_PIN   2
#define KEYPAD   3


//commands
#define DOOR1OPEN 0
#define DOOR1CLOSE 1
#define DOOR2OPEN 2
#define DOOR2CLOSE 3
#define LIGHT1ON 4
#define LIGHT1OFF 5
#define LIGHT2ON 6
#define LIGHT1OFF 7
#define STATUS 8

//status
#define DOOR1STATE 0
#define DOOR2STATE 1
#define LIGHT1STATE 2
#define LIGHT2STATE 3
#define TEMP1 4
#define LIGHTLEVEL1 5
#define LIGHTLEVEL2 6

int door1state;
int door2state;
int light1state;
int light2state;

int temp1;
int lightlevel1;
int lightlevel2;

int statusloop;

//create object
EasyTransferVirtualWire ETin, ETout;

struct RECEIVE_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int command;
  int extra;
};


struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int item;
  int value;
};

//give a name to the group of data
RECEIVE_DATA_STRUCTURE rxdata;
SEND_DATA_STRUCTURE txdata;

TimedAction timedAction = TimedAction(10000,sendstatusall);
TimedAction timedActionBZ = TimedAction(5000,warningbuzzer);

void setup(){
  
  Serial.begin(9600);           // set up Serial library at 9600 bps
  
  //start the library, pass in the data details
  ETin.begin(details(rxdata));
  ETout.begin(details(txdata));
  
  // Initialise the IO and ISR
  vw_set_tx_pin(transmit_pin);
  vw_set_rx_pin(receive_pin);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);	 // Bits per sec

  vw_rx_start();       // Start the receiver PLL running
 
  randomSeed(analogRead(0));
  statusloop=0;
  
  digitalWrite(DOOR1_TRIGGER, HIGH);
  digitalWrite(DOOR2_TRIGGER, HIGH);
  digitalWrite(LIGHT1_TRIGGER, HIGH);
  digitalWrite(LIGHT2_TRIGGER, HIGH);

  // initialize the digital pin as an output.
  pinMode(DOOR1_TRIGGER, OUTPUT);     
  pinMode(DOOR2_TRIGGER, OUTPUT);     
  pinMode(LIGHT1_TRIGGER, OUTPUT);  
  pinMode(LIGHT2_TRIGGER, OUTPUT);  

  pinMode(DOOR1_SWITCH_CLOSED, INPUT);        // switchPin is an input
  pinMode(DOOR2_SWITCH_CLOSED, INPUT);        // switchPin is an input
  digitalWrite(DOOR1_SWITCH_CLOSED, HIGH);    // Activate internal pullup resistor
  digitalWrite(DOOR2_SWITCH_CLOSED, HIGH);    // Activate internal pullup resistor
  
  pinMode(BUZZER, OUTPUT); 
  pinMode(LEDGREEN, OUTPUT); 
  pinMode(LEDRED, OUTPUT);   
  
  timedActionBZ.disable();
}

void loop(){
   //Serial.println("loop");
  
  //check and see if a data packet has come in. 
  if(ETin.receiveData()){
    Serial.println("receiveData");
    Serial.println(rxdata.command);
    Serial.println(rxdata.extra);
    
     switch (rxdata.command) {
      case DOOR1OPEN:
        Serial.println("DOOR1OPEN");
        relaycommand(DOOR1_TRIGGER,DOOR1STATE,1); //(doorpin,currentstate,requestedstate) 
        break;
      case DOOR1CLOSE:
        Serial.println("DOOR1CLOSE");
        relaycommand(DOOR1_TRIGGER,DOOR1STATE,0); //(doorpin,currentstate,requestedstate)         
        break;
      case DOOR2OPEN:
        Serial.println("DOOR2OPEN");
        relaycommand(DOOR2_TRIGGER,DOOR2STATE,1); //(doorpin,currentstate,requestedstate)         
        break;
      case DOOR2CLOSE:
        Serial.println("DOOR2CLOSE");
        relaycommand(DOOR2_TRIGGER,DOOR2STATE,0); //(doorpin,currentstate,requestedstate)                 
        break;
      case LIGHT1OFF:
        Serial.println("LIGHT1OFF");
        relaycommand(LIGHT1_TRIGGER,LIGHT1STATE,0); //(doorpin,currentstate,requestedstate)         
        break;        
      case LIGHT1ON:
        Serial.println("LIGHT1ON");
        relaycommand(LIGHT1_TRIGGER,LIGHT1STATE,1); //(doorpin,currentstate,requestedstate)                         
        break;
      case STATUS:
        Serial.println("STATUS");
        delay(200);
        sendstatus(rxdata.extra);
        break;    
     }
  }
    
  // read pins to get states  
  door1state=checkstate(DOOR1STATE); 
  door2state=checkstate(DOOR2STATE); 
  light1state=checkstate(LIGHT1STATE); 
  light2state=checkstate(LIGHT2STATE);
  temp1=checkstate(TEMP1);    

  //broadcast sensor values every 30 seconds
  timedAction.check();
}

void sendstatusall(){
  Serial.println("sendstatusall");
  sendstatus(statusloop);
  
  if (statusloop<6){
    statusloop=statusloop+1;
  } else {
    statusloop=0; 
  }
  
}

void sendstatus(int statustype){
  Serial.println("send status data");
  
  txdata.item=statustype;
  
   switch (statustype) {
    case DOOR1STATE:
      txdata.value=door1state;
      break;
    case DOOR2STATE:
      txdata.value=door2state;
      break;
    case LIGHT1STATE:
      txdata.value=light1state;
      break;
    case LIGHT2STATE:
      txdata.value=light2state;
      break;
    case TEMP1:
      txdata.value=temp1;
      break;
    case LIGHTLEVEL1:
      txdata.value=lightlevel1;
      break;
    case LIGHTLEVEL2:
      txdata.value=lightlevel2;
      break;      
  }

  ETout.sendData(); 
  
}

void relaycommand(int triggerpin,int statuscommand, int desiredstate){

  int currentstate;
  
  currentstate=checkstate(statuscommand);
  
  if (currentstate!=desiredstate){
    toggle(triggerpin);
    
    if(triggerpin==LIGHT1_TRIGGER){ //turn both lights on and off together otherwise it gets complicated getting current state of light
      toggle(LIGHT2_TRIGGER);
    }
  } 
  
}

void toggle(int pin){
  digitalWrite(pin, LOW); // turns the PIN on or off
  delay(200);
  digitalWrite(pin, HIGH); // turns the PIN on or off
}

int checkstate(int statustype){
  
  int returnval=-1;
  
  switch (statustype) {
     case DOOR1STATE:
        {int readdoorswitch1=digitalRead(DOOR1_SWITCH_CLOSED);
      
        if (door1state!=readdoorswitch1) {
          door1state=readdoorswitch1;
          sendstatus(DOOR1STATE); 
        }
      returnval=readdoorswitch1;
        }
      break;
     case DOOR2STATE:
        {int readdoorswitch2=digitalRead(DOOR2_SWITCH_CLOSED);
      
        if (door2state!=readdoorswitch2) {
          door2state=readdoorswitch2;
          sendstatus(DOOR2STATE); 
        }
      returnval=readdoorswitch2;
        }
      break;
     case LIGHT1STATE:
        {int readlightlevel1=analogRead(PHOTO1_PIN);
        lightlevel1=readlightlevel1;
          
        if (readlightlevel1>=200 && light1state!=1){
          light1state=1;
          sendstatus(LIGHT1STATE);         
        } 
        else if(readlightlevel1<200 && light1state!=0){
          light1state=0;
          sendstatus(LIGHT1STATE); 
        }
      returnval=light1state;
        }
      break;
     case LIGHT2STATE:
        {int readlightlevel2=analogRead(PHOTO2_PIN);
        lightlevel2=readlightlevel2;
          
        if (readlightlevel2>=200 && light2state!=1){
          light2state=1;
          sendstatus(LIGHT2STATE);         
        } 
        else if(readlightlevel2<200 && light2state!=0){
          light2state=0;
          sendstatus(LIGHT2STATE); 
        }
      returnval=light2state; 
        }
      break;
     case TEMP1:
        {int readtemp=(Thermister(analogRead(THERM1_PIN)));
        if (temp1>readtemp+1 || temp1<readtemp-1){ //only send status if it has changed at least 2 degrees, this will keep it from sending over and over when its between 2 values
          temp1 = readtemp;
          sendstatus(TEMP1);
        }
   
        if (readtemp>=freezewarning) { // if we are above set point notify us that freezer is getting warm
          sendstatus(FREEZEWARNING);
          warningbuzzer();
          timedActionBZ.reset();
          timedActionBZ.enable();            
        }     
        
       returnval=readtemp;
        }
       break;
   }

  return returnval;
  
}

void warningbuzzer(){

}

double Thermister(int RawADC) {
  double Temp;
  Temp = log(((10240000/RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
  Temp = Temp - 273.15;            // Convert Kelvin to Celcius
  Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
  return Temp;
}



