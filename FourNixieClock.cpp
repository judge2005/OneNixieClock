/*
 * FourNixieClock.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#include <FourNixieClock.h>

void FourNixieClock::loop(unsigned long nowMs) {
	pNixieDriver->setDisplayOn(isOn() && hvOn && mov);

	if (clockMode) {
		doClock(nowMs);
	} else {
		doCount(nowMs);
	}
}

void FourNixieClock::setHV(bool hv) {
	this->hvOn = hv;
}

void FourNixieClock::setMov(bool mov) {
	this->mov = mov;
}

void FourNixieClock::setAlternateInterval(byte alternateInterval)
{
	this->alternateInterval = alternateInterval;
}

bool FourNixieClock::showAlternate(struct tm &now) {
	bool showAlternate = false;

	if (alternateInterval != 0) {
		if ((now.tm_min % alternateInterval) == 0) {
			showAlternate = now.tm_sec >= 45 && now.tm_sec <= 52;
		}
	}

	if (alternateTime) {
		return !showAlternate;
	}

	return showAlternate;
}

void FourNixieClock::setInEffect(byte effect)
{
	this->in_effect = effect;
}

void FourNixieClock::setOutEffect(byte effect)
{
	this->out_effect = effect;
}

void FourNixieClock::setCurrentEffect(byte effect)
{
	byte mode = effect;
	if (effect == 5) {	// random
		mode = random(0, 9); // Return 0 through 9 (9 effects because 5 has to be excluded)
		if (mode == 5) {
			mode += 1;	// Skip mode 5, because that means random!
		}
	}

	switch (mode) {
	case 1:
		pCurrentEffect = pSlideLeft;
		break;
	case 2:
		pCurrentEffect = pScrollLeft;
		break;
	case 3:
		pCurrentEffect = pSlideRight;
		break;
	case 4:
		pCurrentEffect = pScrollRight;
		break;
	case 6:
		pCurrentEffect = pDivergence;
		break;
	case 7:
		pCurrentEffect = pFadeLeft;
		break;
	case 8:
		pCurrentEffect = pFadeRight;
		break;
	case 9:
		pCurrentEffect = pNoAnimation;
		break;
	case 0:
	default:
		pCurrentEffect = pBubble;
		break;
	}
}

void FourNixieClock::doCount(unsigned long nowMs) {

	if (countSpeed == 0) {
		return;
	}

	if (displayTimer.expired(nowMs)) {
		displayTimer.init(nowMs, 60000 / countSpeed);
		pNixieDriver->setColons(nixieDigit % 2);
		nixieDigit = (nixieDigit + 1) % 10;
		pNixieDriver->setMode(fadeMode);
#ifdef ESP32
		pNixieDriver->setNewNixieDigit(
				nixieDigit +
				nixieDigit * 0x10 +
				nixieDigit * 0x100 +
				nixieDigit * 0x1000 +
				nixieDigit * 0x10000 +
				nixieDigit * 0x100000);
#else
		pNixieDriver->setNewNixieDigit(
				nixieDigit +
				((nixieDigit + 1) % 10) * 0x10 +
				((nixieDigit + 2) % 10) * 0x100 +
				((nixieDigit + 3) % 10) * 0x1000 +
				((nixieDigit + 4) % 10) * 0x10000 +
				((nixieDigit + 5) % 10) * 0x100000);
#endif
	}
}


void FourNixieClock::doClock(unsigned long nowMs) {
	if (displayTimer.expired(nowMs)) {
		struct tm now;
		suseconds_t uSec;
		pTimeSync->getLocalTime(&now, &uSec);
		hourSnap = now.tm_hour;	// Used by isOn()
		suseconds_t realms = uSec / 1000;
#ifdef notdef
		if (realms > 1000) {
			realms = 0;	// Something went wrong so pick a safe number for 1000 - realms...
		}
#endif
		bool tick = false;

		unsigned long tDelay = 1000 - realms;

		if (pCurrentEffect == 0) {
			// First time through
			setCurrentEffect(out_effect);
		}

		static bool previousShowTime = timeMode;

		uint64_t colons = 0;
		bool effectDone = true;

		bool showTime = timeMode != showAlternate(now);
		uint32_t oldNixieDigit = nixieDigit;
		uint32_t newNixieDigit = 0xcccccccc;

		if (showTime) {
			newNixieDigit = getFourDigitTime(now);
			effectDone = pCurrentEffect->in(nowMs, newNixieDigit);
		} else {
			newNixieDigit = getFourDigitDate(now);
			effectDone = pCurrentEffect->out(nowMs, newNixieDigit);
		}

		if (effectDone) {
			if (showTime != previousShowTime) {
				// Start transition
				// in_effect == hide date effect
				// out_effect == show date effect
				setCurrentEffect(previousShowTime ? out_effect : in_effect);
				pCurrentEffect->init(nowMs);
				pCurrentEffect->setCurrent(nixieDigit);
				previousShowTime = showTime;
				tDelay = 0;
			} else {
				pNixieDriver->setMode(fadeMode);
				nixieDigit = newNixieDigit;
				if (showTime) {
					colons = getColons(now);
					tick = true;
				}
			}
		} else {
			pNixieDriver->setMode(pCurrentEffect->getDisplayMode());
			nixieDigit = pCurrentEffect->getCurrent();
			if (pCurrentEffect->getDelay(nowMs) < tDelay) {
				tDelay = pCurrentEffect->getDelay(nowMs);
			}
		}

		displayTimer.init(nowMs, tDelay);

		if (oldNixieDigit != nixieDigit) {
			// Don't call this if nothing has changed - it will reset any transition timer in the driver
			pNixieDriver->setNewNixieDigit(nixieDigit);
		}
		pNixieDriver->setColons(colons);

		if (tick && callback != 0) {
			callback(now.tm_hour * 3600 + now.tm_min * 60 + now.tm_sec);
		}
	}
}

uint32_t FourNixieClock::getFourDigitTime(struct tm &now) {
	union {
		uint32_t packed;
		uint8_t bytes[4];
	} time;

	time.packed = 0xcccccccc;

	uint8_t hour = now.tm_hour;
	if (twelveHour) {
		if (now.tm_hour > 12) {
			hour = now.tm_hour - 12;
		} else if (now.tm_hour == 0) {
			hour = 12;
		}
	}

	if (hour < 10 && !leadingZero) {
		time.bytes[0] = 0x0c;
	} else {
		time.bytes[0] = hour / 10;
	}

	time.bytes[0] += (hour % 10) << 4;
	time.bytes[1] = now.tm_min / 10 + ((now.tm_min % 10) << 4);

	return time.packed;
}

uint32_t FourNixieClock::getFourDigitDate(struct tm &now) {
	union {
		uint32_t packed;
		uint8_t bytes[4];
	} date;

	date.packed = 0xcccccccc;

	uint8_t day = now.tm_mday / 10 + ((now.tm_mday % 10) << 4);
	uint8_t month = now.tm_mon / 10 + ((now.tm_mon % 10) << 4);
	uint8_t year = now.tm_year / 10 + ((now.tm_year % 10) << 4);

	switch (dateFormat) {
	case 0:	// DD-MM
		date.bytes[0] = day;
		date.bytes[1] = month;
		break;
	default: // MM-DD
		date.bytes[0] = month;
		date.bytes[1] = day;
		break;
	}

	return date.packed;
}

byte FourNixieClock::getColons(struct tm& now) {
	byte colons = 0;

	byte evenSec = now.tm_sec & 1;
	if (!_oneColon) {
		if (now.tm_sec < 15) {
			colons = 1;
			colons ^= evenSec;
		} else if (now.tm_sec < 30) {
			colons = 3;
			colons ^= evenSec << 1;
		} else if (now.tm_sec < 45) {
			colons = 7;
			colons ^= evenSec << 2;
		} else if (now.tm_sec < 60) {
			colons = 15;
			colons ^= evenSec << 3;
		}
	} else {
		switch (pNixieDriver->getIndicator()) {
		case 0: colons = 0; break;
		case 1: colons = 1; break;
		case 2: colons = evenSec; break;
		default: colons = (now.tm_hour >= 12);
		}
	}

	return colons;
}
