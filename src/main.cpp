#include "WProgram.h"

#define DEBUG

const int N_INPUTS = 6;
const int N_OUTPUTS = 4;
const int INPUT_PINS[N_INPUTS] = {0, 1, 2, 3, 4, 5};
const int OUTPUT_PINS[N_OUTPUTS] = {6, 7, 8, 9};

void write_matrix(int input_idx){
    int prev_input_idx = (input_idx - 1 + N_INPUTS)%N_INPUTS;
    digitalWrite(INPUT_PINS[prev_input_idx], LOW);
    digitalWrite(INPUT_PINS[input_idx], HIGH);
    #ifdef DEBUG
    Serial.print("INPUT PIN ");
    Serial.print(INPUT_PINS[input_idx]);
    Serial.println(" HIGH");
    #endif
}

void setup_matrix(){
    int idx=0;
    int pin=0;
    for (idx=0; idx<N_INPUTS; idx++)
    {
        pin = INPUT_PINS[idx];
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    for (idx=0; idx<N_OUTPUTS; idx++)
    {
        pin = OUTPUT_PINS[idx];
        pinMode(pin, INPUT);
        *portConfigRegister(pin) &= ~PORT_PCR_PS; //pull down
    }
}

extern "C" int main(void)
{
#ifdef USING_MAKEFILE

	// To use Teensy 3.0 without Arduino, simply put your code here.
	// For example:

	pinMode(13, OUTPUT);
    setup_matrix();

    int incomingByte = 0;
    int input_idx = 0;
	while (1) {
        for (input_idx=0; input_idx<N_INPUTS; ++input_idx){
            write_matrix(input_idx);
            Serial.println("Press a key on host keyboard to move to the next input");
            while (Serial.available() <= 0)
                ;//wait for input
            incomingByte = Serial.read();
            // say what you got:
            Serial.print("I received: ");
            Serial.println(incomingByte, DEC);
        }
	}


#else
	// Arduino's main() function just calls setup() and loop()....
	setup();
	while (1) {
		loop();
		yield();
	}
#endif
}

