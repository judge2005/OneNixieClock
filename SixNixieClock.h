/*
 * SixNixieClock.h
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#ifndef LIBRARIES_SIXNIXIECLOCK_SIXNIXIECLOCK_H_
#define LIBRARIES_SIXNIXIECLOCK_SIXNIXIECLOCK_H_
#include <Arduino.h>
#include <NixieDriver.h>
#include <NixieClock.h>
#include <Effect.h>

class SixNixieClock : public NixieClock {

public:
	SixNixieClock(NixieDriver* pNixieDriver) :
		NixieClock(pNixieDriver)
	{
		setNumDigits(6);
		pDivergence = new ClockEffects::Divergence(*this);
		pBubble = new ClockEffects::Bubble();
		pSlideLeft = new ClockEffects::SlideLeft();
		pScrollLeft = new ClockEffects::ScrollLeft();
		pFadeLeft = new ClockEffects::FadeLeft(*this);
		pSlideRight = new ClockEffects::SlideRight();
		pScrollRight = new ClockEffects::ScrollRight();
		pFadeRight = new ClockEffects::FadeRight(*this);
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

	bool hvOn = true;
	bool mov = true;

	byte alternateInterval = 0;	// In mins. 0 = never
	byte out_effect = 0;	// bubble
	byte in_effect = 0;	// bubble

	ClockEffects::Divergence *pDivergence = 0;
	ClockEffects::Bubble *pBubble = 0;
	ClockEffects::SlideLeft *pSlideLeft = 0;
	ClockEffects::ScrollLeft *pScrollLeft = 0;
	ClockEffects::FadeLeft *pFadeLeft = 0;
	ClockEffects::SlideRight *pSlideRight = 0;
	ClockEffects::ScrollRight *pScrollRight = 0;
	ClockEffects::FadeRight *pFadeRight = 0;

	ClockEffects::Effect *pCurrentEffect = 0;
};


#endif /* LIBRARIES_SIXNIXIECLOCK_SIXNIXIECLOCK_H_ */
