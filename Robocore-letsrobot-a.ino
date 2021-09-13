


// John L. Sokol    2/2/2018 
// https://www.johnsokol.com  
// GPL V3 
//  https://www.youtube.com/watch?v=Bs7hA_bNzwk  
//  https://www.youtube.com/watch?v=ByQOfl8R5qg  Demo of this in action
//
//  Specifcally using the Horizon Hobby - Ruckus Monster Truck but 
//  any RC that takes seperate steering and thottle over PWM signals.
//  
// Currently this is setup for the Roboterra RoboCore v1.4 board, but adapting this to the any ardino is trivial. 
//



#include <Servo.h>

#define SERIAL_PROMPT "RT>"
#define CR 13
#define LF 10
#define MAX_COMMAND_PARAMS 3   // limit the number of parameters accompanying a command to 3
#define MAX_STRING_LENGTH 50

int strim = 90;
int ttrim = 93;
int TURN_AMOUNT = 30;
int TROT_AMOUNT = 20;


unsigned long previousMillis = 0;
const long sInterval = 5000; 
const long tInterval = 150; 

unsigned long currentMillis = 0;


boolean echo_on = true;
char command_buffer[MAX_STRING_LENGTH]; // buffer used to store incoming characters on the serial port


Servo Steering;  // create servo object to control a servo
Servo Throttle;  // create servo object to control a servo

// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

typedef struct {
  char name[MAX_STRING_LENGTH];  // holds the command
  char alias[4]; // pin alias 
  unsigned int params[MAX_COMMAND_PARAMS]; // holds each parameter for the command
  byte no_of_parameters;  // no of parameters included with command
  boolean valid_pin;    // whether the pin included as a parameter is valid
} Arduino_command;

Arduino_command command;




// Display a prompt on the remote terminal
void Prompt()
{
if(echo_on)  
  Serial.print(SERIAL_PROMPT);

}


// ///////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////// S E T U P  /////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

void setup() {
  
  Serial.begin(9600); // USB serial port is used for comms and program upload

  

  // wait for serial port to be available before continuing. 
  while(!Serial);
  delay(1000);
  // Display a prompt
  Prompt();

  Steering.attach(9);  // attaches the RoboCore Servo_B
  Throttle.attach(10);  // attaches the RoboCore Servo_A
  Steering.write(strim); 
  Throttle.write(ttrim); 
}



void wiggle_servos(){
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    Steering.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    Steering.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }


  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    Throttle.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    Throttle.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }

  Steering.write(strim); 
  Throttle.write(ttrim); 
}

// ///////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////// L O O P ////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

void loop() {
  
boolean enter_pressed;

 currentMillis = millis();


if( previousMillis + sInterval < currentMillis){
  Steering.write(strim); 
  
}

if( previousMillis + tInterval < currentMillis){
  Throttle.write(ttrim); 
}


echo_on=true;
  {
    if(Serial.available())
    {
  // read each incoming character   
      enter_pressed = ReadInput();
      if(enter_pressed == true)
      {
     if(!(command_buffer[0] == '\0'))
    {
    // user has pressed enter so extract the command
        if(ParseCommand() == true)
    // process the command
         ExecuteCommand();
    // display a prompt 
    }

    Prompt();
      }
    }  
  } 
}

// Read each character and echo to serial port
boolean  ReadInput()
{
  static int i=0; // index keeps track of the position in the command buffer
  char character_read;
  boolean enter;

// clear the display and show a prompt when new characters are received after a user presses enter on the remote terminal, i.e. the first character of a new command is received


character_read = Serial.read();


  if(i < MAX_STRING_LENGTH)
  {
    switch(character_read)
    {
  case '\r':
  case '\n': 
  // terminate the command buffer if a CR or LF character has been received at the end of a command from the terminal
  command_buffer[i] = '\0';
  if(echo_on)
  {
   Serial.write('\r'); // send a CR LF pair back to the terminal
   Serial.write('\n');
  }
  #ifdef LCD_USED
  lcd.setCursor(0,1);
  cursor_position = LCD_prompt_length;
  #endif
  enter=true; // User has pressed enter or remote software has completed transmission of command by sending the terminating CR or LF
  i=0; // // Reset the index into the command buffer array. New incoming data stored at position 0,
    break;
  
    case 8: 
    case 127:   // backspace - send a backspace character to the remote terminal
    if(i>0)
   {
    i--;     //  move the command buffer index back 1 character
      Serial.write(8);
   }
    
#ifdef LCD_USED
// wipe the last character on the display
      if((cursor_position>0) && (cursor_position > LCD_prompt_length))
        cursor_position--;
      lcd.setCursor(cursor_position,0);
      lcd.print(" ");
      lcd.setCursor(cursor_position,0);
#endif
      enter=false;
      break;
    
    default:
      command_buffer[i] = character_read;
    if(echo_on)
    Serial.write(character_read);  // echo input to remote terminal
#ifdef LCD_USED
      if(i < lcd_chars-1)
      {
        lcd.write(character_read);
        cursor_position++;
      }
#endif
      i++;  // increment the index in the command buffer
      enter=false;
      break;
    }
  }

  else
  {
   enter = false;
   i=0;
   Message("\nToo Many Chars");

   Prompt(); 
  }
  return(enter);
}


// Extract the command and stuff it into a structure with its parameters
boolean ParseCommand()
{
  char delims[] = " "; // delimiter character used between incoming command and its parameters
 
  int i; 
  char *result = NULL;

  for(i=0; i<MAX_COMMAND_PARAMS;i++)
   command.params[i] = 0;

  command.no_of_parameters = 0;
  command.valid_pin = false;
  

  i=0;

  result = strupr(strtok( command_buffer, delims ));  // extract the first section of string in the buffer. This is the command
  
  
  strcpy(command.name,result);  // store the command in a structure

  while( (result = strtok( NULL, delims ))!= NULL ) // continue to extract the parameters of the command
  { // 1
    
  result=strupr(result);
  if(i>MAX_COMMAND_PARAMS-1)
  {
    Message("Too Many Params");
    return(false);
  }
    else
     command.params[i]=atoi(result);
    i++;
    command.no_of_parameters = i;
  } // 1 

  return(true);
}
// Process the command


void steer(int pos){

 if(pos == 0)Steering.write(strim);
 if(pos < 0)Steering.write(strim - TURN_AMOUNT);
 if(pos > 0)Steering.write(strim + TURN_AMOUNT);

}


void ExecuteCommand()
{
  int x;
  unsigned int y;
  char string[100];
  int val;
 // float temperature;
  //char temp_string[7];

  static int pos = 0;

previousMillis = currentMillis; // Keep track since the last time we recieved a command and stop car if ignored. 

  if(strcmp(command.name,"W") == 0)
  {

    if(command.no_of_parameters == 0)
    {
      wiggle_servos(); 
    } else

    if(command.no_of_parameters != 2)
    {
     Message("Wrong Num Params"); 
    } 
    else
    {

      if(command.params[1] >= 0 || command.params[1] <= 255)
      {
       Steering.write(command.params[0]); 
       Throttle.write(command.params[1]); 
       Message("Acknowledge");
      }

      

    }
  }



  if(strcmp(command.name,"STOP") == 0)
  {
//  Steering.write(strim); 
  Throttle.write(ttrim); 
  }
  
  if(strcmp(command.name,"S") == 0)
  {
  Steering.write(strim); 
  Throttle.write(ttrim); 
  }

  if(strcmp(command.name,"F") == 0)
  {
  //Steering.write(strim); 
   Throttle.write(ttrim - TROT_AMOUNT); 
  }

  if(strcmp(command.name,"B") == 0)
  {
 // Steering.write(strim); 
   Throttle.write(ttrim + TROT_AMOUNT); 
  }

  if(strcmp(command.name,"R") == 0)
  {
    if(pos < 1) pos++;
    
    steer(pos);
  //Throttle.write(ttrim); 
  }

  if(strcmp(command.name,"L") == 0)
  {
    if(pos > -1) pos--;
  steer(pos);
  //Throttle.write(ttrim); 
  }
  

  // turn on echoing of input characters to terminal. This also enables the prompt on the terminal
  else if(strcmp(command.name,"ECHOON") == 0)
  {
   echo_on = true;
  }
  
  // turn off echoing of characters to remote terminal
  else if(strcmp(command.name,"ECHOOFF") == 0)
  {
   echo_on = false;
  }

}
  
// Display a message on the LCD and on the remote terminal
void Message(char *message)
{
Serial.println(message);
}
