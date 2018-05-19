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
#include <NixieClock.h>

class OneNixieClock : public NixieClock {
public:
	OneNixieClock(NixieDriver* pNixieDriver) :
		NixieClock(pNixieDriver)
	{
	}

	virtual void loop(unsigned long nowMs);

private:
	uint16_t getDigit();
	void doClock(unsigned long nowMs);
	void doCount(unsigned long nowMs);
};


#endif /* LIBRARIES_ONENIXIECLOCK_ONENIXIECLOCK_H_ */
