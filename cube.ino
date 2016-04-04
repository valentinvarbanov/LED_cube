#include <algorithm>

#define STABILITY_DELAY 500 // in microseconds
#define ANALOG_READ_RESOLUTION 8 // max 12
#define ANALOG_WRITE_RESOLUTION 8 // max 12

using namespace std;

int layer[] = {8, 9, 7, 6};
int leds[] = {30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45};
int input_pin = A7;
int button_pin = 50;
int state[64];
int intensity;
volatile int sequence = 0;
volatile int last_button_state = 0;
volatile int sequence_changed = 0;

int max_read_value = pow(2, ANALOG_READ_RESOLUTION) - 1;
int max_write_value = pow(2, ANALOG_WRITE_RESOLUTION) - 1;

void clear_leds();
int refresh(int);
void clear_state();
void shift_down();
void read_intensity();
void check_button();

void interrupt_handler();

void setup() {
	clear_state();
	int i;
	for (i = 0; i < 16; ++i) {
		pinMode(leds[i], OUTPUT);
	}
	for (i = 0; i < 4; ++i) {
		pinMode(layer[i], OUTPUT);
	}
	pinMode(input_pin, INPUT);
	randomSeed(analogRead(0));
	analogReadResolution(ANALOG_READ_RESOLUTION);
	analogWriteResolution(ANALOG_WRITE_RESOLUTION);
	Serial.begin(9600);

	pinMode(button_pin, INPUT);
	attachInterrupt(button_pin, interrupt_handler, CHANGE);
}

void loop() {

	switch(sequence) {
		case 0: {
 			// sequence 1
			for (int i = 0; i < 64; ++i) {
				state[i] = 1;
				if(refresh(40) == 1) {
					break;
				}
			}
			clear_state();
		} break;
		case 1: {
			// sequence 2
			int should_break = 0;
			int tmp[64] = {
				1, 2, 3, 4, 8, 12, 16, 15, 14, 13, 9, 5, 6, 7, 11, 10,
				26, 22, 23, 27, 31, 30, 29, 25, 21, 17, 18, 19, 20, 24, 28, 32,
				48, 47, 46, 45, 41, 37, 33, 34, 35, 36, 40, 44, 43, 42, 38, 39,
				55, 59, 58, 54, 50, 51, 52, 56, 60, 64, 63, 62, 61, 57, 53, 49
			};
			for (int i = 0; i < 64; ++i) {
				state[tmp[i] - 1] = 1;
				if(refresh(30) == 1) {
					should_break = 1;
					break;
				}
			}
			if (should_break) {
				clear_state();
				break;
			}
			for (int i = 0; i < 64; ++i) {
				state[tmp[i] - 1] = 0;
				if(refresh(30) == 1) {
					break;
				}
			}
			clear_state();
		} break;
		case 2: {
			// sequence 3
			int should_break = 0;
			for (int i = 0; i < 30; ++i) {
				for (int i = 0; i < 4; ++i) {
					state[random(16)] = 1;
				}
				if (refresh(200) == 1) {
					should_break = 1;
					break;
				}
				if (should_break) {
					break;
				}
				shift_down();
			}
			clear_state();
		} break;
		case 3:{
			// sequence 4
			int tmp[64];
			for (int i = 0; i < 64; ++i) {
				tmp[i] = i;
			}
			random_shuffle(tmp, tmp + 64);
			for (int i = 0; i < 64; ++i) {
				state[tmp[i]] = 1;
				if (refresh(200) == 1) {
					break;
				}
			}
			clear_state();
		} break;
		case 4:{
			// sequence 5
			int should_break = 0;
			int arr_seq[][16]{
				{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
				{0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0},
				{0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0},
				{0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0}
			};
			for (int i = 0; i < 5; ++i) {
				for (int j = 0; j < 6; ++j) {
					for (int i = 0; i < 16; ++i) {
						state[i] = state[i + 16] = state[i + 2 * 16] = state[i + 3 * 16] = arr_seq[j][i];
					}
					if (refresh(200) == 1) {
						should_break = 1;
						break;
					}
				}
				if (should_break) {
					break;
				}
			}
			clear_state();
		} break;
	}
}

void clear_leds() {
	int i;
	for (i = 0; i < 16; ++i) {
		digitalWrite(leds[i], LOW);
	}
	for (i = 0; i < 4; ++i) {
		// digitalWrite(layer[i], HIGH);
		analogWrite(layer[i], max_write_value);
		delayMicroseconds(STABILITY_DELAY);
	}
}

int refresh(int time) {
	for (time = time / 10; time > 0; --time) {
		read_intensity();
		check_button();
		for (int i = 0; i < 4; ++i) { 
			if (sequence_changed) {
				sequence_changed = 0;
				return 1;
			}  
			clear_leds();
			// digitalWrite(layer[i], LOW);
			analogWrite(layer[i], max_write_value - intensity);
			if (sequence_changed) {
				sequence_changed = 0;
				return 1;
			}
			delayMicroseconds(STABILITY_DELAY);
			for (int j = 0; j < 16; ++j) {
				if (state[i * 16 + j] == 1) {
					digitalWrite(leds[j], HIGH);
				}
			}
			if (sequence_changed) {
				sequence_changed = 0;
				return 1;
			}
			delayMicroseconds(2500);
		}
	}
	return 0;
}

void clear_state() {
	for (int i = 0; i < 64; ++i) {
		state[i] = 0;
	}
}

void shift_down() {
	for (int i = 63; i >= 16; --i) {
		state[i] = state[i - 16];
	}
	for (int i = 0; i < 16; ++i) {
		state[i] = 0;
	}
}

void read_intensity() {
	if (! ANALOG_READ_RESOLUTION == ANALOG_WRITE_RESOLUTION) {
		intensity = map(analogRead(input_pin), 0, max_read_value, 0, max_write_value);
	} else {
		intensity = analogRead(input_pin);
	}
	// Serial.println(intensity);
	Serial.println(sequence);
}

void check_button() {
	int button_state = digitalRead(button_pin);

	if (button_state != last_button_state) {
		if (button_state == HIGH) {
			sequence++;
			sequence %= 5;
			sequence_changed = 1;
		}
		// debouncing
		delayMicroseconds(10000);
		delayMicroseconds(10000);
		delayMicroseconds(10000);
		delayMicroseconds(10000);
		delayMicroseconds(10000);
	}
	last_button_state = button_state;
}

void interrupt_handler() {
	check_button();
}
