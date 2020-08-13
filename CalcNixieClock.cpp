/*
 * CalcNixieClock.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#include <CalcNixieClock.h>
#include <HV9808ESP32NixieDriverMultiplex.h>

void CalcNixieClock::loop(unsigned long nowMs) {
	pNixieDriver->setDisplayOn(isOn() && hvOn && mov);

	if (clockMode) {
		doClock(nowMs);
	} else {
		doCount(nowMs);
	}
}

void CalcNixieClock::setHV(bool hv) {
	this->hvOn = hv;
}

void CalcNixieClock::setMov(bool mov) {
	this->mov = mov;
}

void CalcNixieClock::setAlternateInterval(byte alternateInterval)
{
	this->alternateInterval = alternateInterval;
}

void CalcNixieClock::setInEffect(byte effect)
{
	this->in_effect = effect;
}

void CalcNixieClock::setOutEffect(byte effect)
{
	this->out_effect = effect;
}

void CalcNixieClock::setCurrentEffect(byte effect)
{
	byte mode = effect;
	if (effect == 5) {	// random
		mode = random(0, 8); // Return 0 through 7 (8 effects)
		if (mode == 5) {
			mode += 1;	// Skip mode 5, because that means random!
		}
	}

	pCurrentEffect = pSlideRight;
}

uint32_t CalcNixieClock::bcdEncode(uint32_t digits, bool isDate) {
	uint32_t hours = (digits / 100000L) % 10;

	if (hours == 0 && !leadingZero && !isDate) {
		hours = 0xc;	// Blank
	}

	if (isDate) {
		digits =
			((digits % 10) << 28)
			+ (((digits / 10) % 10) << 24)
			+ (((digits / 100) % 10) << 20)
			+ (((digits / 1000) % 10) << 16)
			+ (((digits / 10000) % 10) << 12)
			+ (hours << 8)
			+ (0xc << 4)
			+ 0xc
			;
	} else {
		digits =
			  ((digits % 10) << 28)
			+ (((digits / 10) % 10) << 24)
			+ (0xc << 20)
			+ (((digits / 100) % 10) << 16)
			+ (((digits / 1000) % 10) << 12)
			+ (0xc << 8)
			+ (((digits / 10000) % 10) << 4)
			+ hours;
			;
	}

	return digits;
}

void CalcNixieClock::EffectBase::init(unsigned long nowMs) {
	current = 0xcccccccc;
	effectTimer.setEnabled(true);
	effectTimer.init(nowMs, 0);
}

void CalcNixieClock::EffectBase::setCurrent(uint32_t digits) {
	current = digits;
}

uint32_t CalcNixieClock::EffectBase::getCurrent() {
	return current;
}

byte CalcNixieClock::EffectBase::getDelay(unsigned long nowMs) {
	return nowMs - effectTimer.getLastTick() + period;
}

const byte CalcNixieClock::Divergence::RUN_LENGTH[] = {
// Lookup Table for run lengths (appropriate random number in W... 0-7, 0-15, 0-63)
// The first 8 are multiples of 10, for use when tube 7 returns to starting digit.
// The first 16 are multiples of 5, for use when two cycles return digits to same.
// The rest of the 64 are spaced to give good stopping distribution.
		20,
		30,
		40,
		50,
		50,
		60,
		60,
		70,

		15,
		25,
		35,
		45,
		55,
		55,
		55,
		65,

		18,
		19,
		22,
		23,
		24,
		26,
		27,
		28,

		29,
		31,
		32,
		33,
		34,
		36,
		37,
		38,

		39,
		41,
		42,
		43,
		44,
		46,
		47,
		48,

		49,
		16,
		51,
		17,
		52,
		68,
		53,
		67,

		21,
		69,
		54,
		56,
		56,
		64,
		57,
		57,

		66,
		58,
		58,
		63,
		59,
		59,
		61,
		62
};

byte CalcNixieClock::Divergence::runLengths[8];

void CalcNixieClock::Divergence::init(unsigned long nowMs) {
	EffectBase::init(nowMs);

	for (int i=0; i<8; i++) {
		runLengths[i] = RUN_LENGTH[random(64)];
	}

	adjustRL = true;
	pulse = 25;
//	savedBrightness = clock.brightness;
}

bool CalcNixieClock::Divergence::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

#ifdef notdef
	if (adjustRL) {
		adjustRL = false;

		// Adjust run lengths by difference between current and target
		for (int i=0; i<6; i++) {
			// nixieDigit is BCD encoded. nextNixieDigit isn't.
			byte cd = clock.nixieDigit >> (i*4) & 0xf;	// current digit
			byte nd = target >> (i*4) & 0xf;	// next digit

			// cd is 12 for a blank. Leave this as is.
			if (cd != 12 && nd != 12) {
				if (nd > cd) {
					runLengths[i] += nd - cd;
				} else {
					runLengths[i] += nd + 10 - cd;
				}
			}
		}

	}
#endif

	// Rotate current digits to nextNixieDigit
	byte digits[8];
	bool allEqual = true;
	for (int i=0; i<8; i++) {
		byte cd = current >> (i*4) & 0xf;	// current digit
		byte nd = target >> (i*4) & 0xf;	// next digit
		digits[i] = cd;

		if (runLengths[i] - 1 != 0) {
			runLengths[i] -= 1;
			allEqual = false;

			if (cd == 0xc) {
				digits[i] = 0;	// If the current digit is a space, seed it with a number
			} else {
				digits[i] = (cd + 1) % 10;
			}
		} else  {
			// To be sure as this could be a space
			digits[i] = nd;
		}
	}

	current =
			  digits[0]
			+ (digits[1] << 4)
			+ (digits[2] << 8)
			+ (digits[3] << 12)
			+ (digits[4] << 16)
			+ (digits[5] << 20)
			+ (digits[6] << 24)
			+ (digits[7] << 28)
		;

	if (allEqual && pulse != 0) {
		pulse -= 1;
		if (pulse == 0) {
//			clock.unlockBrightness(savedBrightness);
		} else if (pulse < 20){
			byte pulseBrightness = savedBrightness * 2.5;
			if (pulseBrightness > 100) {
				pulseBrightness = 100;
			}
//			clock.lockBrightness(pulseBrightness);
		}
	}

	bool done = allEqual && (pulse == 0);

	if (done) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return done;
}

bool CalcNixieClock::Divergence::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}


void CalcNixieClock::ScrollRight::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
	iteration = 0;
}

bool CalcNixieClock::ScrollRight::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (iteration >= 7) {
		// We have displayed all-spaces, start shifting target in from the 'left'
		current = target << (4 * (12-iteration));
		current |= 0xcccccccc >> (4 * (iteration-6));
	} else {
		// Shift current digits to the 'right', pad with spaces
		current = current << 4;
		current = (current & 0xfffff000) | 0xccc;
	}

	iteration++;

	if (iteration == 13) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool CalcNixieClock::ScrollRight::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

void CalcNixieClock::SlideRight::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
	i = j = 0;
}

bool CalcNixieClock::SlideRight::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (i >= 6) {

		if (j == 0) {
			i++;
			j = 13 - i;
		}
		j--;
		byte l = 12 - i;
		byte m = l-j;

		/*
		 * l decrements more slowly. i.e.
		 * for (int l=5; l>=0; l--) {
		 *   for (int m=0; m<l; m++) {
		 *   }
		 * }
		 */
		// Shift current digits to the 'left', pad with spaces
		// Select the correct digit:
		uint32_t targetSrcMask = 0xf00 << (4*(i-7));
		uint32_t dstMask = 0xf0000000 >> (4*m);
		uint32_t oldDstMask = (dstMask << 4) & 0xffffffff;
		uint32_t digit = (target & targetSrcMask) << (4*j);
		// Set old position to blank
		if (oldDstMask != 0) {
			current = (current & ~oldDstMask) | (oldDstMask & 0xcccccccc);
		}
		// Or in new digit
		current = ((current & ~dstMask) | digit) & 0xffffffff;
	} else {
		/*
		 * i increments more slowly. i.e.
		 * for (int i=0; i<6; i++) {
		 *   for (int j=0; j<=i; j++) {
		 *   }
		 * }
		 */
		// Shift current digits to the 'right', pad with spaces
		// Select the correct digit:
		uint32_t srcMask = 0xf0000000 >> (4*(i - j));
		uint32_t dstMask = (srcMask << 4) & 0xffffffff;
		uint32_t digit = current & srcMask;
		// Set old position to blank
		current = (current & ~srcMask) | (srcMask & 0xcccccccc);
		// Or in new digit
		if (dstMask != 0) {
			current = (current & ~dstMask) | (digit << 4);
		}

		j++;
		if (j > i) {
			i++;
			j = 0;
		}
	}

	if (i == 12) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool CalcNixieClock::SlideRight::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

void CalcNixieClock::doCount(unsigned long nowMs) {

	if (countSpeed == 0) {
		return;
	}

	if (displayTimer.expired(nowMs)) {
		displayTimer.init(nowMs, 60000 / countSpeed);
		pNixieDriver->setColons(nixieDigit % 2);
		nixieDigit = (nixieDigit + 1) % 10;
		pNixieDriver->setMode(fadeMode);
#ifdef ESP32
		uint32_t digit =
				nixieDigit +
				(nixieDigit << 4) +
				(nixieDigit << 8) +
				(nixieDigit << 12) +
				(nixieDigit << 16) +
				(nixieDigit << 20) +
				(nixieDigit << 24) +
				(nixieDigit << 28)
				;

		HV9808ESP32NixieDriverMultiplex *pMux = static_cast<HV9808ESP32NixieDriverMultiplex*>(pNixieDriver);
		pMux->setNewNixieDigit(digit);
		pMux->setDate(digit);

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

static uint64_t TIME_COLONS = 1ULL << 8 | 1ULL << 20;
static uint64_t DATE_COLONS = 1ULL << 12 | 1ULL << 20;

void CalcNixieClock::doClock(unsigned long nowMs) {
	if (displayTimer.expired(nowMs)) {
		struct tm now;
		struct tm altTime;
		suseconds_t uSec;
		pTimeSync->getLocalTime(&now, &uSec);
		pTimeSync->getTimeWithTz("GMT0BST,M3.5.0/1,M10.5.0", &altTime, NULL);
		suseconds_t realms = uSec / 1000;
#ifdef notdef
		if (realms > 1000) {
			realms = 0;	// Something went wrong so pick a safe number for 1000 - realms...
		}
#endif
		bool tick = false;

		unsigned long tDelay = 1000 - realms;

		// Get BCD representation of the time
		uint64_t colons = getColons(now);
		nixieDigit = getEightDigitTime(now);	// Left 8 digits of display
		if (pDivergence->out(nowMs, nixieDigit)) {
			// Not in transition
			if (now.tm_sec == 0) {
				// Start transition
				pDivergence->init(nowMs);
				pDivergence->setCurrent(nixieDigit);
				tDelay = 0;
			} else {
				tick = true;
			}
		} else {
			nixieDigit = pDivergence->getCurrent();
			colons &= 0xffffffff00000000UL;
			tDelay = pDivergence->getDelay(nowMs);
		}

		static bool showAlternate = false;

		uint32_t date = showAlternate ? getSixDigitTime(altTime) : getSixDigitDate(now);	// Right 8 digits of display
		if (pCurrentEffect->out(nowMs, date)) {
			// not in transition
			if ((alternateInterval != 0) && ((now.tm_sec % alternateInterval) == 0)) {
				// Start transition
				pCurrentEffect->init(nowMs);
				pCurrentEffect->setCurrent(date);
				showAlternate = !showAlternate;
				tDelay = 0;
			} else {
				tick = true;
			}
		} else {
			date = pCurrentEffect->getCurrent();
			colons &= 0xffffffff;
			if (pCurrentEffect->getDelay(nowMs) < tDelay) {
				tDelay = pCurrentEffect->getDelay(nowMs);
			}
		}

		displayTimer.init(nowMs, tDelay);

		HV9808ESP32NixieDriverMultiplex *pMux = static_cast<HV9808ESP32NixieDriverMultiplex*>(pNixieDriver);
		pMux->setNewNixieDigit(nixieDigit);
		pMux->setDate(date);
		pMux->setColons(colons);

		if (tick && callback != 0) {
			callback(now.tm_hour * 3600 + now.tm_min * 60 + now.tm_sec);
		}
	}
}

uint32_t CalcNixieClock::getSixDigitTime(struct tm &now) {
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
		time.bytes[1] = 0x0c;
	} else {
		time.bytes[1] = hour / 10;
	}

	time.bytes[1] += (hour % 10) << 4;
	time.bytes[2] = now.tm_min / 10 + ((now.tm_min % 10) << 4);
	time.bytes[3] = now.tm_sec / 10 + ((now.tm_sec % 10) << 4);

	return time.packed;
}

uint32_t CalcNixieClock::getSixDigitDate(struct tm &now) {
	union {
		uint32_t packed;
		uint8_t bytes[4];
	} date;

	date.packed = 0xcccccccc;

	uint8_t day = now.tm_mday / 10 + ((now.tm_mday % 10) << 4);
	uint8_t month = now.tm_mon / 10 + ((now.tm_mon % 10) << 4);
	uint8_t year = now.tm_year / 10 + ((now.tm_year % 10) << 4);

	switch (dateFormat) {
	case 0:	// DD-MM-YY
		date.bytes[1] = day;
		date.bytes[2] = month;
		date.bytes[3] = year;
		break;
	case 1: // MM-DD-YY
		date.bytes[1] = month;
		date.bytes[2] = day;
		date.bytes[3] = year;
		break;
	default: // YY-MM-DD
		date.bytes[1] = year;
		date.bytes[2] = month;
		date.bytes[3] = day;
		break;
	}

	return date.packed;
}

uint32_t CalcNixieClock::getEightDigitTime(struct tm &now) {
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
	time.bytes[1] = ((now.tm_min / 10) << 4) + 0x0c;
	time.bytes[2] = 0xc0 + (now.tm_min % 10);
	time.bytes[3] = now.tm_sec / 10 + ((now.tm_sec % 10) << 4);

	return time.packed;
}

uint64_t CalcNixieClock::getColons(struct tm& now) {
	uint64_t colons = DATE_COLONS << 32;

	switch (pNixieDriver->getIndicator()) {
	case 0: colons = 0; break;
	case 1: colons |= TIME_COLONS; break;
	case 2: if (now.tm_sec & 1) {colons |= TIME_COLONS;} break;
	default: if (now.tm_hour >= 12) {colons |= TIME_COLONS;}
	}

	return colons;
}
