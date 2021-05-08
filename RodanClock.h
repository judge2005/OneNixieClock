/*
 * RodanClock.h
 *
 *  Created on: Mar 23, 2021
 *      Author: Paul Andrews
 */

#ifndef LIBRARIES_RODANCLOCK_RODANCLOCK_H_
#define LIBRARIES_RODANCLOCK_RODANCLOCK_H_
#include <Arduino.h>
#include <NixieDriver.h>
#include <NixieClock.h>
#include <Effect.h>

class RodanClock : public NixieClock {

public:
	RodanClock(NixieDriver* pNixieDriver) :
		NixieClock(pNixieDriver)
	{
		setNumDigits(6);
		pDivergence = new ClockEffects::Divergence(*this, 4);
		pBubble = new ClockEffects::Bubble(4);
		pSlideLeft = new ClockEffects::SlideLeft(4);
		pScrollLeft = new ClockEffects::ScrollLeft(4);
		pFadeLeft = new ClockEffects::SimpleFadeLeft(4);
		pSlideRight = new ClockEffects::SlideRight(4);
		pScrollRight = new ClockEffects::ScrollRight(4);
		pFadeRight = new ClockEffects::SimpleFadeRight(4);
	}

	virtual void loop(unsigned long nowMs);
	virtual void setHV(bool hv);
	virtual void setMov(bool mov);
	virtual void setAlternateInterval(byte alternateInterval);
	virtual bool showAlternate(struct tm &now);
	virtual void setOutEffect(byte effect);
	virtual void setInEffect(byte effect);

private:
	uint32_t bcdEncode(uint32_t digits, bool isDate);
	void doClock(unsigned long nowMs);
	void doCount(unsigned long nowMs);
	void setCurrentEffect(byte effect);
	uint32_t getSixDigitTime(struct tm &now);
	uint32_t getSixDigitDate(struct tm &now);
	byte getColons(struct tm &now);
	uint8_t getMSB(struct tm &now, bool noAMPM);
	bool hvOn = true;
	bool mov = true;

	byte alternateInterval = 0;	// In mins. 0 = never
	byte out_effect = 0;	// bubble
	byte in_effect = 0;	// bubble

	ClockEffects::Divergence *pDivergence = 0;
	ClockEffects::Bubble *pBubble = 0;
	ClockEffects::SlideLeft *pSlideLeft = 0;
	ClockEffects::ScrollLeft *pScrollLeft = 0;
	ClockEffects::SimpleFadeLeft *pFadeLeft = 0;
	ClockEffects::SlideRight *pSlideRight = 0;
	ClockEffects::ScrollRight *pScrollRight = 0;
	ClockEffects::SimpleFadeRight *pFadeRight = 0;

	ClockEffects::Effect *pCurrentEffect = 0;
};


#endif /* LIBRARIES_RODANCLOCK_RODANCLOCK_H_ */
