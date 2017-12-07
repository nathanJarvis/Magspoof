
#define BUTTON_INPUT 2 // the number of the pushbutton pin
#define GREEN_LED 3


// variables will change:
int buttonState = 0;// variable for reading the pushbutton status
int prevbuttonState=0;
int buttonReading=0;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;

int timer=0;

void setup() {
  pinMode(BUTTON_INPUT, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(4, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  deBounce();
 
 while(buttonState==1){//loop that checks the button state every 0.1 secs, turns on green short press LED
  timer++;
  delay(100);
  buttonState=digitalRead(BUTTON_INPUT);
  digitalWrite(3,HIGH);
 }
 if(timer<10){
  digitalWrite(3, LOW);//if the hold is less than 1 second we just turn off the green LED and recognize short press
 }
 if(timer>10){//if time is longer than 1 second we turn off the green short press LED and turn on the red one to show a long press
  digitalWrite(4, HIGH);
  digitalWrite(3,LOW);
  delay(1000);
digitalWrite(4, LOW);
  Serial.print("LONG");
 
 }
timer=0;//resets timer to 0 for a new cycle 

}


void deBounce(){
 // read the state of the pushbutton value:
  buttonReading = digitalRead(BUTTON_INPUT);
  Serial.print("Current Button State:");
  Serial.println(buttonState);
  Serial.print("Previous Button State:");
  Serial.println(prevbuttonState);


  if(buttonState != prevbuttonState){
    lastDebounceTime=millis();
  }
  
  if((millis()-lastDebounceTime)>debounceDelay){
    //valid reading
    buttonState=buttonReading;

    }

  prevbuttonState=buttonState;
  delay(50);
}

