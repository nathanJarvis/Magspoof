
int buttonReading=0;
void setup() {

  pinMode(0, OUTPUT);
  pinMode(1, INPUT);
}

// the loop function runs over and over again forever
void loop() {
  buttonReading = digitalRead(1);
  if(buttonReading == HIGH){
    digitalWrite(0, HIGH);
  }
  else{
    digitalWrite(0, LOW);
  }
  
}
