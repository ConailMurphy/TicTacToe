#include <IRremote.h>

// Pin where the IRSensor is connected to
#define IRSensorPin 11
// draw is 0x00
#define DRAW B00000000
// define remote control keys 1-9
#define r1 0xff30cf
#define r2 0xFF18E7
#define r3 0xFF7A85
#define r4 0xFF10EF
#define r5 0xFF38C7
#define r6 0xFF5AA5
#define r7 0xFF42BD
#define r8 0xFF4AB5
#define r9 0xFF52AD
// define keyboard keys 1-9 (not used currently)
#define k1 49
#define k2 50
#define k3 51
#define k4 52
#define k5 53
#define k6 54
#define k7 55
#define k8 56
#define k9 57
// sensor
IRrecv irrecv(IRSensorPin);
decode_results results;
// counter for games won by p1 and p2 and draws
byte c1 = 0;
byte c2 = 0;
byte d = 0;
// define masks for player 1 and 2
byte p1 = B10101000;
byte p2 = B01010100;
byte winner = B00000000; // equals p1 or p2 as winner or 0x00 if draw
// define masks for rows 1-3 - REVEERT TO all 0
byte row1 = B00000000;
byte row2 = B00000000;
byte row3 = B00000000;
// define set bytes (i.e. which LED is already set by one player)
//byte set[9] = {0,0,0,0,0,0,0,0,0};
// alternatively use only one integer and set appropriate bit (instead of wasting space for 9 bytes)
int bitset = 0;
// boolean determine which player's turn it is
bool first;
bool finished;

void setup() {
  first = true; // first player begins
  finished = false; // game not finished
  Serial.begin(9600); // serial console
  irrecv.enableIRIn(); // start receiver
 // Keyboard.begin(); // only with another arduino
  randomSeed(analogRead(0));
  DDRD |= B11111100; // set ports 2-7 as outputs
  PORTD |= B00000000; // set all ports to low
  DDRB |= B000111; // set ports 8-10 as outputs
  PORTB |= B000000; // set all ports to low
  Serial.println("Starting game:");
  Serial.println("");
}

void printWinner(){
  if(winner == p1){
   Serial.println("Winner: Player 1");
 } else if (winner == p2){
   Serial.println("Winner: Player 2");
 } else if (winner == DRAW){
  Serial.println("Draw");
 }
  Serial.println("Statistics:");
  Serial.print("Player 1");
  Serial.print("\t");
  Serial.print("Player 2");
  Serial.print("\t");
  Serial.println("Draws");
  Serial.print(c1, DEC);
  Serial.print("\t\t");
  Serial.print(c2, DEC);
  Serial.print("\t\t");
  Serial.println(d, DEC);
  Serial.println("");
}

void turnOffLeds(){
 PORTD |= B00000000;
 PORTB |= B000000;
}

void turnOnLeds(){
 // Serial.println("175");
 // initially turn all LEDs off
 PORTD |= B00000000;
 PORTB |= B000000;
  // light row 1
  PORTD = row1;
  PORTB = DDRB & B000110;
  delay(1);
  // light row 2
  PORTD = row2;
  PORTB = DDRB & B000101;
  delay(1);
  // light row 3
  PORTD = row3;
  PORTB = DDRB & B000011; 
  delay(1); // prevent immediate repeat
}

void flash(){
  int t=0;
  while(t<5){
    turnOffLeds();
    delay(100);
    turnOnLeds();
    delay(150);
    turnOffLeds();
    delay(50);
    t+=1;
  }
}

void resetEverything(){
 // resets all values. called after one player won or a draw
 row1 = B00000000;
 row2 = B00000000;
 row3 = B00000000;
 // set winner back to draw
 winner = B00000000;
 // set lit leds back to none lit
 bitset = 0;
 // first player begins
 first = true;
 // not yet finished (of course)
 finished = false;
 // start new game
 Serial.println("Starting new game:");
 Serial.println("");
}

void determineIfWonOrDraw(){
  // test all winning cases
  // i.e. 3 lights diagonal or in row/column
 // case 1: three lights in one column
 byte andcol = (row1 & row2 & row3);
 if (andcol !=0){
   // B10000000 || B00100000 || B00001000
   if ((andcol == 0x80) || (andcol == 0x20) || (andcol == 0x08)){
     winner = p1;
   }
   // B01000000 || B00010000 || B00000100
   if (andcol == 0x40 || andcol == 0x10 || andcol == 0x04){
     winner = p2;
   }
   finished = true;
 } 
 // case 2: any row X
 // per row: if rowX & p1/p2 = 0xA8/0x54
 
 //row 1
 // B10101000 == 0xA8
 if ((row1 & p1) == 0xA8){
   winner = p1;
   finished = true;
 }
 // B01010100 == 0x54
  if ((row1 & p2) == 0x54){
   winner = p2;
   finished = true;
 }
 //row 2
  if ((row2 & p1) == 0xA8){
   winner = p1;
   finished = true;
 }
  if ((row2 & p2) == 0x54){
   winner = p2;
   finished = true;
 }
 //row 3
  if ((row3 & p1) == 0xA8){
   winner = p1;
   finished = true;
 }
  if ((row3 & p2) == 0x54){
   winner = p2;
   finished = true;
 }
 // case 3: diagonal
 // first: ((row1 & 0xCO) | (row2 & 0x3O) | (row3 & 0x0C)) & 0xFC = 0xA8/0x54
 // second:((row1 & 0x0C) | (row2 & 0x3O) | (row3 & 0xC0)) & 0xFC = 0xA8/0x54
 byte first = (((row1 & 0xC0) | (row2 & 0x30) | (row3 & 0x0C)) & 0xFC);
 byte second = (((row1 & 0x0C) | (row2 & 0x30) | (row3 & 0xC0)) & 0xFC);
 // if no one won and all LEDs are set => draw
 if(first == 0xA8 || second == 0xA8){
   winner = p1;
   finished = true;
 }
  if(first == 0x54 || second == 0x54){
   winner = p2;
   finished = true;
 }
 // increment respective counter
 if(winner == p1){
   c1++;
 } else if (winner == p2){
   c2++;
 }
 if(bitset == 511){
   finished = true;
 }
// if finished trigger reaction and reset everything.
  if(finished){
    // this has to be checked only after finished is true because
    // otherwise it would be incremented each time it is called (standard-value of winner is draw)
    if(winner == 0x00){
      d++;
    }
    printWinner();
    flash();
    resetEverything();
  }
}

void loop() { 
 // initialise mask
 byte mask;
 // used to check if player switched (i.e. a valid input has been made via remote)
 bool playerswitch = first;
 // switch mask to match currently active player
 if (first){
   mask = p1;
 } else {
   mask = p2;
 }
 //Serial.println("203");
 //delay(50);
 
 if (irrecv.decode(&results)){
  //Serial.println(results.value, HEX);
  switch(results.value){

  case r1:
    if (!(bitset & 0x0100)){ 
     //if(!set[0]){
       row1 = row1 | (mask & B11000000);
      // set[0] = 1;
       bitset += 256;
       first = !first;
     }  
     break;
     
  case r2:
    if (!(bitset & 0x0080)){
       row1 = row1 | (mask & B00110000);
       bitset += 128;
       first = !first;
     }
     break; 
 
  case r3:
  if (!(bitset & 0x0040)){
       row1 = row1 | (mask & B00001100);
       bitset += 64;
       first = !first;
     } 
    break;
    
  case r4:
  if (!(bitset & 0x0020)){
       row2 = row2 | (mask & B11000000);
       bitset += 32;
       first = !first;    
     } 
    break;
  case r5:
  if (!(bitset & 0x0010)){
       row2 = row2 | (mask & B00110000);
       bitset += 16;
       first = !first;
     }
     
    break; 
  case r6:
  if (!(bitset & 0x0008)){
       row2 = row2 | (mask & B00001100);
       bitset += 8;
       first = !first;
     }
         
    break;
  case r7:
      if (!(bitset & 0x0004)){
       row3 = row3 | (mask & B11000000);
       bitset += 4;
       first = !first;
     } 
    break;
  case r8:
  if (!(bitset & 0x0002)){
       row3 = row3 | (mask & B00110000);
       bitset += 2;
       first = !first; 
     }
    break;
  case r9:
  if (!(bitset & 0x0001)){
       row3 = row3 | (mask & B00001100);
       bitset += 1;
       first = !first;
     }    
    break;  
  default:
    break;
  }
  irrecv.resume();
 }
  // if playerswitch value differs from current first value then an valid input has been made.
  if (playerswitch != first){
    // only necessary to check if state changed  
    determineIfWonOrDraw();
  }
  // turn LEDs on
  turnOnLeds();
}
