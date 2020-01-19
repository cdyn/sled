#include <SerialCommands.h>

/*
  Serial LED Control
  PWM PNP Transistor LED Control From the usb serial monitor has set and fade features
  * uses Serial Commands library https://github.com/ppedro74/Arduino-SerialCommands

  jrunnells@customdyn.com
*/

//LED PNP PINS
int gPin = 9;
int rPin = 10;
int bPin = 13;

//RGB and brightness values, defaualt pinkish white
int red = 255;
int green = 100;
int blue = 50;

//MODE -1 == off, 0 == rgb, 1 == fade
int mode = 0;

char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");


//This is the default handler, and gets called when no other command matches. 
void cmd_unrecognized(SerialCommands* sender, const char* cmd) {
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}

void cmd_rgba (SerialCommands* sender) {
  //R
  char* r_str = sender->Next();
  if (r_str == NULL){
    sender->GetSerial()->println("ERROR: EXPECTED R");
    return;
  } int r = atoi(r_str);
  //G
  char* g_str = sender->Next();
  if (g_str == NULL){
    sender->GetSerial()->println("ERROR: EXPECTED G");
    return;
  } int g = atoi(g_str);
  //B
  char* b_str = sender->Next();
  if (b_str == NULL){
    sender->GetSerial()->println("ERROR: EXPECTED B");
    return;
  } int b = atoi(b_str);
  //A
  char* a_str = sender->Next();
  if (a_str == NULL){
    sender->GetSerial()->println("ERROR: EXPECTED A");
    return;
  } int a = atoi(a_str);
  if(sender->Next() != NULL){
    sender->GetSerial()->println("ERROR: TOO MANY ARGS");
    return;
  }
  //bound input 0 <= x <= 255
  if(r < 0){r=0;} else if(r > 255){r=255;}
  if(g < 0){g=0;} else if(g > 255){g=255;}
  if(b < 0){b=0;} else if(b > 255){b=255;}
  if(a < 0){a=0;} else if(a > 255){a=255;}
  //notify
  sender->GetSerial()->print("Setting (R,G,B,A): (");
  sender->GetSerial()->print(r);
  sender->GetSerial()->print(",");
  sender->GetSerial()->print(g);
  sender->GetSerial()->print(",");
  sender->GetSerial()->print(b);
  sender->GetSerial()->print(",");
  sender->GetSerial()->print(a);
  sender->GetSerial()->println(")");
  //alpha adjust
  red = (r*a)/255;
  green = (g*a)/255;
  blue = (b*a)/255;
  //set mode and exec
  mode = 0;
  set_rgb(red, green, blue);
}

void cmd_off (SerialCommands* sender) {
  if(sender->Next() != NULL){
    sender->GetSerial()->println("ERROR: TOO MANY ARGS");
    return;
  }
  //toggle on/off
  if(mode == -1){
    mode = 0;
    sender->GetSerial()->println("Turning ON");
    set_rgb(red,green,blue);
  } else {
    sender->GetSerial()->println("Turning OFF");
    mode = -1;
  }
}

void cmd_fade(SerialCommands* sender) {
  if(mode == 1){//prohibit stack trace overflow from recursive function call since fade() can also ReadSerial()
    mode == -1;
    off();
    sender->GetSerial()->println("Fade running, turning OFF");
    return;
  }
  //R
  char* r_str = sender->Next();
  if (r_str == NULL){
    sender->GetSerial()->println("ERROR: EXPECTED R");
    return;
  } int r = atoi(r_str);
  //G
  char* g_str = sender->Next();
  if (g_str == NULL){
    sender->GetSerial()->println("ERROR: EXPECTED G");
    return;
  } int g = atoi(g_str);
  //B
  char* b_str = sender->Next();
  if (b_str == NULL){
    sender->GetSerial()->println("ERROR: EXPECTED B");
    return;
  } int b = atoi(b_str);
  //A
  char* a_str = sender->Next();
  if (a_str == NULL){
    sender->GetSerial()->println("ERROR: EXPECTED A");
    return;
  } int a = atoi(a_str);
  //T
  char* t_str = sender->Next();
  if (t_str == NULL){
    sender->GetSerial()->println("ERROR: EXPECTED T");
    return;
  } unsigned long t = strtoul(t_str, NULL, 10);
  if(sender->Next() != NULL){
    sender->GetSerial()->println("ERROR: TOO MANY ARGS");
    return;
  }
  //bound input 0 <= x <= 255
  if(r < 0){r=0;} else if(r > 255){r=255;}
  if(g < 0){g=0;} else if(g > 255){g=255;}
  if(b < 0){b=0;} else if(b > 255){b=255;}
  if(a < 0){a=0;} else if(a > 255){a=255;}
  //notify
  sender->GetSerial()->print("Fading to (R,G,B,A): (");
  sender->GetSerial()->print(r);
  sender->GetSerial()->print(",");
  sender->GetSerial()->print(g);
  sender->GetSerial()->print(",");
  sender->GetSerial()->print(b);
  sender->GetSerial()->print(",");
  sender->GetSerial()->print(a);
  sender->GetSerial()->print(") in(ms):");
  sender->GetSerial()->println(t);
  //alpha adjust
  r = (r*a)/255;
  g = (g*a)/255;
  b = (b*a)/255;
  //set mode and exec
  mode = 1;
  fade(r,g,b,t);
}

void set_rgb(int r, int g, int b){
  analogWrite(rPin, r);
  analogWrite(gPin, g);
  analogWrite(bPin, b);
}

void off(){
  set_rgb(0,0,0);
}

void fade(int r, int g, int b, unsigned long t){//same as set_rbg(r,b,g) with a time component 't' in milliseconds
  unsigned long mark = millis();
  unsigned long lastT = mark; //Last tick
  unsigned long stepD = mark % t; //Step Duration
  int rSpan = red - r;
  int gSpan = green - g;
  int bSpan = blue - b;
  while (mode == 1){
    mark = millis();
    lastT = (mark / t) * t;
    stepD = mark % t;
    if((lastT / t) % 2 == 1){ //stepping up on odd ticks
      set_rgb(red - (rSpan * stepD) / t, green - (gSpan * stepD) / t, blue - (bSpan * stepD) / t);
    } else if((lastT / t) % 2 == 0){ //stepping down on even ticks
      set_rgb(r + (rSpan * stepD) / t, g + (gSpan * stepD) / t, b + (bSpan * stepD) / t);
    }
    serial_commands_.ReadSerial();
  }
}

SerialCommand cmd_rgba_("RGBA", cmd_rgba);
SerialCommand cmd_off_("OFF", cmd_off);
SerialCommand cmd_fade_("FADE", cmd_fade);

void setup() {
  Serial.begin(9600);
  analogWrite(rPin, red);
  analogWrite(gPin, green);
  analogWrite(bPin, blue);

  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_rgba_);
  serial_commands_.AddCommand(&cmd_off_);
  serial_commands_.AddCommand(&cmd_fade_);
  
  Serial.println("Ready!");
}

void loop() {
  serial_commands_.ReadSerial();  
}

/*// fade in from min to max in increments of 5 points:
  for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
    // sets the value (range from 0 to 255):
    analogWrite(gPin, fadeValue);
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }*/
