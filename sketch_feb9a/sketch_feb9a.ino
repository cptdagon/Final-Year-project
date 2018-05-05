#include "LTask.h"
#include "vmcell.h"
#include <LGSM.h>

// Cellular Network information refresh delay
const int loop_delay_ms = 2500;
/* 
  Wrapper to the low-level function vm_cell_open()
  Initialize the Cell register on LinkIt ONE
*/
static boolean cell_open(void *userData) {
  *(VMINT *)userData = vm_cell_open();
  return TRUE;
}
/*
  Wrapper to the low-level function vm_cell_get_cur_cell_info()
  Get the current cell info. Result is stored into a struct
*/
static boolean get_current_cell_info(void *userData) {
  *(vm_cell_info_struct **)userData = vm_cell_get_cur_cell_info();
  return TRUE;
}

void init_cell_register() {
  VMINT cell_open_result;
  boolean stop = FALSE;
  
  Serial.print("Initializing...  ");
  LTask.remoteCall(&cell_open, (void *)&cell_open_result);
  
  while(!stop) {
    switch(cell_open_result) {
      case VM_CELL_OPEN_SUCCESS:
      Serial.println("Done.");
      stop = TRUE;
      break;

      case VM_CELL_OPEN_ALREADY_OPEN:
      Serial.println("Done. Cell register was already open.");
      stop = TRUE;
      break;

      case VM_CELL_OPEN_RES_LESS:
      Serial.println("Error. Lack of resources while opening cell register. Retrying in 5 seconds.");
      delay(5000);      
      break;

      case VM_CELL_OPEN_REG_ERROR:
      Serial.println("Error while opening cell register. Retrying in 5 seconds.");
      delay(5000);      
      break;

      default:
      Serial.println("Unknown error while opening cell register. Retrying in 5 seconds.");
      delay(5000);
      
    }
  }
}

void print_cell_info(vm_cell_info_struct* cell_ptr) {
  if (cell_ptr == NULL) {
    Serial.println("No cell info available");
  }
  else {
    char buffer[20];
    int l = cell_ptr->rxlev;
    sprintf(buffer, "Signal Level: %d", l);
    Serial.println(buffer);
    Serial1.print(buffer);
  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  //pinMode(2, INPUT);
  //pinMode(3, OUTPUT);
  while(!LSMS.ready())
  {
    Serial.println("SIM Initialising");
    delay(1000);
  } 
  init_cell_register();
  delay(5000);
  //digitalWrite(3, LOW);
  Serial.println("Ready"); 
}
/*
Check for a new SMS Message
if new message, send contents to myRio by UART
*/
void loop() {
  if(LSMS.available())             // Check if there is new SMS
  { 
    char buf[20];
    int v;
    Serial.println("There is new message.");
    LSMS.remoteNumber(buf, 20);    // display Number part
    Serial.print("Number:");
    Serial.println(buf);
    Serial.print("Content:");      // display Content part
    while(true)
    { 
      v = LSMS.read();
      if(v < 0)
        break;
      Serial.print((char)v);
      Serial1.print((char)v);
    } 
    Serial.println();
    LSMS.flush();                  // delete message
  } 
  delay(loop_delay_ms);
  //if(digitalRead(2) == HIGH
/*
Check UART for incoming data
if data - send SMS
*/
  if(Serial1.available())//
  {
    //int z = 0;
    Serial.println("Reading UART");
    //digitalWrite(3, HIGH);
    LSMS.beginSMS("");
    //while(!Serial1.available())
    //{
      //delay(1000);
      //z = z+1;
      //if(z = 20)
      //{
       // break;  
      //}
      //Serial.println(z);
    //}
    int k;
    Serial.println("Rio Says");
    while(true)
    {
      k = Serial1.read();
      if (k<0)
        break;
      Serial.print((char)k);
      LSMS.print((char)k);
    }
    Serial.println();
    if(LSMS.endSMS())
    { 
      Serial.println("SMS is sent"); 
      Serial1.print("Log Action: SMS Sent");
    } 
    else    
    { 
      Serial.println("SMS is not sent");
      Serial1.print("Log Action: SMS Not Sent");
    }     
    //digitalWrite(3, LOW);
    delay(loop_delay_ms);
  }
  
/*
Get Current Signal Strength and Cell Info
print result to PC Console Window and Send result to myRio by UART
*/
  
  delay(loop_delay_ms);
  vm_cell_info_struct *current_cell_ptr;
  LTask.remoteCall(&get_current_cell_info, (void *)&current_cell_ptr);
  Serial.print("Current cell info    :");
  print_cell_info(current_cell_ptr);
  Serial.println();
  delay(loop_delay_ms);
}
