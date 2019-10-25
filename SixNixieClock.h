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

class SixNixieClock : public NixieClock {
	class Effect {
	public:
		virtual void init() = 0;
		virtual bool in(uint32_t digits) = 0;
		virtual bool out(uint32_t digits) = 0;
		virtual byte getDelay() = 0;
	};

	class Bubble : public Effect {
	public:
		Bubble(SixNixieClock &clock) : clock(clock) {}
		virtual void init();
		virtual bool in(uint32_t digits);
		virtual bool out(uint32_t digits);
		virtual byte getDelay();

	private:
		SixNixieClock &clock;
	};

	class Divergence : public Effect {
	public:
		Divergence(SixNixieClock &clock) : clock(clock) {}
		virtual void init();
		virtual bool in(uint32_t digits);
		virtual bool out(uint32_t digits);
		virtual byte getDelay();

	private:
		static const byte RUN_LENGTH[];
		static byte runLengths[];
		bool adjustRL = false;
		byte pulse = 0;
		byte savedBrightness;

		SixNixieClock &clock;
	};

	class ScrollLeft : public Effect {
	public:
		ScrollLeft(SixNixieClock &clock) : clock(clock) {}
		virtual void init();
		virtual bool in(uint32_t digits);
		virtual bool out(uint32_t digits);
		virtual byte getDelay();

	private:
		SixNixieClock &clock;
		byte iteration = 0;
	};

	class ScrollRight : public Effect {
	public:
		ScrollRight(SixNixieClock &clock) : clock(clock) {}
		virtual void init();
		virtual bool in(uint32_t digits);
		virtual bool out(uint32_t digits);
		virtual byte getDelay();

	private:
		SixNixieClock &clock;
		byte iteration = 0;
	};

	class SlideLeft : public Effect {
	public:
		SlideLeft(SixNixieClock &clock) : clock(clock) {}
		virtual void init();
		virtual bool in(uint32_t digits);
		virtual bool out(uint32_t digits);
		virtual byte getDelay();

	private:
		SixNixieClock &clock;
		byte i = 0;
		byte j = 0;
	};

	class SlideRight : public Effect {
	public:
		SlideRight(SixNixieClock &clock) : clock(clock) {}
		virtual void init();
		virtual bool in(uint32_t digits);
		virtual bool out(uint32_t digits);
		virtual byte getDelay();

	private:
		SixNixieClock &clock;
		byte i = 0;
		byte j = 0;
	};

public:
	SixNixieClock(NixieDriver* pNixieDriver) :
		NixieClock(pNixieDriver)
	{
		setNumDigits(6);
		pDivergence = new Divergence(*this);
		pBubble = new Bubble(*this);
		pSlideLeft = new SlideLeft(*this);
		pScrollLeft = new ScrollLeft(*this);
		pSlideRight = new SlideRight(*this);
		pScrollRight = new ScrollRight(*this);
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

	bool hvOn = true;
	bool mov = true;
	bool lockedBrightness = false;

	byte colonMask = 0;
	byte alternateInterval = 0;	// In mins. 0 = never
	byte out_effect = 0;	// bubble
	byte in_effect = 0;	// bubble

	Divergence *pDivergence = 0;
	Bubble *pBubble = 0;
	SlideLeft *pSlideLeft = 0;
	ScrollLeft *pScrollLeft = 0;
	SlideRight *pSlideRight = 0;
	ScrollRight *pScrollRight = 0;

	Effect *pCurrentEffect = 0;
};


#endif /* LIBRARIES_SIXNIXIECLOCK_SIXNIXIECLOCK_H_ */
