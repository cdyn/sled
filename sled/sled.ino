#include <SerialCommands.h>

/*
  Serial LED Control
  PWM PNP Transistor LED Control From the usb serial monitor has set and fade features
  * uses Serial Commands library https://github.com/ppedro74/Arduino-SerialCommands

  jrunnells@customdyn.com
*/

//LED PINS
int gPin = 9;
int rPin = 10;
int bPin = 13;

//RGB and brightness values, defaualt pinkish white
int red = 255;
int green = 100;
int blue = 50;

int mode = 0; //MODE -1 == off, 0 == rgb, 1 == fade
unsigned long tick = 30; //Step Duration
unsigned long lastT = 0;

char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");

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

//This is the default handler, and gets called when no other command matches. 
void cmd_unrecognized(SerialCommands* sender, const char* cmd) {
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}

void cmd_rgba (SerialCommands* sender) {
  //Note: Every call to Next moves the pointer to next parameter

  char* pwm_str = sender->Next();
  if (pwm_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_PWM");
    return;
  }
  int pwm = atoi(pwm_str);

  char* led_str;
  while ((led_str = sender->Next()) != NULL)
  {
    if (set_led(led_str, pwm))
    {
      sender->GetSerial()->print("LED_STATUS ");
      sender->GetSerial()->print(led_str);
      sender->GetSerial()->print(" ");
      sender->GetSerial()->println(pwm);
    }
    else
    {
      sender->GetSerial()->print("ERROR ");
      sender->GetSerial()->println(led_str);
    }
  }
}

void cmd_off (SerialCommands* sender) {
  //Note: Every call to Next moves the pointer to next parameter
  if(mode == -1){
    mode = 0;
    set_rgb(red,green,blue);
  } else {
    mode = -1;
  }
}

void cmd_fade(SerialCommands* sender) {
  //Note: Every call to Next moves the pointer to next parameter

  char* pwm_str = sender->Next();
  if (pwm_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_PWM");
    return;
  }
  int pwm = atoi(pwm_str);

  char* led_str;
  while ((led_str = sender->Next()) != NULL)
  {
    if (set_led(led_str, pwm))
    {
      sender->GetSerial()->print("LED_STATUS ");
      sender->GetSerial()->print(led_str);
      sender->GetSerial()->print(" ");
      sender->GetSerial()->println(pwm);
    }
    else
    {
      sender->GetSerial()->print("ERROR ");
      sender->GetSerial()->println(led_str);
    }
  }
}

void set_rgb(int r, int g, int b){
  analogWrite(rPin, r);
  analogWrite(gPin, g);
  analogWrite(bPin, b);
}

void rgba(int r, int g, int b, int a) {//set rgb with 'a' represeneting 'alpha' or brightness as a fraction of 255
  //bound input 0 <= x <= 255
  if(r < 0){r=0;} else if(r > 255){r=255;}
  if(g < 0){g=0;} else if(g > 255){g=255;}
  if(b < 0){b=0;} else if(b > 255){b=255;}
  if(a < 0){a=0;} else if(a > 255){a=255;}
  //preform alpha calculation
  red = (r*a)/255;
  green = (g*a)/255;
  blue = (b*a)/255;
  set(red, green ,blue);
}

void off(){
  set_rgb(0,0,0);
}

void fade(int r, int g, int b, int a, int t){
  while (mode == 1) {
    
    for (int fadeValue = 0; fadeValue <= 255; fadeValue +=5){
      analog
      delay(t);
    }
    
    for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 5){
      
    }
    
  }
}

void loop() {
  if(mode == -1){
    off();
  } else if(mode == 0){
    set_rgb(red,green,blue);
  } else if(mode == 1){//note fade diverts the main loop
    //fade(); 
  }
  serial_commands_.ReadSerial();
  
  /*// fade in from min to max in increments of 5 points:
  for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
    // sets the value (range from 0 to 255):
    analogWrite(gPin, fadeValue);
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }*/
}
