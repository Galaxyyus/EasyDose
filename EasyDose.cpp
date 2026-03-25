// Pins setup
int morning_button = 5;
int afternoon_button = 6;
int night_button = 7;

int buzzer = 11;
int alarm_led = 12;
int alarm_master = 13;

int before_switch = 2;
int after_switch = 3;

// Alarm state struct
struct Alarm {
	bool enabled;
	bool before;
	bool after;
};

Alarm morning = { false, false, false };
Alarm afternoon = { false, false, false };
Alarm night = { false, false, false };

// Stored alarm state
bool morning_set = false;
bool afternoon_set = false;
bool night_set = false;

bool alarm_locked = false;

// Extra auxillary variables
bool alarm_turned_off = false;
bool ring = false;
bool alarm_state = false;
unsigned long last_on = 0;
unsigned long wait_time = 300;

bool last_master_state = false;

// Sets up the pins to the required digital state
void setupPins() {
	pinMode(morning_button, INPUT_PULLUP);
	pinMode(afternoon_button, INPUT_PULLUP);
	pinMode(night_button, INPUT_PULLUP);

	pinMode(before_switch, INPUT);
	pinMode(after_switch, INPUT);

	pinMode(alarm_master, INPUT_PULLUP);

	pinMode(buzzer, OUTPUT);
	pinMode(alarm_led, OUTPUT);
}

void readAlarmConfig() {
	bool before = (digitalRead(before_switch) == LOW);
	bool after = (digitalRead(after_switch) == LOW);

	if (digitalRead(morning_button) == LOW) {
		morning.enabled = true;
		morning.before = before;
		morning.after = after;
	}

	if (digitalRead(afternoon_button) == LOW) {
		afternoon.enabled = true;
		afternoon.before = before;
		afternoon.after = after;
	}

	if (digitalRead(night_button) == LOW) {
		night.enabled = true;
		night.before = before;
		night.after = after;
	}
}

// Prints the state fo the alarm times
void printAlarmConfig() {
	Serial.println("Alarm configuration:");

	Serial.print("Morning: ");
	if (!morning.enabled) {
		Serial.println("OFF");
	} else if (morning.before) {
		Serial.println("BEFORE");
	} else if (morning.after) {
		Serial.println("AFTER");
	} else {
		Serial.println("ON (no window)");
	}

	Serial.print("Afternoon: ");
	if (!afternoon.enabled) {
		Serial.println("OFF");
	} else if (afternoon.before) {
		Serial.println("BEFORE");
	} else if (afternoon.after) {
		Serial.println("AFTER");
	} else {
		Serial.println("ON (no window)");
	}

	Serial.print("Night: ");
	if (!night.enabled) {
		Serial.println("OFF");
	} else if (night.before) {
		Serial.println("BEFORE");
	} else if (night.after) {
		Serial.println("AFTER");
	} else {
		Serial.println("ON (no window)");
	}
}

//  Reads the time from the Serial continuously
String readTimeFromSerial() {
	while (!Serial.available()) {
		;
	}
	return Serial.readStringUntil('\n');
}

bool isAlarmTimeMatch(String time_now, String switch_now) {
	time_now.trim();
	switch_now.trim();

	Alarm* a = nullptr;

	if (time_now == "morning") {
		a = &morning;
	} else if (time_now == "afternoon") {
		a = &afternoon;
	} else if (time_now == "night") {
		a = &night;
	}

	if (a == nullptr || !a->enabled) {
		return false;
	}

	if (switch_now == "before") {
		return a->before;
	}
	if (switch_now == "after") {
		return a->after;
	}

	return false;
}

// Turns on the alarm
void triggerAlarm() {
	if (!ring) {
		return;
	}

	if (millis() - last_on > wait_time) {
		alarm_state = !alarm_state;
		last_on = millis();
	}

	digitalWrite(alarm_led, alarm_state);
	digitalWrite(buzzer, alarm_state);
}

// Extra debug function
// void debugDump() {
//     Serial.println("---- DEBUG DUMP ----");

//     Serial.print("Morning -> enabled=");
//     Serial.print(morning.enabled);
//     Serial.print(", before=");
//     Serial.print(morning.before);
//     Serial.print(", after=");
//     Serial.println(morning.after);

//     Serial.print("Afternoon -> enabled=");
//     Serial.print(afternoon.enabled);
//     Serial.print(", before=");
//     Serial.print(afternoon.before);
//     Serial.print(", after=");
//     Serial.println(afternoon.after);

//     Serial.print("Night -> enabled=");
//     Serial.print(night.enabled);
//     Serial.print(", before=");
//     Serial.print(night.before);
//     Serial.print(", after=");
//     Serial.println(night.after);

//     Serial.print("ring=");
//     Serial.print(ring);
//     Serial.print(", alarm_locked=");
//     Serial.println(alarm_locked);

//     Serial.println("--------------------");
// }


void setup() {
	setupPins();
	Serial.begin(9600);
	Serial.println("Alarm system ready.");
}

void loop() {
	bool master_now = (digitalRead(alarm_master) == LOW);

	// Master button edge detect
	if (master_now && !last_master_state) {

		// First press: lock config and start input
		if (!alarm_locked) {
			alarm_locked = true;
			alarm_turned_off = false;
			printAlarmConfig();
			Serial.println("Enter current time (morning / afternoon / night):");
		}

		// Second press: stop alarm and reopen input
		else if (ring) {
			ring = false;
			alarm_state = false;
			alarm_turned_off = true;

			digitalWrite(alarm_led, LOW);
			digitalWrite(buzzer, LOW);

			Serial.println("Alarm stopped. Enter new time:");
		}
	}

	last_master_state = master_now;

	// CONFIG MODE
	if (!alarm_locked) {
		readAlarmConfig();
		// debugDump;
		return;
	}

	// INPUT MODE (after config or after stopping alarm)
	if (!ring) {
		if (Serial.available()) {
			String time_now = readTimeFromSerial();
			Serial.println("Enter before / after:");
			String switch_now = readTimeFromSerial();

			if (isAlarmTimeMatch(time_now, switch_now)) {
				ring = true;
				alarm_turned_off = false;
				last_on = millis();
				Serial.println("Alarm triggered!");
			} else {
				Serial.println("No alarm match. Try again:");
			}
		}
		return;
	}

	// RINGING MODE
	triggerAlarm();
}
