/*
 * NixieClock.h
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#ifndef LIBRARIES_NIXIECLOCK_NIXIECLOCK_H_
#define LIBRARIES_NIXIECLOCK_NIXIECLOCK_H_
#include <Arduino.h>
#include <NixieDriver.h>

class NixieClock {
public:
	NixieClock(NixieDriver* pNixieDriver) :
		pNixieDriver(pNixieDriver)
	{
	}

	virtual ~NixieClock() {}

	virtual void loop(unsigned long nowMs) = 0;
	virtual void init();

	void setNixieDriver(NixieDriver* pNixieDriver) {
		this->pNixieDriver = pNixieDriver;
	}

	void setShowSeconds(bool showSeconds) {
		this->showSeconds = showSeconds;
	}

	void set12hour(bool twelveHour) {
		if (this->twelveHour != twelveHour) {
			this->twelveHour = twelveHour;
			nextNixieDisplay = 0;
		}
	}

	void setTimeMode(bool timeMode) {
		if (this->timeMode != timeMode) {
			this->timeMode = timeMode;
			nextNixieDisplay = 0;
		}
	}

	void setAlternateTime(bool alternateTime) {
		if (this->alternateTime != alternateTime) {
			this->alternateTime = alternateTime;
			nextNixieDisplay = 0;
		}
	}

	void toggleAlternateTime() {
		alternateTime = !alternateTime;
		nextNixieDisplay = 0;
	}

	void setDateFormat(byte dateFormat) {
		if (this->dateFormat != dateFormat) {
			this->dateFormat = dateFormat;
			nextNixieDisplay = 0;
		}
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

	void setNumDigits(byte numDigits) {
		this->numDigits = numDigits;
	}

	void setDigit(uint16_t digit);

	void setCountSpeed(byte countSpeed);

	void setClockMode(bool clockMode);

	void setOnOff(byte on, byte off);

	bool isOn();

	uint16_t getNixieDigit() {
		return nixieDigit;
	}

	byte getTimePart() {
		return timePart;
	}

	byte getACPCount() {
		return acpCount;
	}
protected:
	NixieDriver* pNixieDriver;

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
	bool alternateTime = false;
	byte dateFormat = 1;
	NixieDriver::DisplayMode fadeMode = NixieDriver::FADE_OUT;

	bool clockMode = true;
	byte countSpeed = 60;
	byte numDigits = 12;
	uint16_t nixieDigit = 0;
	byte on = 0;
	byte off = 24;
};


#endif /* LIBRARIES_NIXIECLOCK_NIXIECLOCK_H_ */
