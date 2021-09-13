

// John L. Sokol    9/12/2021
// https://www.johnsokol.com  
// GPL V3 
//  https://youtu.be/6ZdH2ZGgzts   Demo of this in action






//www.elegoo.com

//    Left motor truth table
//  ENA         IN1               IN2         Description  
//  LOW   Not Applicable    Not Applicable    Motor is off
//  HIGH        LOW               LOW         Motor is stopped (brakes)
//  HIGH        HIGH              LOW         Motor is on and turning forwards
//  HIGH        LOW               HIGH        Motor is on and turning backwards
//  HIGH        HIGH              HIGH        Motor is stopped (brakes)

//    Right motor truth table
//  ENB         IN3             IN4         Description  
//  LOW   Not Applicable   Not Applicable   Motor is off
//  HIGH        LOW             LOW         Motor is stopped (brakes)
//  HIGH        LOW             HIGH        Motor is on and turning forwards
//  HIGH        HIGH            LOW         Motor is on and turning backwards
//  HIGH        HIGH            HIGH        Motor is stopped (brakes)  

//    The direction of the car's movement
//  Left motor    Right motor     Description
//  stop(off)     stop(off)       Car is stopped
//  forward       forward         Car is running forwards
//  forward       backward        Car is turning right
//  backward      forward         Car is turning left
//  backward      backward        Car is running backwards

//define the L298n IO pin
#define ENB 5
#define IN1 7
#define IN2 8
#define IN3 9
#define IN4 11
#define ENA 6


#define RMOTOR 1
#define LMOTOR 2
#define SPEED 120

void xloop() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);  //go forward
  delay(1000);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);   //stop
  delay(1000);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);   //go back
  delay(1000);
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);  //stop
  delay(1000);
}



float Rinertia = 0;
float Linertia = 0; 

int motorSpeed(int m,int speed){


if(speed == 0){
  Rinertia = 0;
  Linertia = 0;
//  Message("\n zip");
  return 0;
}

   if(m == LMOTOR){
      Linertia += speed;
     motorUpdate(m,Linertia);
  }else{
      Rinertia += speed;
      motorUpdate(m,Rinertia);
  }
 
 
}

void motorUpdate(int m, int speed){
int en = ENA;  
int ma = IN2, mb = IN1;

  //  Serial.print("Motor ");  Serial.print(m);    Serial.print(" , ");     Serial.println(speed);

 // speed = Simulate_inertia(m,speed);

  if(m == LMOTOR){
    en = ENB;
    ma = IN3;
    mb = IN4;
  }
  
  if( abs(speed) > 255 ){      
     speed = 255;
  }
  
  if(speed == 0){
    analogWrite(en, 0 ); 
    digitalWrite(en, LOW);
    digitalWrite(ma, LOW);
    digitalWrite(mb, LOW);
  }
  else{
    analogWrite(en, abs(speed) ); 
  } 

  if(speed < 0) {
    digitalWrite(ma, HIGH);
    digitalWrite(mb, LOW);
  }else {
    digitalWrite(ma, LOW);
    digitalWrite(mb, HIGH);
  }
 
} 
 

#define SERIAL_PROMPT "RT>"
#define CR 13
#define LF 10
#define MAX_COMMAND_PARAMS 3   // limit the number of parameters accompanying a command to 3
#define MAX_STRING_LENGTH 50



unsigned long previousMillis = 0;
unsigned long LoopMillis = 0;
unsigned long currentMillis = 0;

const long sInterval = 5000; 
const long tInterval = 5000; 




boolean echo_on = true;
char command_buffer[MAX_STRING_LENGTH]; // buffer used to store incoming characters on the serial port




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

  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);

      motorSpeed(LMOTOR,0);
      motorSpeed(RMOTOR,0);

    LoopMillis = previousMillis =  currentMillis = millis();
 
}





// ///////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////// L O O P ////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

void loop() {
  
boolean enter_pressed;

int emill;

 LoopMillis = currentMillis;
 //delay(100);
 currentMillis = millis();

 emill = currentMillis - LoopMillis;


Rinertia = Rinertia * 0.970;
Linertia = Linertia * 0.970;

if( Rinertia != 0 || Linertia !=0){ 
     motorUpdate(LMOTOR,Linertia);
     motorUpdate(RMOTOR,Rinertia);
  }


 
//Serial.print("Rinertia ");  Serial.println(Rinertia);

if( previousMillis + sInterval < currentMillis){
      motorSpeed(LMOTOR,0);
      motorSpeed(RMOTOR,0); 
     // Message("\n sInterval");
  
}

if( previousMillis + tInterval < currentMillis){
      motorSpeed(LMOTOR,0);
      motorSpeed(RMOTOR,0);
     // Message("\n tInterval");
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
       xloop();
      //wiggle_servos(); 
    } else

    if(command.no_of_parameters != 2)
    {
     Message("Wrong Num Params"); 
    } 
    else
    {

      if(command.params[1] >= 0 || command.params[1] <= 255)
      {
             motorSpeed(LMOTOR,command.params[0]);
             motorSpeed(RMOTOR,command.params[1]);

             Message("Acknowledge");
      }

      

    }
  }



  if(strcmp(command.name,"STOP") == 0)
  {
      motorSpeed(LMOTOR,0);
      motorSpeed(RMOTOR,0);
  }
  
  if(strcmp(command.name,"S") == 0)
  {
      motorSpeed(LMOTOR,0);
      motorSpeed(RMOTOR,0);
  }

  if(strcmp(command.name,"F") == 0)
  {
      motorSpeed(LMOTOR,SPEED);
      motorSpeed(RMOTOR,SPEED);
  }

  if(strcmp(command.name,"B") == 0)
  {
      motorSpeed(LMOTOR,-SPEED);
      motorSpeed(RMOTOR,-SPEED);
  }

  if(strcmp(command.name,"R") == 0)
  {
      motorSpeed(LMOTOR,-SPEED);
      motorSpeed(RMOTOR,SPEED);
  }

  if(strcmp(command.name,"L") == 0)
  {
      motorSpeed(LMOTOR,SPEED);
      motorSpeed(RMOTOR,-SPEED); 
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
