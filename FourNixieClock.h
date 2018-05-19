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

private:
	void doClock(unsigned long nowMs);
	void doCount(unsigned long nowMs);

	byte colonMask = 0;
};


#endif /* LIBRARIES_FOURNIXIECLOCK_FOURNIXIECLOCK_H_ */
