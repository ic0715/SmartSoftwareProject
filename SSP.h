// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef SmartCar_01_H_
#define SmartCar_01_H_
#include "Arduino.h"
//add your includes for the project SmartCar_1 here

 #define FORWARD 0x09
 #define BACKWARD 0x06
 #define LEFT_U 0x0A
 #define RIGHT_U 0x05
 #define STOP 0x00

//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project SmartCar_1 here

void Motor_mode(int da);
void Motor_Control(char da, unsigned int OC_value);
void Timer2_ISR();
void getMaxValueIndex(unsigned long int score[], int size);
void getMinValueIndex(unsigned long int RX_front[]);
void avoid_front_wall();
void controlSpeed();
void setBuzzer();
void Encoder_count_L();
void Encoder_count_R();
void LED_set(int LED_state);

//Do not add code below this line
#endif /* SmartCar_1_H_ */




