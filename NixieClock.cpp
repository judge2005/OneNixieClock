/*
 * NixieClock.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#include <NixieClock.h>
#include <TimeLib.h>

void NixieClock::init() {
	timePart = 0;
	displayTimer.init(millis(), 0);
	nixieDigit = 0;
}

void NixieClock::syncDisplay() {
	timePart = 0;
	displayTimer.init(millis(), 0);
	nixieDigit = 0;
}

void NixieClock::setClockMode(bool clockMode) {
	if (clockMode != this->clockMode) {
		this->clockMode = clockMode;
		timePart = 0;
		acpCount = 0;
		time_t _now = now();
		monthSnap = month(_now);
		daySnap = day(_now);
		hourSnap = hour(_now);
		minSnap = minute(_now);
		secSnap = second(_now);
		displayTimer.init(millis(), 0);
	}
}

void NixieClock::setOnOff(byte on, byte off) {
	this->on = on;
	this->off = off;
}

bool NixieClock::isOn() {
	if (!clockMode) {
		return true;
	}

	if (on == off) {
		return true;
	}

	if (on < off) {
		return hourSnap >= on && hourSnap < off;
	}

	if (on > off) {
		return !(hourSnap >= off && hourSnap < on);
	}

	return false;
}

void NixieClock::setDigit(const uint32_t digit) {
	if (!clockMode && this->nixieDigit != digit) {
		if (countSpeed != 0) {
			displayTimer.init(millis(), 60000 / countSpeed);
		}
		nixieDigit = digit;
		pNixieDriver->setNewNixieDigit(nixieDigit);
	}
}

void NixieClock::setCountSpeed(byte countSpeed) {
	if (!clockMode && this->countSpeed != countSpeed) {
		if (countSpeed != 0) {
			if (this->countSpeed != 0) {
				int adj = 60000/countSpeed - 60000/this->countSpeed;

				displayTimer.init(millis(), adj > 0 ? adj : 0);
			} else {
				displayTimer.init(millis(), 60000 / countSpeed);
			}
		}
	}

	this->countSpeed = countSpeed;
}
