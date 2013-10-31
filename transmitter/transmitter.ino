#include <TimedAction.h>

#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>

const int transmit_pin = 2;
const int receive_pin = 3;

//pins
#define FRONTDOOR 12
#define BACKDOOR 10
#define DOORTOGARAGE 11

//commands
#define DOOR1OPEN 0
#define DOOR1CLOSE 1
#define DOOR2OPEN 2
#define DOOR2CLOSE 3
#define LIGHT1ON 4
#define LIGHT1OFF 5
#define STATUS 6

//status
#define DOOR1STATE 0
#define DOOR2STATE 1
#define LIGHT1STATE 2
#define LIGHT2STATE 3
#define TEMP1 4
#define LIGHTLEVEL1 5
#define LIGHTLEVEL2 6

int frontdoorstate;
int backdoorstate;
int doortogaragestate;

//create object
EasyTransferVirtualWire ETin, ETout;

struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int command;
  int extra;
};

struct RECEIVE_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int item;
  int value;
};


//give a name to the group of data
RECEIVE_DATA_STRUCTURE rxdata;
SEND_DATA_STRUCTURE txdata;

TimedAction timedActionGL = TimedAction(600000,garagelighttimeout);

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
  
  vw_rx_start();
  
  pinMode(FRONTDOOR, INPUT);        // switchPin is an input
  pinMode(BACKDOOR, INPUT);        // switchPin is an input
  pinMode(DOORTOGARAGE, INPUT);        // switchPin is an input  
  digitalWrite(FRONTDOOR, HIGH);    // Activate internal pullup resistor
  digitalWrite(BACKDOOR, HIGH);    // Activate internal pullup resistor
  digitalWrite(DOORTOGARAGE, HIGH);    // Activate internal pullup resistor  
  
  timedActionGL.disable();
}

void loop(){
  
  //timedAction.check();

  int readdoorswitch1=digitalRead(FRONTDOOR);  
  if (frontdoorstate!=readdoorswitch1) {
    frontdoorstate=readdoorswitch1;
    if (frontdoorstate==1){
      Serial.println("front door open");
    } else {
      Serial.println("front door closed");      
    }
  }

  int readdoorswitch2=digitalRead(BACKDOOR);
  if (backdoorstate!=readdoorswitch2) {
    backdoorstate=readdoorswitch2;
    if (backdoorstate==1){
      Serial.println("back door open");
    } else {
      Serial.println("back door closed");      
    }
  }
  
   int readdoorswitch3=digitalRead(DOORTOGARAGE);
   if (doortogaragestate!=readdoorswitch3) {
    doortogaragestate=readdoorswitch3;
    if (doortogaragestate==1){
      Serial.println("door to garage open");
      sendcommand(LIGHT1ON,-1);
      timedActionGL.reset();
      timedActionGL.enable();
    } else {
      Serial.println("door to garage closed");      
    }
  }
  
  if (Serial.available() > 0) {
     // we receive a char representing an integer. let's converto to int
    char incomingCommand = Serial.read();
    
    // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingCommand, DEC);
    
    incomingCommand=incomingCommand-48;
    
    if (incomingCommand<=5) {
      sendcommand(incomingCommand,-1);
    } else {
      sendcommand(6,incomingCommand-6);
    }
  }
  
  if(ETin.receiveData()){
    
    Serial.println("received data");

    Serial.print("item=");
     switch (rxdata.item) {
      case DOOR1STATE:
        Serial.println("door1state"); 
        break;
      case DOOR2STATE:
        Serial.println("door2state");
        break;
      case LIGHT1STATE:
        Serial.println("light1state");
        break;
      case LIGHT2STATE:
        Serial.println("light2state");
        break;
      case TEMP1:
        Serial.println("temp1");
        break;
      case LIGHTLEVEL1:
        Serial.println("lightlevel1");
        break;
      case LIGHTLEVEL2:
        Serial.println("lightlevel2");
        break;      
    }       
   
    Serial.print("value="); 
    Serial.println(rxdata.value);  

  }
  
  //turn light off after timeout
  timedActionGL.check();
    
}

void sendcommand(int command, int extra){
   //this is how you access the variables. [name of the group].[variable name]
  txdata.command = command;
  txdata.extra = extra;
  
  Serial.println("send data");
  //send the data
  ETout.sendData(); 
}

void garagelighttimeout(){
  Serial.println("light timeout");
  
  sendcommand(LIGHT1OFF,-1);
  timedActionGL.disable();
}
