/*
 * NixieDisplay.h
 *
 *  Created on: Nov 22, 2023
 *      Author: pandrews
 */

#ifndef LIBRARIES_ONENIXIECLOCK_NIXIEDISPLAY_H_
#define LIBRARIES_ONENIXIECLOCK_NIXIEDISPLAY_H_
#include <Arduino.h>
#include <NixieDriver.h>
#include <Timer.h>

class NixieDisplay {
public:
	NixieDisplay(NixieDriver* pNixieDriver) :
		pNixieDriver(pNixieDriver)
	{
		pNixieDriver->setBrightness(brightness);
	}
	virtual ~NixieDisplay() {}

	virtual void loop(unsigned long nowMs) = 0;
	virtual void init();

	void setNixieDriver(NixieDriver* pNixieDriver) {
		this->pNixieDriver = pNixieDriver;
		pNixieDriver->setBrightness(brightness);
	}

	NixieDriver* getNixieDriver() {
		return pNixieDriver;
	}

	virtual void setBrightness(const byte b) {
		if (!lockedBrightness) {
			brightness = b;
			pNixieDriver->setBrightness(brightness);
		}
	}

	void setFadeMode(byte fadeMode) {
		this->fadeMode = (NixieDriver::DisplayMode)fadeMode;
	}

	void setDigit(uint32_t digit);

	void setCountSpeed(byte countSpeed);

	uint32_t getNixieDigit() {
		return nixieDigit;
	}

	void setNixieDigit(uint32_t nixieDigit) {
		this->nixieDigit = nixieDigit;
	}

	byte getBrightness() {
		return brightness;
	}

	void lockBrightness(const byte b) {
		lockedBrightness = true;
		brightness = b;
		pNixieDriver->setBrightness(brightness);
	}

	void unlockBrightness(const byte b) {
		lockedBrightness = false;
		setBrightness(b);
	}

protected:
	NixieDriver* pNixieDriver;

	ClockTimer::Timer displayTimer;
	int digitsOn = 1500;

	byte brightness = 100;
	bool lockedBrightness = false;
	NixieDriver::DisplayMode fadeMode = NixieDriver::NO_FADE;

	byte countSpeed = 60;
	uint32_t nixieDigit = 0;
};

#endif /* LIBRARIES_ONENIXIECLOCK_NIXIEDISPLAY_H_ */
