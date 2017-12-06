
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define PIN_A 0
#define PIN_B 1
#define ENABLE_PIN 3 // also green LED
#define BUTTON_PIN 2
#define INIDICATOR_PIN 4
#define CLOCK_US 200
//try as define otherwise make long
#define TIME_DELAY 200


#define BETWEEN_ZERO 53 // 53 zeros between track1 & 2


//-------------------CHANGE FOR NEW CARD------------------
#define NUM_TRACKS 4 //INCREMENT BY 2 FOR EACH CARD

// consts get stored in flash as we don't adjust them
const char* tracks[] = {
";00013=02925164729?\0", //  CARD 1 TRACK 1
";00013=02925164729?\0", //CARD 1 TRACK 2
";00013=03929752819?\0", //CARD 2 Track 1
";00013=03929752819?\0"// CARD 2 Track 2, 
//ADD ADDITIONAL CARD FOLLOWING FORMAT
};

//--------------------------------------------------------

char revTrack[41];


//-------------------CHANGE FOR NEW CARD------------------
const int sublen[] = {
  32, 48, 32, 48 }; //ADD 32, 48 for new card
const int bitlen[] = {
  7, 5, 7, 5 }; //ADD 7, 5 for new card
//--------------------------------------------------------

unsigned int curTrack = 1;
int dir;
long start_time = 0;
long elapsed_time = 0;

void setup()
{
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(INIDICATOR_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  flash(50, 5);

  

  // store reverse to play later
  storeRevTrack(2);
}

void flash(int msdelay, int times){
   for (int i = 0; i < times; i++)
  {
    digitalWrite(INIDICATOR_PIN, HIGH);
    digitalWrite(ENABLE_PIN, LOW);
    delay(msdelay);
    digitalWrite(INIDICATOR_PIN, LOW);
    digitalWrite(ENABLE_PIN, HIGH);
    delay(msdelay);
  }
  digitalWrite(ENABLE_PIN, LOW);
}

void blink(int pin, int msdelay, int times)
{
  for (int i = 0; i < times; i++)
  {
    digitalWrite(pin, HIGH);
    delay(msdelay);
    digitalWrite(pin, LOW);
    delay(msdelay);
  }
}

// send a single bit out
void playBit(int sendBit)
{
  dir ^= 1;
  digitalWrite(PIN_A, dir);
  digitalWrite(PIN_B, !dir);
  delayMicroseconds(CLOCK_US);

  if (sendBit)
  {
    dir ^= 1;
    digitalWrite(PIN_A, dir);
    digitalWrite(PIN_B, !dir);
  }
  delayMicroseconds(CLOCK_US);

}

// when reversing
void reverseTrack(int track)
{
  int i = 0;
  track--; // index 0
  dir = 0;

  while (revTrack[i++] != '\0');
  i--;
  while (i--)
    for (int j = bitlen[track]-1; j >= 0; j--)
      playBit((revTrack[i] >> j) & 1);
}

// plays out a full track, calculating CRCs and LRC
void playTrack(int track)
{
  int tmp, crc, lrc = 0;
  track--; // index 0
  dir = 0;

  // enable H-bridge and LED
  digitalWrite(ENABLE_PIN, HIGH);

  // First put out a bunch of leading zeros.
  for (int i = 0; i < 25; i++)
    playBit(0);

  //
  for (int i = 0; tracks[track][i] != '\0'; i++)
  {
    crc = 1;
    tmp = tracks[track][i] - sublen[track];

    for (int j = 0; j < bitlen[track]-1; j++)
    {
      crc ^= tmp & 1;
      lrc ^= (tmp & 1) << j;
      playBit(tmp & 1);
      tmp >>= 1;
    }
    playBit(crc);
  }

  // finish calculating and send last "byte" (LRC)
  tmp = lrc;
  crc = 1;
  for (int j = 0; j < bitlen[track]-1; j++)
  {
    crc ^= tmp & 1;
    playBit(tmp & 1);
    tmp >>= 1;
  }
  playBit(crc);

  // if track 1 of card, play 2nd track in reverse
  //-----------------------------------------------
  if (track%2 == 0)
  {
    // if track 1 of card, also play track 2 in reverse
    // zeros in between
    for (int i = 0; i < BETWEEN_ZERO; i++)
      playBit(0);

    // send second track in reverse
    reverseTrack(track+2);
  }

  // finish with 0's
  for (int i = 0; i < 5 * 5; i++)
    playBit(0);

  digitalWrite(PIN_A, LOW);
  digitalWrite(PIN_B, LOW);
  digitalWrite(ENABLE_PIN, LOW);

}



// stores track for reverse usage later
void storeRevTrack(int track)
{
  int i, tmp, crc, lrc = 0;
  track--; // index 0
  dir = 0;

  for (i = 0; tracks[track][i] != '\0'; i++)
  {
    crc = 1;
    tmp = tracks[track][i] - sublen[track];

    for (int j = 0; j < bitlen[track]-1; j++)
    {
      crc ^= tmp & 1;
      lrc ^= (tmp & 1) << j;
      tmp & 1 ?
        (revTrack[i] |= 1 << j) :
        (revTrack[i] &= ~(1 << j));
      tmp >>= 1;
    }
    crc ?
      (revTrack[i] |= 1 << 4) :
      (revTrack[i] &= ~(1 << 4));
  }

  // finish calculating and send last "byte" (LRC)
  tmp = lrc;
  crc = 1;
  for (int j = 0; j < bitlen[track]-1; j++)
  {
    crc ^= tmp & 1;
    tmp & 1 ?
      (revTrack[i] |= 1 << j) :
      (revTrack[i] &= ~(1 << j));
    tmp >>= 1;
  }
  crc ?
    (revTrack[i] |= 1 << 4) :
    (revTrack[i] &= ~(1 << 4));

  i++;
  revTrack[i] = '\0';
}

void sleep()
{
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT2);                   // Use PB3 as interrupt pin
  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

  MCUCR &= ~_BV(ISC01);
  MCUCR &= ~_BV(ISC00);       // Interrupt on rising edge
  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
  sleep_cpu();                            // sleep

  cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT2);                  // Turn off PB3 as interrupt pin
  sleep_disable();                        // Clear SE bit
  ADCSRA |= _BV(ADEN);                    // ADC on

  sei();                                  // Enable interrupts
}

// XXX move playtrack in here?
ISR(PCINT0_vect) {
  /*  noInterrupts();
   while (digitalRead(BUTTON_PIN) == LOW);
   delay(50);
   while (digitalRead(BUTTON_PIN) == LOW);
   playTrack(1 + (curTrack++ % 2));
   delay(400);
   interrupts();*/

}


void loop()
{

  
  while (digitalRead(BUTTON_PIN) == HIGH){
    //do nothing
  }
  start_time = millis();
  while(digitalRead(BUTTON_PIN) == LOW){
    //do nothing
  }
  elapsed_time = millis() - start_time;
  
  if(elapsed_time > TIME_DELAY){
    //switch tracks
     curTrack += 2;
     curTrack = curTrack % NUM_TRACKS;
     storeRevTrack(curTrack + 1);
     flash(25, 5);
     delay(100);
     blink(INIDICATOR_PIN, 50, (curTrack/2)+1);  
  }
  else{
    //activate device
     blink(INIDICATOR_PIN, 50, (curTrack/2)+1);
     playTrack(curTrack);
     //wait before next input
     digitalWrite(INIDICATOR_PIN,HIGH);
     delay(400);
     digitalWrite(INIDICATOR_PIN,LOW);
     interrupts();
  }
  
}

