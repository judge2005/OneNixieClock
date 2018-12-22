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
	virtual void syncDisplay();

	void setNixieDriver(NixieDriver* pNixieDriver) {
		this->pNixieDriver = pNixieDriver;
	}

	void setShowSeconds(bool showSeconds) {
		this->showSeconds = showSeconds;
	}

	void set12hour(bool twelveHour) {
		if (this->twelveHour != twelveHour) {
			this->twelveHour = twelveHour;
			displayTimer.setDuration(0);
		}
	}

	void setLeadingZero(bool leadingZero) {
		if (this->leadingZero != leadingZero) {
			this->leadingZero = leadingZero;
			displayTimer.setDuration(0);
		}
	}

	void setShowDate(bool showDate) {
		if (this->showDate != showDate) {
			this->showDate = showDate;
			showDate = 0;
		}
	}

	virtual void setTimeMode(bool timeMode) {
		if (this->timeMode != timeMode) {
			this->timeMode = timeMode;
			displayTimer.setDuration(0);
		}
	}

	virtual void setAlternateTime(bool alternateTime) {
		if (this->alternateTime != alternateTime) {
			this->alternateTime = alternateTime;
			displayTimer.setDuration(0);
		}
	}

	virtual void toggleAlternateTime() {
		alternateTime = !alternateTime;
		displayTimer.setDuration(0);
	}

	void setDateFormat(byte dateFormat) {
		if (this->dateFormat != dateFormat) {
			this->dateFormat = dateFormat;
			displayTimer.setDuration(0);
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

	void setDigit(uint32_t digit);

	void setCountSpeed(byte countSpeed);

	void setClockMode(bool clockMode);

	void setOnOff(byte on, byte off);

	bool isOn();

	uint32_t getNixieDigit() {
		return nixieDigit;
	}

	byte getTimePart() {
		return timePart;
	}

	byte getACPCount() {
		return acpCount;
	}

	/*
	 * Overflow-safe timer
	 */
	class Timer {
	public:
		Timer(unsigned int duration) : lastTick(0), duration(duration) {}
		Timer() : lastTick(0), duration(0) {}

		bool expired(uint32_t now) {
			return now - lastTick >= duration;
		}
		void setDuration(unsigned int duration) {
			this->duration = duration;
		}
		void init(uint32_t now, unsigned int duration) {
			lastTick = now;
			this->duration = duration;
		}
	private:
		uint32_t lastTick;
		unsigned int duration;
	};

protected:
	NixieDriver* pNixieDriver;

	Timer displayTimer;
	unsigned long nextACP = 0;
	byte timePart = 0;
	int digitsOn = 1500;
	bool scrollback = true;

	#define DIGIT_LINGER 1000
	int yearSnap = 0;
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
	bool showDate = true;
	bool leadingZero = true;
	byte dateFormat = 1;
	NixieDriver::DisplayMode fadeMode = NixieDriver::FADE_OUT;

	bool clockMode = true;
	byte countSpeed = 60;
	byte numDigits = 12;
	uint32_t nixieDigit = 0;
	byte on = 0;
	byte off = 24;
};


#endif /* LIBRARIES_NIXIECLOCK_NIXIECLOCK_H_ */
