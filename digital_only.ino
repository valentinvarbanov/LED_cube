#include <algorithm> // used for random_shuffle

using namespace std;

int layer[] = {8, 9, 7, 6};
int leds[] = {30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45};
int state[64];

void clear_leds();
void refresh(int);
void clear_state();
void shift_down();

void setup() {
	clear_state();
	int i;
	for (i = 0; i < 16; ++i) {
		pinMode(leds[i], OUTPUT);
	}
	for (i = 0; i < 4; ++i) {
		pinMode(layer[i], OUTPUT);
	}
	randomSeed(analogRead(0));
}


void loop() {
	// sequence 1
	// for (int t = 0; t < 3; ++t) {
		for (int i = 0; i < 64; ++i) {
				state[i] = 1;
				refresh(40);
		}
		clear_state();
	// }

	// sequence 2
	// for (int t = 0; t < 3; ++t) {
		int tmp[64] = {
			1, 2, 3, 4, 8, 12, 16, 15, 14, 13, 9, 5, 6, 7, 11, 10,
			26, 22, 23, 27, 31, 30, 29, 25, 21, 17, 18, 19, 20, 24, 28, 32,
			48, 47, 46, 45, 41, 37, 33, 34, 35, 36, 40, 44, 43, 42, 38, 39,
			55, 59, 58, 54, 50, 51, 52, 56, 60, 64, 63, 62, 61, 57, 53, 49
		};
		for (int i = 0; i < 64; ++i) {
			state[tmp[i] - 1] = 1;
			refresh(30);
		}
		for (int i = 0; i < 64; ++i) {
			state[tmp[i] - 1] = 0;
			refresh(30);
		}
		clear_state();
	// }

	// sequence 3
	for (int i = 0; i < 30; ++i) {
		for (int i = 0; i < 4; ++i) {
			state[random(16)] = 1;
		}
		refresh(200);
		shift_down();
	}
	clear_state();

	// sequence 4
	// int tmp[64];
	for (int i = 0; i < 64; ++i) {
		tmp[i] = i;
	}
	random_shuffle(tmp, tmp + 64);
	for (int i = 0; i < 64; ++i) {
		state[tmp[i]] = 1;
		refresh(200);
	}

	// sequence 5
	for (int i = 0; i < 5; ++i) {
		int arr_seq[][16]{
			{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
			{0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0},
			{0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0},
			{0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0}
		};
		for (int j = 0; j < 6; ++j) {
			for (int i = 0; i < 16; ++i) {
				state[i] = state[i + 16] = state[i + 2 * 16] = state[i + 3 * 16] = arr_seq[j][i];
			}
			refresh(200);
		}
	}
	clear_state();


}

void clear_leds() {
	int i;
	for (i = 0; i < 16; ++i) {
		digitalWrite(leds[i], LOW);
	}
	for (i = 0; i < 4; ++i) {
		digitalWrite(layer[i], HIGH);
	}
}

void refresh(int time) {
	for (time = time / 10; time > 0; --time) {
		for (int i = 0; i < 4; ++i) {		
			clear_leds();
			digitalWrite(layer[i], LOW);
			for (int j = 0; j < 16; ++j) {
				if (state[i * 16 + j] == 1) {
					digitalWrite(leds[j], HIGH);
				}
			}
			delayMicroseconds(2500);
		}
	}
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
