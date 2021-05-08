/*
 * SixNixieClock.h
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#ifndef LIBRARIES_FOURNIXIECLOCK_FOURNIXIECLOCK_H_
#define LIBRARIES_FOURNIXIECLOCK_FOURNIXIECLOCK_H_
#include <Arduino.h>
#include <NixieDriver.h>
#include <NixieClock.h>
#include <Effect.h>

class FourNixieClock : public NixieClock {

public:
	FourNixieClock(NixieDriver* pNixieDriver) :
		NixieClock(pNixieDriver)
	{
		setNumDigits(4);
		pNoAnimation = new ClockEffects::NoAnimation();
		pDivergence = new ClockEffects::Divergence(*this, 4);
		pBubble = new ClockEffects::Bubble(4);
		pSlideLeft = new ClockEffects::SlideLeft(4);
		pScrollLeft = new ClockEffects::ScrollLeft(4);
		pFadeLeft = new ClockEffects::SimpleFadeLeft(4);
		pSlideRight = new ClockEffects::SlideRight(4);
		pScrollRight = new ClockEffects::ScrollRight(4);
		pFadeRight = new ClockEffects::SimpleFadeRight(4);

		pCurrentEffect = pNoAnimation;
	}

	virtual void loop(unsigned long nowMs);
	virtual void setHV(bool hv);
	virtual void setMov(bool mov);
	virtual void setAlternateInterval(byte alternateInterval);
	virtual bool showAlternate(struct tm &now);
	virtual void setOutEffect(byte effect);
	virtual void setInEffect(byte effect);
	virtual void oneColon(bool oneColon) {
		this->_oneColon = oneColon;
	}

private:
	void doClock(unsigned long nowMs);
	void doCount(unsigned long nowMs);
	void setCurrentEffect(byte effect);
	uint32_t getFourDigitTime(struct tm &now);
	uint32_t getFourDigitDate(struct tm &now);
	byte getColons(struct tm &now);

	bool hvOn = true;
	bool mov = true;

	byte alternateInterval = 0;	// In mins. 0 = never
	byte out_effect = 0;	// bubble
	byte in_effect = 0;	// bubble
	bool _oneColon = false;

	ClockEffects::Effect *pNoAnimation = 0;
	ClockEffects::Effect *pDivergence = 0;
	ClockEffects::Effect *pBubble = 0;
	ClockEffects::Effect *pSlideLeft = 0;
	ClockEffects::Effect *pScrollLeft = 0;
	ClockEffects::Effect *pFadeLeft = 0;
	ClockEffects::Effect *pSlideRight = 0;
	ClockEffects::Effect *pScrollRight = 0;
	ClockEffects::Effect *pFadeRight = 0;

	ClockEffects::Effect *pCurrentEffect = 0;
};


#endif /* LIBRARIES_FOURNIXIECLOCK_FOURNIXIECLOCK_H_ */
