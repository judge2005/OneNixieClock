/*
 * OneNixieClock.h
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#ifndef LIBRARIES_ONENIXIECLOCK_ONENIXIECLOCK_H_
#define LIBRARIES_ONENIXIECLOCK_ONENIXIECLOCK_H_
#include <Arduino.h>
#include <NixieDriver.h>

class OneNixieClock {
public:
	OneNixieClock(NixieDriver& nixieDriver) :
		nixieDriver(nixieDriver)
	{
	}

	void loop(unsigned long nowMs);
	void init();

	void setShowSeconds(bool showSeconds) {
		this->showSeconds = showSeconds;
	}

	void setTimeSeconds(bool timeMode) {
		this->timeMode = timeMode;
	}

	void set12hour(bool twelveHour) {
		this->twelveHour = twelveHour;
	}

	void setTimeMode(bool timeMode) {
		this->timeMode = timeMode;
	}

	void setDateFormat(byte dateFormat) {
		this->dateFormat = dateFormat;
	}

	void setScrollback(bool scrollback) {
		this->scrollback = scrollback;
	}

	void setDigitsOn(int digitsOn) {
		this->digitsOn = digitsOn;
	}

	void setFadeMode(byte fadeMode) {
		this->fadeMode = (NixieDriver::DisplayMode)fadeMode;
	}

	void setDigit(byte digit);

	void setCountSpeed(byte countSpeed);

	void setClockMode(bool clockMode);

	void setOnOff(byte on, byte off);

	bool isOn();

	byte getNixieDigit() {
		return nixieDigit;
	}

	byte getTimePart() {
		return timePart;
	}

	byte getACPCount() {
		return acpCount;
	}
private:
	void doClock(unsigned long nowMs);
	void doCount(unsigned long nowMs);

	NixieDriver& nixieDriver;

	unsigned long nextNixieDisplay = 0;
	unsigned long nextACP = 0;
	byte timePart = 0;
	int digitsOn = 1500;
	bool scrollback = true;

	#define DIGIT_LINGER 1000
	int monthSnap = 0;
	int daySnap = 0;
	int hourSnap = 0;
	int minSnap = 0;
	int secSnap = 0;
	int acpCount = 0;

	bool showSeconds = true;
	bool twelveHour = false;
	bool timeMode = true;
	byte dateFormat = 1;
	NixieDriver::DisplayMode fadeMode = NixieDriver::FADE_OUT;

	bool clockMode = true;
	byte countSpeed = 60;
	byte nixieDigit = 0;
	byte on = 0;
	byte off = 24;
};


#endif /* LIBRARIES_ONENIXIECLOCK_ONENIXIECLOCK_H_ */
