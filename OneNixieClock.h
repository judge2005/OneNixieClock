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

	OneNixieClock(NixieDriver* pNixieDriver, byte scrollBackDelay) :
		NixieClock(pNixieDriver), scrollBackDelay(scrollBackDelay)
	{
	}

	virtual void loop(unsigned long nowMs);

private:
	uint32_t getDigit();
	void doClock(unsigned long nowMs);
	void doCount(unsigned long nowMs);

	byte scrollBackDelay = 50;
};


#endif /* LIBRARIES_ONENIXIECLOCK_ONENIXIECLOCK_H_ */
