/*
 * CalcNixieClock.h
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#ifndef LIBRARIES_CALCNIXIECLOCK_CALCNIXIECLOCK_H_
#define LIBRARIES_CALCNIXIECLOCK_CALCNIXIECLOCK_H_
#include <Arduino.h>
#include <NixieDriver.h>
#include <NixieClock.h>

class CalcNixieClock : public NixieClock {
	class Effect {
	public:
		virtual void init(unsigned long nowMs) = 0;
		virtual void setCurrent(uint32_t digits) = 0;
		virtual uint32_t getCurrent() = 0;
		virtual bool in(unsigned long nowMs, uint32_t digits) = 0;
		virtual bool out(unsigned long nowMs, uint32_t digits) = 0;
		virtual byte getDelay(unsigned long nowMs) = 0;
	};

	class EffectBase : public Effect {
	public:
		virtual void init(unsigned long nowMs);
		virtual void setCurrent(uint32_t digits);
		virtual uint32_t getCurrent();
		virtual byte getDelay(unsigned long nowMs);
	protected:
		byte period = 0;
		uint32_t current = 0xcccccccc;
		Timer effectTimer;
	};

	class ScrollRight : public EffectBase {
	public:
		ScrollRight() { period = 100; }
		virtual void init(unsigned long nowMs);
		virtual bool in(unsigned long nowMs, uint32_t digits);
		virtual bool out(unsigned long nowMs, uint32_t digits);

	private:
		byte iteration = 0;
	};

	class Divergence : public EffectBase {
	public:
		Divergence() { period = 25; }
		virtual void init(unsigned long nowMs);
		virtual bool in(unsigned long nowMs, uint32_t digits);
		virtual bool out(unsigned long nowMs, uint32_t digits);

	private:
		static const byte RUN_LENGTH[];
		static byte runLengths[];
		bool adjustRL = false;
		byte pulse = 0;
		byte savedBrightness;
	};

	class SlideRight : public EffectBase {
	public:
		SlideRight() { period = 50; }
		virtual void init(unsigned long nowMs);
		virtual bool in(unsigned long nowMs, uint32_t digits);
		virtual bool out(unsigned long nowMs, uint32_t digits);

	private:
		byte i = 0;
		byte j = 0;
	};

public:
	CalcNixieClock(NixieDriver* pNixieDriver) :
		NixieClock(pNixieDriver)
	{
		setNumDigits(6);
		pScrollRight = new ScrollRight();
		pSlideRight = new SlideRight();
		pDivergence = new Divergence();
		pCurrentEffect = pScrollRight;
	}

	virtual void loop(unsigned long nowMs);
	virtual void setHV(bool hv);
	virtual void setMov(bool mov);
	virtual void setAlternateInterval(byte alternateInterval);
	virtual void setOutEffect(byte effect);
	virtual void setInEffect(byte effect);
	virtual void setBrightness(const byte b) {
		if (!lockedBrightness) {
			brightness = b;
			pNixieDriver->setBrightness(brightness);
		}
	}

protected:
	void lockBrightness(const byte b) {
		lockedBrightness = true;
		brightness = b;
		pNixieDriver->setBrightness(brightness);
	}

	void unlockBrightness(const byte b) {
		lockedBrightness = false;
		setBrightness(b);
	}

private:
	uint32_t bcdEncode(uint32_t digits, bool isDate);
	void doClock(unsigned long nowMs);
	void doCount(unsigned long nowMs);
	void setCurrentEffect(byte effect);
	uint32_t getSixDigitTime(struct tm &now);
	uint32_t getSixDigitDate(struct tm &now);
	uint32_t getEightDigitTime(struct tm& now);
	uint64_t getColons(struct tm& now);

	bool hvOn = true;
	bool mov = true;
	bool lockedBrightness = false;

	byte colonMask = 0;
	byte alternateInterval = 0;	// In mins. 0 = never
	byte out_effect = 0;	// bubble
	byte in_effect = 0;	// bubble

	ScrollRight *pScrollRight = 0;
	Divergence *pDivergence = 0;
	SlideRight *pSlideRight = 0;

	Effect *pCurrentEffect = 0;
};


#endif /* LIBRARIES_CALCNIXIECLOCK_CALCNIXIECLOCK_H_ */
