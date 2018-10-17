#include "mbed.h"                 
#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_stack_check.h>
#include <nrk_stats.h>
//#include <string>
#include <string.h>
//#include <iostream.h>
//#include "basic_rf.h"
//#include "bmac.h"


#include <algorithm>    // std::copy
#include <vector>       // std::vector

#define MAX_RX_BUF_SIZE 70*6   //Receive buffer size

/* Fredy Monterroza
	 Ye Yang
	 Jing Qui
	 ESE 519: CompDuctor
	 main.cpp
*/



/* Left Hand = servo/leftFinger (p21, p16)
	 Middle Hand = servo2/middleFinger (p22, p17)
	 Right Hand = servo3/rightFinger (p23, p18)
*/

Serial bt(p9,p10);   //tx, rx      Bluetooth read in   //can also be USBSerial

#define MAX_RX_BUF_SIZE 70*6   //Receive buffer

char rx_buf[MAX_RX_BUF_SIZE];
void clearBuf(void);
void musicFileDecode(void);

void printChar(char *p, int size) {
    char *tmp = p;
    for(int i= 0; i < size; i++) {
        putchar(char(*(tmp+i)));
    }
}

PwmOut servo(p21);
PwmOut servo2(p22); //24 good on , p22 doesn't work on m125 mBed to drive servo, but can get the right waveform on oscilloscope
PwmOut servo3(p23);
DigitalOut leftFinger(p14); 
DigitalOut middleFinger(p15);
DigitalOut rightFinger(p16);


int i = 0;
int j = 0;
int iM = 0;
int jM = 0;
int iR = 0;
int jR = 0;

//float shiftAngleByDuty = 0.00011666;

float initialPWM = .001140;
//char keyboardIn[2];
char keyboardIn;

int keyCount = 0; //Do Re ...Ti 4% to 11% OR FIND APPROPRIATE LEFT RIGHT OR RIGHT LEFT
float keyPWMVal[12];

Timer tim;

//Task Control (Locks) Variables
bool turnPermit = 1;
bool taskFlag1 = 1;
bool taskFlag2 = 1;
bool taskFlag3 = 1;
bool taskFlag4 = 1;

//bool turnPermitM = 1;
bool taskFlag1M = 1;
bool taskFlag2M = 1;
bool taskFlag3M = 1;
bool taskFlag4M = 1;

//bool turnPermitR = 1;
bool taskFlag1R = 1;
bool taskFlag2R = 1;
bool taskFlag3R = 1;
bool taskFlag4R = 1;



// Variables to ensure timing of music and physical constraints of servos. 
int pressingTime;
int depressingTime;
int startTurningTime;
int lastTurningTime;
int endTurningTime;

int pressingTimeM;
int depressingTimeM;
int startTurningTimeM;
int lastTurningTimeM;
int endTurningTimeM;

int pressingTimeR;
int depressingTimeR;
int startTurningTimeR;
int lastTurningTimeR;
int endTurningTimeR;


NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void Task2 (void);

NRK_STK Stack3[NRK_APP_STACKSIZE];
nrk_task_type TaskThree;
void Task3 (void);

NRK_STK Stack4[NRK_APP_STACKSIZE];
nrk_task_type TaskFour;
void Task4 (void);

/*nrk_task_type RX_TASK;
NRK_STK rx_task_stack[NRK_APP_STACKSIZE];
void rx_task (void);

nrk_task_type TX_TASK;
NRK_STK tx_task_stack[NRK_APP_STACKSIZE];
void tx_task (void);



char tx_buf[RF_MAX_PAYLOAD_SIZE];
char rx_buf[RF_MAX_PAYLOAD_SIZE];


// You do not need to modify this function
struct __FILE { int handle; };
*/

void nrk_create_taskset();


int register_1 = 0;
int register_2 = 0;

//Marry Had a Little Lamb  
//#define mhallSize 26*3
#define mhallSizeM (13*3 )+ 1
#define mhallSizeR (14*3) + 1
//float mhallFile[mhallSize];
//float mhallFileM[mhallSizeM] = {12,1,0, 10,1,.5, 8,1,.5, 10,3,.5, 10,1,.5, 10,1,.5, 10,5,.5, 10,1,.5, 8,1,.5, 10,5,.5, 10,3,.5, 10,1,.5, 8,1,.5};    
//float mhallFileR[mhallSizeR] = {2,4,.5, 2,1,.5, 2,1,.5, 2,4,.5, 2,1,.5, 5,1,.5, 5,1,.5, 2,4,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,2,.5, 2,1,.5, 2,1,.5}; 	


//Jingle Bells
#define jblSize (9*3) + 1
#define jbmSize (16*3) + 1
#define jbrSize (40*3) + 1
/*
float jingleBellsL[jblSize] = {6,6,.5, 6,5,.5, 8,9,.5, 5,6,.5, 6,6,.5, 6,5,.5, 8,7,.5, 5,1,.5, 10,5,3,};
float jingleBellsM[jbmSize] = {12,3,0, 3,5,.5, 8,1,.5, 10,1,.5, 3,5,.5, 3,5,.5, 10,1,.5, 10,2,.5, 10,5,.5, 3,5,.5, 8,1,.5, 10,1,.5, 3,7,.5, 3,8,3, 8,1,.5, 10,1,3,};
float jingleBellsR[jbrSize] = {2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 5,3,.5, 2,1,.5, 3,1,.5, 3,1,.5, 3,1,.5, 3,1,.5, 3,1,.5,
															 2,1,.5, 2,1,.5, 2,1,.5, 2,3,.5, 2,2,5, 5,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 5,3,.5,
															 2,1,.5, 3,1,.5, 3,1,.5, 3,1,.5, 3,1,.5, 3,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 5,1,.5, 5,1,.5, 2,1,.5,};
*/


//Little Dummer Boy

//Heart and Soul

//Cloud Atlas Sextet

/*
float file[jblSize];// = {6,6,.5};// = {6,6,.5, 6,5,.5, 8,9,.5, 5,6,.5, 6,6,.5, 6,5,.5, 8,7,.5, 5,1,.5, 10,5,3};     
float fileM[mhallSizeM] = {12,1,0, 10,1,.5, 8,1,.5, 10,3,.5, 10,1,.5, 10,1,.5, 10,4,.5, 10,1,.5, 8,1,.5, 10,5,.5, 10,3,.5, 10,1,.5, 8,1,.5}; 
float fileR[mhallSizeR] = {2,4,.5, 2,1,.5, 2,1,.5, 2,4,.5, 2,1,.5, 5,1,.5, 5,1,.5, 2,4,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,2,.5, 2,1,.5, 2,1,.5}; 
*/	

float file[(40*3) + 1];// = {6,6,.5};// = {6,6,.5, 6,5,.5, 8,9,.5, 5,6,.5, 6,6,.5, 6,5,.5, 8,7,.5, 5,1,.5, 10,5,3};     
float fileM[(40*3) + 1];// = {12,1,0, 10,1,.5, 8,1,.5, 10,3,.5, 10,1,.5, 10,1,.5, 10,4,.5, 10,1,.5, 8,1,.5, 10,5,.5, 10,3,.5, 10,1,.5, 8,1,.5}; 
float fileR[(40*3) + 1];// = {2,4,.5, 2,1,.5, 2,1,.5, 2,4,.5, 2,1,.5, 5,1,.5, 5,1,.5, 2,4,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,2,.5, 2,1,.5, 2,1,.5};

int fileSize = 0;
int fileMSize = 0;
int fileRSize = 0;


void calibrateHand(void);
void setServos(void);
void pressKey(void);
void depressKey(void);
void setNextKey(void);

	
bool musicSelectionOver = 0;
void musicSelection(void);    // Selecting music
   	
	

int main(void)

  {
       
			/*
			servo.period(0.02); 
			servo2.period(0.02);
			servo3.period(0.02);
			servo.pulsewidth(.001140); 
			servo2.pulsewidth(.000690); 
			servo3.pulsewidth(.001420); 	
      calibrateHand();
      */
		
			
			bt.baud(115200);      //Sets up baud rate for bloetooth module communication, for USBSerial this is not necessary
		
      //Start Timer     
			tim.start();
				
			//50 Hz Period (All Servos Periods are tied together, however, the duty cycles can be independent.
			servo.period(0.02); 
			servo2.period(0.02);
			servo3.period(0.02);
				
			//Bring Servos to Middle (Position Hands)
			servo.pulsewidth(.001140); 
			servo2.pulsewidth(.000690); 
			servo3.pulsewidth(.001420); 
			
			//Set GPIO's for solenoids controlling fingers	
			leftFinger = 0;
			middleFinger = 0;
			rightFinger = 0;
			
			
			 while(1)
       {
						musicSelection();
						while(musicSelectionOver){							 
							setServos();
							pressKey();
							depressKey();
							setNextKey();				 
            }
			}
			
			
			
		
			/*
			while(1){
				
					setServos();
					pressKey();
					depressKey();
					setNextKey();
				
			}
			*/

			/*
			nrk_setup_ports();
				
			nrk_init();
		  //bmac_task_config();
			nrk_create_taskset();
			nrk_start();
			*/
			
			return 0;

  }
      


 
 
		//This task is for turning the servo
 void Task1()                 
 {
                     
		//int key;
					 
		while(1){   
							
			//This will set the left, middle and right servos to the next positions.
			setServos(); 
			
			/*
			//printf("task1a\r\n");
			servo.pulsewidth(.001020);
			servo2.pulsewidth(.000800);
			servo3.pulsewidth(.000600);
			//printf("task1b\r\n");
			*/					
			nrk_wait_until_next_period();
 
		}
}

 
 
 void Task2()                    //This task is for pressing the key
 {

      
      while(1)
      {
                
				//This handles the pressing of left/middle/right solenoids.
				pressKey();
		 
        nrk_wait_until_next_period();
                  
     
     }
    }
      
      
  void Task3()   //Task3 is for depressing the key
    {

          
        while(1)
        {
              
						depressKey();
						nrk_wait_until_next_period();
      
         }
            
     
    }
      

  
void Task4()                   //This task is for setting the pwm
 {
         
       while(1)
       {
              
					//This function checks if it's time to press the next key.
					setNextKey();                  
				  nrk_wait_until_next_period();
      }
        
         
      
      
 }
    

void nrk_create_taskset()

{
 
    nrk_task_set_entry_function( &TaskOne, Task1);
    nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
    TaskOne.prio = 1;
    TaskOne.FirstActivation = TRUE;
    TaskOne.Type = BASIC_TASK;
    TaskOne.SchType = PREEMPTIVE;
    TaskOne.period.secs = 0;
    TaskOne.period.nano_secs = 50*NANOS_PER_MS;//100*NANOS_PER_MS;//500*NANOS_PER_MS;
    TaskOne.cpu_reserve.secs = 0;
    //TaskOne.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
    TaskOne.cpu_reserve.nano_secs = 0;
    TaskOne.offset.secs = 0;
    TaskOne.offset.nano_secs= 0;
    nrk_activate_task (&TaskOne);

		nrk_task_set_entry_function( &TaskTwo, Task2);
		nrk_task_set_stk( &TaskTwo, Stack2, NRK_APP_STACKSIZE);
		TaskTwo.prio = 2;
		TaskTwo.FirstActivation = TRUE;
		TaskTwo.Type = BASIC_TASK;
		TaskTwo.SchType = PREEMPTIVE;
		TaskTwo.period.secs = 0;
		TaskTwo.period.nano_secs = 50*NANOS_PER_MS;//500*NANOS_PER_MS;
		TaskTwo.cpu_reserve.secs = 0;
		//TaskTwo.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
		TaskTwo.cpu_reserve.nano_secs = 0;
		TaskTwo.offset.secs = 0;
		TaskTwo.offset.nano_secs= 0;
		nrk_activate_task (&TaskTwo);
					
		nrk_task_set_entry_function( &TaskThree, Task3);
		nrk_task_set_stk( &TaskThree, Stack3, NRK_APP_STACKSIZE);
		TaskThree.prio = 3;
		TaskThree.FirstActivation = TRUE;
		TaskThree.Type = BASIC_TASK;
		TaskThree.SchType = PREEMPTIVE;
		TaskThree.period.secs = 0;
		TaskThree.period.nano_secs = 50*NANOS_PER_MS;//500*NANOS_PER_MS;
		TaskThree.cpu_reserve.secs = 0;
		//TaskThree.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
		TaskThree.cpu_reserve.nano_secs = 0;
		TaskThree.offset.secs = 0;
		TaskThree.offset.nano_secs= 0;
		nrk_activate_task (&TaskThree);
					
		nrk_task_set_entry_function( &TaskFour, Task4);
		nrk_task_set_stk( &TaskFour, Stack4, NRK_APP_STACKSIZE);
		TaskFour.prio = 4;
		TaskFour.FirstActivation = TRUE;
		TaskFour.Type = BASIC_TASK;
		TaskFour.SchType = PREEMPTIVE;
		TaskFour.period.secs = 0;
		TaskFour.period.nano_secs = 50*NANOS_PER_MS;//500*NANOS_PER_MS;
		TaskFour.cpu_reserve.secs = 0;
		//TaskFour.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
		TaskFour.cpu_reserve.nano_secs = 0;
		TaskFour.offset.secs = 0;
		TaskFour.offset.nano_secs= 0;
		nrk_activate_task (&TaskFour);
												
}


void calibrateHand() {
	
		//Code to 'calibrate' the servo/structure: Centered at Sol, 3.25 inches from keyboard and 2.75 inches high
		servo.period(.02);
		//servo3.pulsewidth(.0008);
		servo.pulsewidth(.001140);
		while(keyCount < 12) {
				//const char* constIn;
			
				//printf("inside while keycount\r\n");
				//cin>>keyboardIn;
				//gets(keyboardIn);
				keyboardIn = getchar();
				//putchar(keyboardIn);
				printf("printing keyboardIn: %c and count is: %d \r\n", keyboardIn, keyCount);
				servo.pulsewidth(initialPWM);
				
				//strcpy(keyboardIn, constIn);
			
				if (keyboardIn == 'w') { //increase duty by a small number
						//printf("w: %f\r\n", initialPWM);
						initialPWM += .00001;
						servo.pulsewidth(initialPWM);
						printf("w: %f\r\n", initialPWM);
				}
				if (keyboardIn == 's') {
				//if (!strcmp(keyboardIn, "d")){  
						initialPWM = initialPWM - .00001;
						servo.pulsewidth(initialPWM);
						printf("d: %f\r\n", initialPWM);
				}
				if (keyboardIn == 'x') { //save
						printf("PWM Value to be Saved: %f \r\n", initialPWM);
						keyPWMVal[keyCount] = initialPWM;
						keyCount++;
						//initialPWM = .0008; 
				}	
		}
		for (int herp = 0; herp < 12; herp++) {
				printf("%f\r\n", keyPWMVal[herp]);
		}
	
}


//Each Finger/Hand/Servo covers a range of 6 Keys
void setServos() {
	int key = 0;
	int keyM = 0;
	int keyR = 0;
	

	//Go home if you've finished each hand/finger's part.
	//if (i >= jblSize && iM >= jbmSize && iR >= jbrSize) {
	if (file[i] == 11 && fileM[iM] == 11 && fileR[iR] == 11) {	
			printf("~~~ End of Music ~~~ \r\n");
			
			//Reset Indices for New Music
			i = 0;
			iM = 0;
			iR = 0;
			//Center the Servos/Fingers
		  servo.pulsewidth(.001140); 
			servo2.pulsewidth(.000690);  
			servo3.pulsewidth(.001420); 
		
			musicSelectionOver = 0;       // At the end of the music, wait for the next one.
      taskFlag1 = 0;
			taskFlag2 = 0;
			taskFlag3 = 0;
			taskFlag4 = 0;
			
		  taskFlag1M = 0;
			taskFlag2M = 0;
			taskFlag3M = 0;
			taskFlag4M = 0;
			
			taskFlag1R = 0;
			taskFlag2R = 0;
			taskFlag3R = 0;
			taskFlag4R = 0;
	}
	
	//====Left Servo: Range from Sol - Mi, Centered at Ti/Do Boundary====
	if(taskFlag1)
	{                          
							 if(i < fileSize - 1){
								key = file[i];
								switch(key) {
										case 1: {    
													servo.pulsewidth(.001440);												
													break; //Sol
										}
										case 2: {
													servo.pulsewidth(.001380);
													break;//Sol#
										}
										case 3: {
														servo.pulsewidth(.001310);																				
														break;//La
										}
										case 4: {
														servo.pulsewidth(.001250); 
														break;//La#
										}
										case 5: {
														servo.pulsewidth(.001190); 
														break; //Ti
										}
										case 6: {
														servo.pulsewidth(.001110);            
														break;//Do
										}
										case 7: {
														servo.pulsewidth(.001060);
														break;//Do#
										}
										case 8: {
														servo.pulsewidth(.001010); 
														break;//Re
										}
										case 9: {
														servo.pulsewidth(.000950);
														break;//Re#
										}
										case 10: {
														servo.pulsewidth(.000880);
														break;//Mi
										}
										case 11: { //Stop Bit
														//servo.pulsewidth(.000830); //These are covered by middle servo.
														break;//Fa
										}
										case 12:{													
														//servo.pulsewidth(.000300); //Ditto												
														break;//Fa#
										}
										default: {
											break;
										}																																							
								}						
								//printf("Left Key: %d \r\n", key);											
								startTurningTime = tim.read_ms();
								taskFlag1 = 0;
								taskFlag2 = 1;
								taskFlag3 = 0;
								taskFlag4 = 0;
								//printf("startTurning time %d \r\n", startTurningTime);									
							} //close if
							else
							{
										taskFlag1 = 0;
										taskFlag2 = 0;
										taskFlag3 = 0;
										taskFlag4 = 0;
										printf("End of Left Hand \r\n");						
							}										 
		}
													
	//====Middle Servo: Range from Fa - Re, Centered at La/Ti Boundary====
	if(taskFlag1M)
							{                          
							 if(iM < fileMSize - 1){
								keyM = fileM[iM];
								switch(keyM) {
										case 1: {
													servo2.pulsewidth(.001140);
													break;//Fa
										}
										case 2: {
													servo2.pulsewidth(.001080);
													break;//Fa#
										}
										case 3: {
														servo2.pulsewidth(.001020);																				
														break;//Sol
										}
										case 4: {
														servo2.pulsewidth(.000960);
														break;//Sol#
										}
										case 5: {
														servo2.pulsewidth(.000890);
														break; //La
										}
										case 6: {
														servo2.pulsewidth(.000830);             
														break;//La#
										}
										case 7: {
														servo2.pulsewidth(.000770);
														break;//Ti
										}
										case 8: {
														servo2.pulsewidth(.000710);
														break;//Do
										}
										case 9: {
														servo2.pulsewidth(.000650);
														break;//Do#
										}
										case 10: {
														servo2.pulsewidth(.000590);
														break;//Re
										}
										case 11: { //Stop Bit
														//servo2.pulsewidth(.000390); //Covered by right hand
														break;//Re#
										}
										case 12:{
														//servo2.pulsewidth(.000300); //Ditto
														break;//Mi
										}
										default: {
											break;
										}
																																														
								}												
								//printf("Middle Key: %d \r\n", keyM);			
								startTurningTimeM = tim.read_ms();
								taskFlag1M = 0;
								taskFlag2M = 1;
								taskFlag3M = 0;
								taskFlag4M = 0;
								//printf("Middle startTurning time %d \r\n", startTurningTimeM);			
								} //close if
								else
								{
											taskFlag1M = 0;
											taskFlag2M = 0;
											taskFlag3M = 0;
											taskFlag4M = 0;
											printf("End of Middle Hand Part \r\n");							
								}
										
												 
		}

	//====Right Servo: Range from Re# - Do, Centered at Sol/La Boundary====
			if(taskFlag1R)
							{
							 if(iR < fileRSize - 1){
								keyR = fileR[iR];
								
								switch(keyR) {
										case 1: {
													servo3.pulsewidth(.001740);
													break;//Re#
										}
										case 2: {
													servo3.pulsewidth(.001680);
													break;//Mi
										}
										case 3: {
														servo3.pulsewidth(.001600);																					
														break;//Fa
										}
										case 4: {
														servo3.pulsewidth(.001540);
														break;//Fa#
										}
										case 5: {
														servo3.pulsewidth(.001470);
														break; //Sol
										}
										case 6: {
														servo3.pulsewidth(.001400);            
														break;//Sol#
										}
										case 7: {
														servo3.pulsewidth(.001330);
														break;//La
										}
										case 8: {
														servo3.pulsewidth(.001260);
														break;//La#
										}
										case 9: {
														servo3.pulsewidth(.001190);
														break;//Ti
										}
										case 10: {
														servo3.pulsewidth(.001130);
														break;//Do
										}
										case 11: { //Stop Bit
														//servo3.pulsewidth(.001080); //Unused
														break;//Do#
										}
										case 12:{
														//servo3.pulsewidth(.000330); //Ditto
														break;//Re
										}
										default: {
											break;
										}
																																														
								}
													
								//printf("Right Key: %d \r\n", keyR);								
								startTurningTimeR = tim.read_ms();
								taskFlag1R = 0;
								taskFlag2R = 1;
								taskFlag3R = 0;
								taskFlag4R = 0;					
								//printf("Right startTurning time %d \r\n", startTurningTimeR);		
								} //close if
								else
								{
											taskFlag1R = 0;
											taskFlag2R = 0;
											taskFlag3R = 0;
											taskFlag4R = 0;
											printf("End of Right Hand Part \r\n");	//For MHHL this is larger file so it wont print  this statement			
								}																					 
		}
}
		
void pressKey() {
	
	  int currentTime;
	  currentTime = tim.read_ms();
			
		//printf("pressKey\r\n");

		//====Left Solenoid====
		if((currentTime - startTurningTime) >= 200)       //Time for Servo to Reach next Key, then Solenoid can press key.
		{					
						if(taskFlag2)
						 {							 
								 if (file[i] != 12){
									 leftFinger = 1; //press, only if not a 'buffer key'
								 }
								 pressingTime = tim.read_ms();
								 endTurningTime = tim.read_ms();
								 turnPermit = 0;  // press flag on
								 taskFlag1 = 0;
								 taskFlag2 = 0;
								 taskFlag3 = 1;
								 taskFlag4 = 0;							
							  //printf("Pressing time %d \r\n", pressingTime);
						 }				 
		 }
			 
		 //====Middle Solenoid====		 
			if((currentTime - startTurningTimeM) >= 200)       //Time for Servo to Reach next Key, then Solenoid can press key.
			{						
							if(taskFlag2M)
							 {
								 if(fileM[iM] != 12){
										 middleFinger = 1; //press, only if not a 'buffer key'
								 }
								 pressingTimeM = tim.read_ms();
								 endTurningTimeM = tim.read_ms();
								 //turnPermitM = 0;  // press flag on
								 taskFlag1M = 0;
								 taskFlag2M = 0;
								 taskFlag3M = 1;
								 taskFlag4M = 0;						
								 //printf("Middle Pressing time %d \r\n", pressingTimeM);
							 }					 
			 }
				 
			 //====Right Solenoid====																	 
			if((currentTime - startTurningTimeR) >= 200)       //Time for Servo to Reach next Key, then Solenoid can press key.
			{			
							if(taskFlag2R)
							 {
									 //printf("PRESSING KEY\r\n");
								 if(fileR[iR] != 12){	
									 rightFinger = 1; //press
								 }
								 pressingTimeR = tim.read_ms();
								 endTurningTimeR = tim.read_ms();
								 //turnPermitR = 0;  // press flag on
								 taskFlag1R = 0;
								 taskFlag2R = 0;
								 taskFlag3R = 1;
								 taskFlag4R = 0;				
								 //printf("Right Pressing time %d \r\n", pressingTimeR);
							 }								 
			 }
		 
				 
	
	
}
	
void depressKey() {
		int currentTime;
		currentTime = tim.read_ms();  
    
		//printf("depressKey\r\n");

	
		//check the press duration, so check the difference between the time of press (of piano key) and current time, if this is
		//larger than the "press duration" then depress the key, i.e. set the solenoid GPOI to 0.
		if( (currentTime - pressingTime) >= (file[i+2])*1000 )    																																																																		
		{
				if (taskFlag3)
				{   
						leftFinger = 0; // de - press
						depressingTime = tim.read_ms();
						taskFlag1 = 0;
						taskFlag2 = 0;
						taskFlag4 = 1;
						taskFlag3 = 0;
						//printf("depressing time %d \r\n", depressingTime);						
				 }                
      }

		//====Middle Solenoid====	
		if( (currentTime - pressingTimeM) >= (fileM[iM+2])*1000 )    																																																																	
		{
				if (taskFlag3M)
				{  
						middleFinger = 0; // de - press
						depressingTimeM = tim.read_ms();
						taskFlag1M = 0;
						taskFlag2M = 0;
						taskFlag4M = 1;
						taskFlag3M = 0;
						//printf("Middle depressing time %d \r\n", depressingTimeM);					
				 }            
      }
		
		//====Right Solenoid====		
		if( (currentTime - pressingTimeR) >= (fileR[iR+2])*1000 )    																																																																		
		{
				if (taskFlag3R)
				{  
						rightFinger = 0; // de - press
						depressingTimeR = tim.read_ms();
						taskFlag1R = 0;
						taskFlag2R = 0;
						taskFlag4R = 1;
						taskFlag3R = 0;
						//printf("Right depressing time %d \r\n", depressingTimeR);										
				 }                              
      }
}


void setNextKey() {
	
	int currentTime;
	currentTime = tim.read_ms();
     
	//printf("setNextKey\r\n");	
     
	 // set the next pwm. So, after time to next key - press duration has elapsed, and this is taken as the
	 // difference between current time and the time at which the key was depressed, since a task is used to
	 // check if key press duration has elapsed.
	
	
	//====Left Key====
	
	
	if (file[i] != 11){ //Don't increment index after reached stop bit
		if((currentTime - depressingTime)  > (file[i+1]-file[i+2])*1000)  
			{   
					 if(taskFlag4)
					 {   
										//printf("i %d\r\n", i);
										i = i+3;
										turnPermit = 1;
										taskFlag1 = 1;
										taskFlag2 = 0;
										taskFlag3 = 0;
										taskFlag4 = 0;
									
					 }
			}
}
		
		
	//====Middle Key====
  
	if (fileM[iM] != 11){
		if((currentTime - depressingTimeM)  > (fileM[iM+1]-fileM[iM+2])*1000)  
			{   
					 if(taskFlag4M)
					 {        
									//printf("iM %d\r\n", iM);
										iM = iM+3;
										//turnPermitM = 1;
										taskFlag1M = 1;
										taskFlag2M = 0;
										taskFlag3M = 0;
										taskFlag4M = 0;
									
					 }
			}
	}
		
		//====Right Servo====
	
	if (fileR[iR] != 11){
		if((currentTime - depressingTimeR)  > (fileR[iR+1]-fileR[iR+2])*1000)  
			{   
					 if(taskFlag4R)
					 {                   
									//printf("iR %d\r\n", iR);
										iR = iR+3;
										//turnPermitR = 1;
										taskFlag1R = 1;
										taskFlag2R = 0;
										taskFlag3R = 0;
										taskFlag4R = 0;
									
					 }
			}
	}
		
}



void musicSelection()
{
	int i = 0;
	
	while(musicSelectionOver == 0)
	{
		     printf("derp!\r\n");
	       while (bt.readable() && i < 70*6) {
            char tmp = bt.getc();
           
            //pc.putc(tmp);
            rx_buf[i++] = tmp;
					 printf("derp2!\r\n");
            //end = tmp;
          }
				   wait(2);
					
					 //printChar(&rx_buf[0], i);
          // printf("\r\n");
					 if(rx_buf[i-1] == '!')
           {
           //printf("Bluetooth Packet Received: ");
           //printChar(&rx_buf[0], i);
           //printf("\r\n");
           //i = 0;
           printChar(&rx_buf[0], i);
           printf("\r\n");
			     
					 
					 musicFileDecode();  //function that use the rx_buf to assign file fileM fileR
					 musicSelectionOver = 1;
					 }
           
           }

  
						
	        clearBuf();					
          
	
	/*
  int musicNumber;
  //musicNumber =  bt.getc() - 48;      //change char into int
  musicNumber = getchar() - 48;
	
	switch (musicNumber)
    {
		 case 1: {
			printf("Play Mary Had a Little Lamb!\r\n");
					 
			float mhallFileL[1] = {11}; 
			float mhallFileM[mhallSizeM] = {12,1,0, 10,1,.5, 8,1,.5, 10,3,.5, 10,1,.5, 10,1,.5, 10,6,.5, 10,1,.5, 8,1,.5, 10,5,.5, 10,3,.5, 10,1,.5, 8,1,.5, 11};    
			float mhallFileR[mhallSizeR] = {2,4,.5, 2,1,.5, 2,1,.5, 2,4,.5, 2,1,.5, 5,1,.5, 5,1,.5, 2,4,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,2,.5, 2,1,.5, 2,1,.5, 11};  	 
			
			std::copy(mhallFileL,mhallFileL+(1), file);		
			std::copy(mhallFileM,mhallFileM+(mhallSizeM), fileM);
			std::copy(mhallFileR,mhallFileR+(mhallSizeR), fileR);

				
			fileSize = 1;
			fileMSize = mhallSizeM;
			fileRSize = mhallSizeR;
				
			//printf("fileM %f\r\n", fileM[39]);
				
			musicSelectionOver = 1;
			//Task Control (Locks) Variables
			turnPermit = 1;
			taskFlag1 = 1;
			taskFlag2 = 1;
			taskFlag3 = 1;
			taskFlag4 = 1;

			//bool turnPermitM = 1;
			taskFlag1M = 1;
			taskFlag2M = 1;
			taskFlag3M = 1;
			taskFlag4M = 1;

			//bool turnPermitR = 1;
			taskFlag1R = 1;
			taskFlag2R = 1;
			taskFlag3R = 1;
			taskFlag4R = 1;	
								
			break;
			}
                     
     case 2: {
       printf("Play Jingle Bell!\r\n");
         
			 //float jingleBellsL[jblSize] = {6,6,.5, 6,5,.5, 8,9,.5, 5,6,.5, 6,6,.5, 6,5,.5, 8,7,.5, 5,1,.5, 10,5,3, 11};
			 float jingleBellsL[1] = {11};
			 float jingleBellsM[jbmSize] = {12,3,0, 3,5,.5, 8,1,.5, 10,1,.5, 3,5,.5, 3,5,.5, 10,1,.5, 10,2,.5, 10,5,.5, 3,5,.5, 8,1,.5, 10,1,.5, 3,5,.5, 3,7,3, 8,1,.5, 10,1,3, 11};
	     float jingleBellsR[jbrSize] = {2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 5,3,.5, 2,1,.5, 3,1,.5, 3,1,.5, 3,1,.5, 3,1,.5, 3,1,.5,
																			2,1,.5, 2,1,.5, 2,1,.5, 2,3,.5, 2,2,5, 5,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 5,3,.5,
																			2,1,.5, 3,1,.5, 3,1,.5, 3,1,.5, 3,1,.5, 3,1,.5, 2,1,.5, 2,1,.5, 2,1,.5, 5,1,.5, 5,1,.5, 2,1,.5, 11};
       //std::copy(jingleBellsL,jingleBellsL+(jblSize), file);
			 std::copy(jingleBellsL,jingleBellsL+(1), file);
			 std::copy(jingleBellsM,jingleBellsM+(jbmSize), fileM);
			 std::copy(jingleBellsR,jingleBellsR+(jbrSize), fileR);
          
			//fileSize = jblSize;
			fileSize = 1;	 
			fileMSize = jbmSize;
			fileRSize = jbrSize;
                     
			musicSelectionOver = 1;
			turnPermit = 1;
			taskFlag1 = 1;
			taskFlag2 = 1;
			taskFlag3 = 1;
			taskFlag4 = 1;

			//bool turnPermitM = 1;
			taskFlag1M = 1;
			taskFlag2M = 1;
			taskFlag3M = 1;
			taskFlag4M = 1;

			//bool turnPermitR = 1;
			taskFlag1R = 1;
			taskFlag2R = 1;
			taskFlag3R = 1;
			taskFlag4R = 1;	
					 
				 
			break;
      }
                     
   case 3: {
            printf("Play Little Drummer Boy!\r\n");
         
            musicSelectionOver = 1;
            break;
           }
     case 4: {
            printf("Play Heart and Soul!\r\n");
         
                musicSelectionOver = 1;
            break;
           }
     
     default: {
            printf("Do nothing!\r\n");
                musicSelectionOver = 0;
                break;
           }
  }
   

*/
}

void musicFileDecode()
{
	  int i = 0;
	  int j = 0;
    int hand;
	
	  int leftCount = 0;
	  int middleCount = 0;
	  int rightCount = 0;
	
	  //printf("derp3!\r\n");
	  while(i == 0|| rx_buf[i] != '!')
		{
			    // printf("derp3! i= %d\r\n",i);
          if(rx_buf[i] == 'L')
					{
          hand = 1;
				  i++;
				  continue;
          }	
					
				  if(rx_buf[i] == 'M')
					{
          hand = 2;
				  i++;
				  continue;
          }
					
					if(rx_buf[i] == 'R')
					{
          hand = 3;
				  i++;
				  continue;
          }
					
					if(rx_buf[i] == '{')
					{
					i++;
          continue;
          }
					
					if(rx_buf[i] == '}')
					{
					i++;
          continue;
          }
					
					if(rx_buf[i] == ',')
					{
             if((rx_buf[i-2] == ',') ||(rx_buf[i-2] == '{'))	//  ",6,"
						 {
             switch(hand)
							 {
               case 1: 
								  {
                   file[leftCount] =  (rx_buf[i-1]-48)+0.0;   
									 
									 leftCount++;
								   break;
                  }
								 
						   case 2:
							    {
                   fileM[middleCount] = (rx_buf[i-1]-48)+0.0;
									 middleCount++;
                   break;

                  }
							 case 3:
					   		 {
                  fileR[rightCount] =  (rx_buf[i-1]-48)+0.0; // 
                  rightCount++;
                  break;
                 }		   
						
								}
              }
						

             if(rx_buf[i-3] == ','|| rx_buf[i-3] == '{') //",10,"
						 {
             switch(hand)
							 {
               case 1: 
								  {
                   file[leftCount] =  (rx_buf[i-2]-48)*10.0 + (rx_buf[i-1]-48) ;// 
									 
									 leftCount++;
								   break;
                  }
								 
						   case 2:
							    {
                   fileM[middleCount] = (rx_buf[i-2]-48)*10.0 + (rx_buf[i-1]-48) ;
									 middleCount++;
                   break;

                  }
							 case 3:
					   		 {
                  fileR[rightCount] = (rx_buf[i-2]-48)*10.0 + (rx_buf[i-1]-48) ; // 
                  rightCount++;
                  break;
                 }
							 }
						 }							 
							
								 
					 if(rx_buf[i-4] == ','|| rx_buf[i-4] == '{')  //",0.5,"
						 {
							 if(rx_buf[i-2] == ',')
							 {
							 i++;
               continue;
               }
             
							 switch(hand)
							 {
               case 1: 
								  {
                   file[leftCount] = 0.1*(rx_buf[i-1]-48)+(rx_buf[i-3]-48); // 
									 
									 leftCount++;
								   break;
                  }
								 
						   case 2:
							    {
                   fileM[middleCount] =  0.1*(rx_buf[i-1]-48)+(rx_buf[i-3]-48);
									 middleCount++;
                   break;

                  }
							 case 3:
					   		 {
                  fileR[rightCount] = 0.1*(rx_buf[i-1]-48)+(rx_buf[i-3]-48); // 
                  rightCount++;
                  break;
                 }							
							
							
							
						}
					}
					i++;
				}
				if(rx_buf[i] >= '0'&& rx_buf[i]<= '9')
				{
         i++;
				 continue;
        }
				
				if(rx_buf[i] == '.')
				{
         i++;
				 continue;

        }
				
			}
			//printf("derp4!\r\n");
			for (int derp = 0; derp < (leftCount); derp++) {
				printf("left: %f\r\n", file[derp]);
			}
			
			for (int derp2 = 0; derp2 < (middleCount); derp2++) {
				printf("middle: %f\r\n", fileM[derp2]);
			}
			
			for (int derp3 = 0; derp3 < (rightCount); derp3++) {
			printf("right: %f\r\n", fileR[derp3]);
			}
		}

void clearBuf()
{
 for (int i = 0; i < sizeof(rx_buf); i++) {
        rx_buf[i] = '\0';
}
}


