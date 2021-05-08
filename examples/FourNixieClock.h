/*
 * FourNixieClock.h
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#ifndef LIBRARIES_FOURNIXIECLOCK_FOURNIXIECLOCK_H_
#define LIBRARIES_FOURNIXIECLOCK_FOURNIXIECLOCK_H_
#include <Arduino.h>
#include <NixieDriver.h>
#include <NixieClock.h>

class FourNixieClock : public NixieClock {
public:
	FourNixieClock(NixieDriver* pNixieDriver) :
		NixieClock(pNixieDriver)
	{
		setNumDigits(10);
	}

	virtual void loop(unsigned long nowMs);

	virtual void setTimeMode(bool timeMode) {
		if (this->timeMode != timeMode) {
			this->timeMode = timeMode;
			initAlternateTime();
			displayTimer.setDuration(0);
		}
	}

	virtual void setAlternateTime(bool alternateTime) {
		if (!alternateTime) {	// Set display to default
			if (initAlternateTime()) {
				displayTimer.setDuration(0);
			}
		} else {
			toggleAlternateTime();
		}
	}

	virtual void toggleAlternateTime() {
		alternateTime = (alternateTime + 1) % 3;
		displayTimer.setDuration(0);
	}

	void oneColon(bool oneColon) {
		this->_oneColon = oneColon;
	}

private:
	bool initAlternateTime() {
		bool ret = false;
		if (timeMode) {	// default display is time
			if (alternateTime != 0) {
				alternateTime = 0;	// Display time
				ret = true;
			}
		} else {		// default display is month and day
			if (alternateTime != 1) {
				alternateTime = 1;	// Display month and day
				ret = true;
			}
		}

		return ret;
	}

	void doClock(unsigned long nowMs);
	void doCount(unsigned long nowMs);

	byte alternateTime = 0;
	byte colonMask = 0;
	bool _oneColon = false;
};


#endif /* LIBRARIES_FOURNIXIECLOCK_FOURNIXIECLOCK_H_ */
