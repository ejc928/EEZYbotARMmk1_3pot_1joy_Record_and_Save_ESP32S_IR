#include <ESP32Servo.h> //Servo header file
#include <IRremote.h>

// Potentiometer and Joystick
int potpin1 = 26;  //Servo 1
int potpin2 = 27;  //Servo 2
int potpin3 = 14;  //Servo 3
int joy4 = 25;     //Servo 4

//Infrared Receiver
int IRPIN = 32;

//Last Code Received
unsigned long lastCode;

//Declare object for 5 Servo Motors  
Servo Servo_0;
Servo Servo_1;
Servo Servo_3;
Servo Gripper;

//Global Variable Declaration 
int S0_pos, S1_pos, S3_pos, G_pos; 
int P_S0_pos, P_S1_pos, P_S3_pos, P_G_pos;
int C_S0_pos, C_S1_pos, C_S3_pos, C_G_pos;
int POT_0, POT_1, POT_3 ,POT_4;

//statement
bool Records = false;
bool Plays = false;
bool Resets = false;
bool Manuals = false;
bool Stops = false;

int saved_data[700]; //Array for saving recorded data

int array_index = 0;
char incoming = 0;

int action_pos;
int action_servo;

void setup() {
  Serial.begin(9600); //Serial Monitor for Debugging

  IrReceiver.begin(IRPIN, ENABLE_LED_FEEDBACK);

  //Declare the pins to which the Servo Motors are connected to 
  Servo_0.attach(2);
  Servo_1.attach(4);
  Servo_3.attach(16);
  Gripper.attach(17);

  //Write the servo motors to initial position 
  Servo_0.write(0);
  Servo_1.write(0);
  Servo_3.write(0);
  Gripper.write(0);

  Serial.println("READY"); //Instruct the user 
}

void loop() {
    //IR Receiver
  if (IrReceiver.decode()) {
  Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX); // Print "old" raw data
  {
    if (IrReceiver.decodedIRData.decodedRawData == 0) {
      //REPEAT LAST CODE
      Serial.println("Last Code initialized");
      IrReceiver.decodedIRData.decodedRawData = lastCode;
    }
    if (IrReceiver.decodedIRData.decodedRawData == 0xE718FF00) {
      Serial.println("Manual Mode");
      lastCode = IrReceiver.decodedIRData.decodedRawData;
      Records = false;
      Plays = false;
      Stops = false;
      Manuals = true;
      Resets = false;
    }
    if (IrReceiver.decodedIRData.decodedRawData == 0xF708FF00) {
      Serial.println("Record Mode");
      lastCode = IrReceiver.decodedIRData.decodedRawData;
      Records = true;
      Plays = false;
      Stops = false;
      Manuals = false;
      Resets = false;
    }
    if (IrReceiver.decodedIRData.decodedRawData == 0xA55AFF00) {
      Serial.println("Play Mode");
      lastCode = IrReceiver.decodedIRData.decodedRawData;
      Records = false;
      Plays = true;
      Stops = false;
      Manuals = false;
      Resets = false;
    }
    if (IrReceiver.decodedIRData.decodedRawData == 0xE31CFF00) {
      Serial.println("RESET");
      lastCode = IrReceiver.decodedIRData.decodedRawData;
      Records = false;
      Plays = false;
      Stops = false;
      Manuals = false;
      Resets = true;
    }
    if (IrReceiver.decodedIRData.decodedRawData == 0xAD52FF00) {
      Serial.println("STOP");
      lastCode = IrReceiver.decodedIRData.decodedRawData;
      Records = false;
      Plays = false;
      Stops = true;
      Manuals = false;
      Resets = false;
    }
  }
  IrReceiver.resume();
  }

if (Manuals == true) //If user has selected Stop Mode 
Manual();

if (Records == true) //If user has selected Record mode
Record();

if (Plays == true) //If user has selected Play Mode 
Play();

if (Stops == true) //If user has selected Stop Mode 
Stop();

if (Resets == true) //If user has selected Stop Mode 
Reset();

}

void Read_POT() { //Function to read the Analog value form POT and map it to Servo value
   POT_0 = analogRead(potpin1); 
   POT_1 = analogRead(potpin2); 
   POT_3 = analogRead(potpin3); 
   POT_4 = analogRead(joy4); //Read the Analog values form all five POT
   S0_pos = ((180./4095.)*POT_0); //Map it for 1st Servo (Base motor)
   S1_pos = ((90./4095.)*POT_1); //Map it for 2nd Servo (Hip motor)
   S3_pos = ((90./4095.)*POT_3); //Map it for 4th Servo (Neck motor)
   if (POT_4 <= 3000) {
    G_pos = 60;
    } else if (POT_4 >= 3200) {
    G_pos = 0;
    }
}

void Record() { //Function to Record the movements of the Robotic Arm

Read_POT(); //Read the POT values  for 1st time

//Save it in a variable to compare it later
   P_S0_pos = S0_pos;
   P_S1_pos = S1_pos;
   P_S3_pos = S3_pos;
   P_G_pos  = G_pos;
   
Read_POT(); //Read the POT value for 2nd time
  
   if (P_S0_pos == S0_pos) //If 1st and 2nd value are same
   {
    Servo_0.write(S0_pos); //Control the servo
    
    if (C_S0_pos != S0_pos) //If the POT has been turned 
    {
      saved_data[array_index] = S0_pos + 0; //Save the new position to the array. Zero is added for zeroth motor (for understading purpose)
      array_index++; //Increase the array index 
    }
    
    C_S0_pos = S0_pos; //Saved the previous value to check if the POT has been turned 
   }

//Similarly repeat for all 5 servo Motors
   if (P_S1_pos == S1_pos)
   {
    Servo_1.write(S1_pos);
    
    if (C_S1_pos != S1_pos)
    {
      saved_data[array_index] = S1_pos + 1000; //1000 is added for 1st servo motor as differentiator 
      array_index++;
    }
    
    C_S1_pos = S1_pos;
   }

   if (P_S3_pos == S3_pos)
   {
    Servo_3.write(S3_pos); 
    
    if (C_S3_pos != S3_pos)
    {
      saved_data[array_index] = S3_pos + 3000; //3000 is added for 3rd servo motor as differentiater 
      array_index++;
    }
    
    C_S3_pos = S3_pos;   
   }

   if (P_G_pos == G_pos)
   {
    Gripper.write(G_pos);
    
    if (C_G_pos != G_pos)
    {
      saved_data[array_index] = G_pos + 4000; //4000 is added for 4th servo motor as differentiator 
      array_index++;
    }
    
    C_G_pos = G_pos;
   }
   
  //Print the value for debugging 
  Serial.print(S0_pos);  
  Serial.print("  "); 
  Serial.print(S1_pos); 
  Serial.print("  "); 
  Serial.print(S3_pos); 
  Serial.print("  "); 
  Serial.println(G_pos);
  Serial.print ("Index = "); 
  Serial.println (array_index); 
}

void Play() //Functon to play the recorded movements on the Robotic ARM
{
  for (int Play_action=0; Play_action<array_index; Play_action++) //Navigate through every saved element in the array 
  {
    action_servo = saved_data[Play_action] / 1000; //The fist character of the array element is split for knowing the servo number
    action_pos = saved_data[Play_action] % 1000; //The last three characters of the array element is split to know the servo postion 

    switch(action_servo){ //Check which servo motor should be controlled 
      case 0: //If zeroth motor
        Servo_0.write(action_pos);
      break;

      case 1://If 1st motor
        Servo_1.write(action_pos);
      break;

      case 3://If 3rd motor
        Servo_3.write(action_pos);
      break;

      case 4://If 4th motor
        Gripper.write(action_pos);
      break;
    }
    delay(50);
  }
}

void Reset() {
  Servo_0.write(0);
  Servo_1.write(0);
  Servo_3.write(0);
  Gripper.write(0);
  array_index = 0;
}

void Manual() {
  POT_0 = analogRead(potpin1); 
  POT_1 = analogRead(potpin2); 
  POT_3 = analogRead(potpin3); 
  POT_4 = analogRead(joy4); //Read the Analog values form all five POT
  S0_pos = ((180./4095.)*POT_0); //Map it for 1st Servo (Base motor)
  Servo_0.write(S0_pos);
  S1_pos = ((90./4095.)*POT_1); //Map it for 2nd Servo (Hip motor)
  Servo_1.write(S1_pos);
  S3_pos = ((90./4095.)*POT_3); //Map it for 4th Servo (Neck motor)
  Servo_3.write(S3_pos);
  if (POT_4 <= 3000) {
    G_pos = 60;
    Gripper.write(G_pos);
  } else if (POT_4 >= 3200) {
    G_pos = 0;
    Gripper.write(G_pos);
  }
}

void Stop() {
  //
}
