/*
 * TwoNixieClock.h
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#ifndef LIBRARIES_TWONIXIECLOCK_TWONIXIECLOCK_H_
#define LIBRARIES_TWONIXIECLOCK_TWONIXIECLOCK_H_
#include <Arduino.h>
#include <NixieDriver.h>
#include <NixieClock.h>

class TwoNixieClock : public NixieClock {
public:
	TwoNixieClock(NixieDriver* pNixieDriver) :
		NixieClock(pNixieDriver)
	{
	}

	virtual void loop(unsigned long nowMs);

private:
	void doClock(unsigned long nowMs);
	void doCount(unsigned long nowMs);
};


#endif /* LIBRARIES_TWONIXIECLOCK_TWONIXIECLOCK_H_ */
