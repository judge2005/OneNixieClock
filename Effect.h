/*
 * Effect.h
 *
 *  Created on: Sep 26, 2020
 *      Author: mpand
 */

#ifndef LIBRARIES_ONENIXIECLOCK_EFFECT_H_
#define LIBRARIES_ONENIXIECLOCK_EFFECT_H_

#include <stdint.h>
#include "Timer.h"
#include "NixieClock.h"
#include "NixieDriver.h"

namespace ClockEffects {

class Effect {
public:
	virtual void init(unsigned long nowMs) = 0;
	virtual void setCurrent(uint32_t digits) = 0;
	virtual uint32_t getCurrent() = 0;
	virtual bool in(unsigned long nowMs, uint32_t digits) = 0;
	virtual bool out(unsigned long nowMs, uint32_t digits) = 0;
	virtual uint16_t getDelay(unsigned long nowMs) = 0;
	virtual NixieDriver::DisplayMode getDisplayMode() { return NixieDriver::DisplayMode::NO_FADE; }
};

class NoAnimation : public Effect {
public:
	virtual void init(unsigned long nowMs);
	virtual void setCurrent(uint32_t digits);
	virtual uint32_t getCurrent();
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);
	virtual uint16_t getDelay(unsigned long nowMs);

protected:
	uint32_t current = 0xcccccccc;
};

class EffectBase : public Effect {
public:
	virtual void init(unsigned long nowMs);
	virtual void setCurrent(uint32_t digits);
	virtual uint32_t getCurrent();
	virtual uint16_t getDelay(unsigned long nowMs);
	virtual void setNumDigits(byte numDigits) { this->numDigits = numDigits; }
protected:
	byte numDigits = 6;
	uint16_t period = 0;
	uint32_t current = 0xcccccccc;
	ClockTimer::Timer effectTimer;
};

class Divergence : public EffectBase {
public:
	Divergence(NixieClock &clock, byte numDigits) : clock(clock) { setNumDigits(numDigits); period = 30; }
	Divergence(NixieClock &clock) : Divergence(clock, 6) {}
	virtual void init(unsigned long nowMs);
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);

private:
	static const uint8_t RUN_LENGTH[];
	static uint8_t runLengths[];
	bool adjustRL = false;
	uint8_t pulse = 0;
	uint8_t savedBrightness;
	NixieClock &clock;
};

class Bubble : public EffectBase {
public:
	Bubble(byte numDigits) { setNumDigits(numDigits); period = 100; }
	Bubble() : Bubble(6) {}
	virtual void init(unsigned long nowMs);
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);
};

class ScrollLeft : public EffectBase {
public:
	ScrollLeft(byte numDigits) { setNumDigits(numDigits); period = 100; }
	ScrollLeft() : ScrollLeft(6) {}
	virtual void init(unsigned long nowMs);
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);

private:

	uint8_t iteration = 0;
};

class ScrollRight : public EffectBase {
public:
	ScrollRight(byte numDigits) { setNumDigits(numDigits); period = 100; }
	ScrollRight() : ScrollRight(6) {}
	virtual void init(unsigned long nowMs);
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);

private:

	uint8_t iteration = 0;
};

class FadeLeft : public EffectBase {
public:
	FadeLeft(NixieClock &clock, byte numDigits) : clock(clock) { setNumDigits(numDigits); period = 50; }
	FadeLeft(NixieClock &clock) : FadeLeft(clock, 6) {}
	virtual void init(unsigned long nowMs);
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);

private:
	NixieClock &clock;
	uint8_t iteration = 0;
};

class FadeRight : public EffectBase {
public:
	FadeRight(NixieClock &clock, byte numDigits) : clock(clock) { setNumDigits(numDigits); period = 50; }
	FadeRight(NixieClock &clock) : FadeRight(clock, 6) {}
	virtual void init(unsigned long nowMs);
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);

private:
	NixieClock &clock;
	uint8_t iteration = 0;
};

class SimpleFadeLeft : public EffectBase {
public:
	SimpleFadeLeft(byte numDigits) { setNumDigits(numDigits); period = 350; }
	SimpleFadeLeft() : SimpleFadeLeft(6) {}
	virtual void init(unsigned long nowMs);
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);
	virtual NixieDriver::DisplayMode getDisplayMode() { return NixieDriver::DisplayMode::CROSS_FADE_FAST; }

private:
	uint8_t iteration = 0;
};

class SimpleFadeRight : public EffectBase {
public:
	SimpleFadeRight(byte numDigits) { setNumDigits(numDigits); period = 350; }
	SimpleFadeRight() : SimpleFadeRight(6) {}
	virtual void init(unsigned long nowMs);
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);
	virtual NixieDriver::DisplayMode getDisplayMode() { return NixieDriver::DisplayMode::CROSS_FADE_FAST; }

private:
	uint8_t iteration = 0;
};

class SlideLeft : public EffectBase {
public:
	SlideLeft(byte numDigits) { setNumDigits(numDigits); period = 50; }
	SlideLeft() : SlideLeft(6) {}
	virtual void init(unsigned long nowMs);
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);

private:

	uint8_t i = 0;
	uint8_t j = 0;
};

class SlideRight : public EffectBase {
public:
	SlideRight(byte numDigits) { setNumDigits(numDigits); period = 50; }
	SlideRight() : SlideRight(6) {}
	virtual void init(unsigned long nowMs);
	virtual bool in(unsigned long nowMs, uint32_t digits);
	virtual bool out(unsigned long nowMs, uint32_t digits);

private:

	uint8_t i = 0;
	uint8_t j = 0;
};

} /* namespace ClockEffects */

#endif /* LIBRARIES_ONENIXIECLOCK_EFFECT_H_ */
