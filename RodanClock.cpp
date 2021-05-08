/*
 * RodanClock.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#include <RodanClock.h>

void RodanClock::loop(unsigned long nowMs) {
	pNixieDriver->setDisplayOn(isOn() && hvOn && mov);

	if (clockMode) {
		doClock(nowMs);
	} else {
		doCount(nowMs);
	}
}

void RodanClock::setHV(bool hv) {
	this->hvOn = hv;
}

void RodanClock::setMov(bool mov) {
	this->mov = mov;
}

void RodanClock::setAlternateInterval(byte alternateInterval)
{
	this->alternateInterval = alternateInterval;
}

bool RodanClock::showAlternate(struct tm &now) {
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

void RodanClock::setInEffect(byte effect)
{
	this->in_effect = effect;
}

void RodanClock::setOutEffect(byte effect)
{
	this->out_effect = effect;
}

void RodanClock::setCurrentEffect(byte effect)
{
	byte mode = effect;
	if (effect == 5) {	// random
#ifdef ITS1A
		mode = random(0, 5); // Return 0 through 4 (5 effects)
#else
		mode = random(0, 9); // Return 0 through 8 (8 effects because 5 has to be excluded)
		if (mode == 5) {
			mode += 1;	// Skip mode 5, because that means random!
		}
#endif
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
	case 0:
	default:
		pCurrentEffect = pBubble;
		break;
	}
}

uint32_t RodanClock::bcdEncode(uint32_t digits, bool isDate) {
	uint32_t hours = (digits / 100000L) % 10;
	uint32_t encoded = digits =
			  (digits % 10) * 0x100000
			+ ((digits / 10) % 10) * 0x10000
			+ ((digits / 100) % 10) * 0x1000
			+ ((digits / 1000) % 10) * 0x100
			+ ((digits / 10000) % 10) * 0x10;

	if (hours == 0 && !leadingZero && !isDate) {
		hours = 12;	// Blank
	}
	digits += hours;

	return digits;

}

void RodanClock::doCount(unsigned long nowMs) {

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


void RodanClock::doClock(unsigned long nowMs) {
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
			newNixieDigit = getSixDigitTime(now);
			effectDone = pCurrentEffect->in(nowMs, newNixieDigit);
		} else {
			newNixieDigit = getSixDigitDate(now);
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
			nixieDigit = (pCurrentEffect->getCurrent() & 0xff00ffff) | (nixieDigit & 0xff0000);
			if (pCurrentEffect->getDelay(nowMs) < tDelay) {
				tDelay = pCurrentEffect->getDelay(nowMs);
			}
		}

		displayTimer.init(nowMs, tDelay);

		pNixieDriver->setNewNixieDigit(nixieDigit);
		pNixieDriver->setColons(colons);

		if (tick && callback != 0) {
			callback(now.tm_hour * 3600 + now.tm_min * 60 + now.tm_sec);
		}
	}
}

uint8_t RodanClock::getMSB(struct tm &now, bool noAMPM) {
	uint8_t am_pm = 0xc;	// No am/pm
	if (twelveHour && !noAMPM) {
		am_pm = 0;	// AM
		if (now.tm_hour >= 12) {
			am_pm = 1;	// PM
		}
	}

		// day of week [0,6] (Sunday = 0)
	uint8_t low_nibble = am_pm;
	switch (now.tm_wday) {
	case 0:
		if (low_nibble == 0xc) {	// No AM/PM
			low_nibble = 8;
		} else {
			low_nibble += 10;
		}
		break;		// 0, 1, 8, 10, 11
	case 1:
		if (low_nibble == 0xc) {	// No AM/PM
			low_nibble = 9;
		} else {
			low_nibble += 13;
		}
		break;	// 0, 1, 9, 13, 14
	}

	uint8_t high_nibble = 0xc;
	if (now.tm_wday >= 2) {
		high_nibble = now.tm_wday - 2;
	}

	return low_nibble +  (high_nibble << 4);
}

uint32_t RodanClock::getSixDigitTime(struct tm &now) {
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

	time.bytes[2] = getMSB(now, false);

	return time.packed;
}

uint32_t RodanClock::getSixDigitDate(struct tm &now) {
	union {
		uint32_t packed;
		uint8_t bytes[4];
	} date;

	date.packed = 0xcccccccc;

	uint8_t day = now.tm_mday / 10 + ((now.tm_mday % 10) << 4);
	uint8_t month = now.tm_mon / 10 + ((now.tm_mon % 10) << 4);
	uint8_t year = now.tm_year / 10 + ((now.tm_year % 10) << 4);

	date.bytes[2] = getMSB(now, true);

	switch (dateFormat) {
	case 0:	// DD-MM-YY
		date.bytes[0] = day;
		date.bytes[1] = month;
		break;
	default: // MM-DD-YY
		date.bytes[0] = month;
		date.bytes[1] = day;
		break;
	}

	return date.packed;
}

byte RodanClock::getColons(struct tm& now) {
	byte colons = 0;

	switch (pNixieDriver->getIndicator()) {
	case 1: colons = 1; break;
	case 2: colons = now.tm_sec & 1; break;
	case 3: colons = (now.tm_hour >= 12); break;
	case 4: now.tm_sec & 1 ? colons = 2 : colons = 3; break;
	case 5: now.tm_sec & 1 ? colons = 4 : colons = 5; break;
	default: colons = 0; break;
	}

	return colons;
}
