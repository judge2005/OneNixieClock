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

void CalcNixieClock::Bubble::init() {
}

byte CalcNixieClock::Bubble::getDelay() {
	return 100;
}

bool CalcNixieClock::Bubble::out(uint32_t target) {
	// Rotate current digits to nextNixieDigit
	byte digits[6];
	for (int i=0; i<6; i++) {
		// nixieDigit is BCD encoded. nextNixieDigit isn't.
		byte cd = clock.nixieDigit >> (i*4) & 0xf;	// current digit
		byte nd = target >> (i*4) & 0xf;	// next digit
		digits[i] = cd;
		if (cd != nd) {
			if (cd == 12) {
				digits[i] = nd;
			} else {
				digits[i] = (cd + 1) % 10;
			}
		}
	}

	clock.nixieDigit =
			  digits[0]
			+ digits[1] * 0x10
			+ digits[2] * 0x100
			+ digits[3] * 0x1000
			+ digits[4] * 0x10000
			+ digits[5] * 0x100000
					 ;

	return clock.nixieDigit == target;
}

bool CalcNixieClock::Bubble::in(uint32_t target) {
	// Rotate current digits to nextNixieDigit
	byte digits[6];
	for (int i=0; i<6; i++) {
		// nixieDigit is BCD encoded. nextNixieDigit isn't.
		byte cd = clock.nixieDigit >> (i*4) & 0xf;	// current digit
		byte nd = target >> (i*4) & 0xf;	// next digit
		digits[i] = cd;
		if (cd != nd) {
			if (nd == 12) {
				digits[i] = nd;
			} else {
				digits[i] = (cd + 9) % 10;
			}
		}
	}

	clock.nixieDigit =
			  digits[0]
			+ digits[1] * 0x10
			+ digits[2] * 0x100
			+ digits[3] * 0x1000
			+ digits[4] * 0x10000
			+ digits[5] * 0x100000
					 ;

	return clock.nixieDigit == target;
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

byte CalcNixieClock::Divergence::runLengths[6];

void CalcNixieClock::Divergence::init() {
	for (int i=0; i<6; i++) {
		runLengths[i] = RUN_LENGTH[random(64)];
	}

	adjustRL = true;
	pulse = 25;
	savedBrightness = clock.brightness;
}

byte CalcNixieClock::Divergence::getDelay() {
	return 25;
}

bool CalcNixieClock::Divergence::out(uint32_t target) {
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
	byte digits[6];
	bool allEqual = true;
	for (int i=0; i<6; i++) {
		byte cd = clock.nixieDigit >> (i*4) & 0xf;	// current digit
		byte nd = target >> (i*4) & 0xf;	// next digit
		digits[i] = cd;

		if (runLengths[i] - 1 != 0) {
			runLengths[i] -= 1;
			allEqual = false;

			if (cd == 12) {
				digits[i] = 0;	// If the current digit is a space, seed it with a number
			} else {
				digits[i] = (cd + 1) % 10;
			}
		} else  {
			// To be sure as this could be a space
			digits[i] = nd;
		}
	}

	clock.nixieDigit =
			  digits[0]
			+ digits[1] * 0x10
			+ digits[2] * 0x100
			+ digits[3] * 0x1000
			+ digits[4] * 0x10000
			+ digits[5] * 0x100000
					 ;

	if (allEqual && pulse != 0) {
		pulse -= 1;
		if (pulse == 0) {
			clock.unlockBrightness(savedBrightness);
		} else if (pulse < 20){
			byte pulseBrightness = savedBrightness * 2.5;
			if (pulseBrightness > 100) {
				pulseBrightness = 100;
			}
			clock.lockBrightness(pulseBrightness);
		}
	}

	return allEqual && (pulse == 0);
}

bool CalcNixieClock::Divergence::in(uint32_t target) {
	return out(target);
}

void CalcNixieClock::ScrollLeft::init() {
	iteration = 0;
}

byte CalcNixieClock::ScrollLeft::getDelay() {
	return 100;
}

bool CalcNixieClock::ScrollLeft::out(uint32_t target) {
	if (iteration == 13) {
		return true;
	}

	if (iteration >= 7) {
		// We have displayed all-spaces, start shifting target in from the 'left'
		clock.nixieDigit = target << (4 * (12-iteration));
		clock.nixieDigit |= 0xcccccc >> (4 * (iteration-6));
	} else {
		// Shift current digits to the 'left', pad with spaces
		clock.nixieDigit = clock.nixieDigit >> 4;
		clock.nixieDigit = (clock.nixieDigit & 0xfffff) | 0xc00000;
	}

	iteration++;

	return false;
}

bool CalcNixieClock::ScrollLeft::in(uint32_t target) {
	return out(target);
}

void CalcNixieClock::ScrollRight::init() {
	iteration = 0;
}

byte CalcNixieClock::ScrollRight::getDelay() {
	return 100;
}

bool CalcNixieClock::ScrollRight::out(uint32_t target) {
	if (iteration == 13) {
		return true;
	}

	if (iteration >= 7) {
		// We have displayed all-spaces, start shifting target in from the 'left'
		clock.nixieDigit = target >> (4 * (12-iteration));
		clock.nixieDigit |= (0xcccccc << (4 * (iteration-6))) & 0xffffff;
	} else {
		// Shift current digits to the 'right', pad with spaces
		clock.nixieDigit = clock.nixieDigit << 4;
		clock.nixieDigit = (clock.nixieDigit & 0xfffff0) | 0xc;
	}

	iteration++;

	return false;
}

bool CalcNixieClock::ScrollRight::in(uint32_t target) {
	return out(target);
}

void CalcNixieClock::FadeLeft::init() {
	iteration = 1;
	clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_FADE_OUT, -1);
}

byte CalcNixieClock::FadeLeft::getDelay() {
	return 50;
}

bool CalcNixieClock::FadeLeft::out(uint32_t target) {
	if (iteration == 2 && clock.getNixieDriver()->animationDone()) {
		clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_NONE, 0);
		return true;
	}

	if (iteration == 1 && clock.getNixieDriver()->animationDone()) {
		clock.nixieDigit = target;
		iteration = 2;
		clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_FADE_IN, -1);
	}

	return false;
}

bool CalcNixieClock::FadeLeft::in(uint32_t target) {
	return out(target);
}

void CalcNixieClock::FadeRight::init() {
	iteration = 1;
	clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_FADE_OUT, 1);
}

byte CalcNixieClock::FadeRight::getDelay() {
	return 50;
}

bool CalcNixieClock::FadeRight::out(uint32_t target) {
	if (iteration == 2 && clock.getNixieDriver()->animationDone()) {
		clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_NONE, 0);
		return true;
	}

	if (iteration == 1 && clock.getNixieDriver()->animationDone()) {
		clock.nixieDigit = target;
		iteration = 2;
		clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_FADE_IN, 1);
	}

	return false;
}

bool CalcNixieClock::FadeRight::in(uint32_t target) {
	return out(target);
}

void CalcNixieClock::SlideLeft::init() {
	i = j = 0;
}

byte CalcNixieClock::SlideLeft::getDelay() {
	return 50;
}

bool CalcNixieClock::SlideLeft::out(uint32_t target) {
	if (i == 12) {
		return true;
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
		uint32_t targetSrcMask = 0xf << (4*(i-7));
		uint32_t dstMask = 0xf00000 >> (4*m);
		uint32_t oldDstMask = (dstMask << 4) & 0xffffff;
		uint32_t digit = (target & targetSrcMask) << (4*j);
		// Set old position to blank
		if (oldDstMask != 0) {
			clock.nixieDigit = (clock.nixieDigit & ~oldDstMask) | (oldDstMask & 0xcccccc);
		}
		// Or in new digit
		clock.nixieDigit = ((clock.nixieDigit & ~dstMask) | digit) & 0xffffff;
	} else {
		/*
		 * i increments more slowly. i.e.
		 * for (int i=0; i<6; i++) {
		 *   for (int j=0; j<=i; j++) {
		 *   }
		 * }
		 */
		// Shift current digits to the 'left', pad with spaces
		// Select the correct digit:
		uint32_t srcMask = 0xf << (4*(i - j));
		uint32_t dstMask = srcMask >> 4;
		uint32_t digit = clock.nixieDigit & srcMask;
		// Set old position to blank
		clock.nixieDigit = (clock.nixieDigit & ~srcMask) | (srcMask & 0xcccccc);
		// Or in new digit
		if (dstMask != 0) {
			clock.nixieDigit = (clock.nixieDigit & ~dstMask) | (digit >> 4);
		}

		j++;
		if (j > i) {
			i++;
			j = 0;
		}
	}

	return false;
}

bool CalcNixieClock::SlideLeft::in(uint32_t target) {
	return out(target);
}

void CalcNixieClock::SlideRight::init() {
	i = j = 0;
}

byte CalcNixieClock::SlideRight::getDelay() {
	return 50;
}

bool CalcNixieClock::SlideRight::out(uint32_t target) {
	if (i == 12) {
		return true;
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
		uint32_t targetSrcMask = 0xf00000 >> (4*(i-7));
		uint32_t dstMask = 0xf << (4*m);
		uint32_t oldDstMask = (dstMask >> 4) & 0xffffff;
		uint32_t digit = (target & targetSrcMask) >> (4*j);
		// Set old position to blank
		if (oldDstMask != 0) {
			clock.nixieDigit = (clock.nixieDigit & ~oldDstMask) | (oldDstMask & 0xcccccc);
		}
		// Or in new digit
		clock.nixieDigit = ((clock.nixieDigit & ~dstMask) | digit) & 0xffffff;
	} else {
		/*
		 * i increments more slowly. i.e.
		 * for (int i=0; i<6; i++) {
		 *   for (int j=0; j<=i; j++) {
		 *   }
		 * }
		 */
		// Shift current digits to the 'left', pad with spaces
		// Select the correct digit:
		uint32_t srcMask = 0xf00000 >> (4*(i - j));
		uint32_t dstMask = (srcMask << 4) & 0xffffff;
		uint32_t digit = clock.nixieDigit & srcMask;
		// Set old position to blank
		clock.nixieDigit = (clock.nixieDigit & ~srcMask) | (srcMask & 0xcccccc);
		// Or in new digit
		if (dstMask != 0) {
			clock.nixieDigit = (clock.nixieDigit & ~dstMask) | (digit << 4);
		}

		j++;
		if (j > i) {
			i++;
			j = 0;
		}
	}

	return false;
}

bool CalcNixieClock::SlideRight::in(uint32_t target) {
	return out(target);
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
		suseconds_t uSec;
		pTimeSync->getLocalTime(&now, &uSec);
		suseconds_t realms = uSec / 1000;
#ifdef notdef
		if (realms > 1000) {
			realms = 0;	// Something went wrong so pick a safe number for 1000 - realms...
		}
#endif
		uint32_t oldNixieDigit = nixieDigit;

		secSnap = now.tm_sec;
		hourSnap = now.tm_hour;
		minSnap = now.tm_min;
		yearSnap = now.tm_year;
		monthSnap = now.tm_mon;
		daySnap = now.tm_mday;

		bool showDate = (alternateInterval != 0) && ((minSnap % alternateInterval) == 0) && (secSnap >= 45) && (secSnap <= 52);
		bool tick = false;

		uint32_t nextNixieDigit = 0;

		displayTimer.init(nowMs, 1000 - realms);

		if (twelveHour) {
			if (hourSnap > 12) {
				nextNixieDigit = (hourSnap - 12) * 10000;
			} else {
				nextNixieDigit = hourSnap * 10000;
			}

			if (hourSnap == 0) {
				nextNixieDigit = 120000;
			}
		} else {
			nextNixieDigit = hourSnap * 10000;
		}
		nextNixieDigit += minSnap * 100;
		nextNixieDigit += secSnap;

		// Get BCD representation of the time
		nextNixieDigit = bcdEncode(nextNixieDigit, false);
		tick = true;

		nixieDigit = nextNixieDigit;

		uint32_t date = 0;

		switch (dateFormat) {
		case 0:	// DD-MM-YY
			date = daySnap * 10000L + monthSnap * 100L + yearSnap;
			break;
		case 1: // MM-DD-YY
			date = monthSnap * 10000L + daySnap * 100L + yearSnap;
			break;
		default: // YY-MM-DD
			date = yearSnap * 10000L + monthSnap * 100L + daySnap;
			break;
		}

		date = bcdEncode(date, true);

		byte evenSec = secSnap & 1;
		uint64_t colons = DATE_COLONS << 32;
		switch (pNixieDriver->getIndicator()) {
		case 0: colons = 0; break;
		case 1: colons |= TIME_COLONS; break;
		case 2: if (evenSec) {colons |= TIME_COLONS;} break;
		default: if (hourSnap >= 12) {colons |= TIME_COLONS;}
		}

		if (oldNixieDigit != nixieDigit) {
			HV9808ESP32NixieDriverMultiplex *pMux = static_cast<HV9808ESP32NixieDriverMultiplex*>(pNixieDriver);
			pMux->setNewNixieDigit(nixieDigit);
			pMux->setDate(date);
			pMux->setColons(colons);
		}

		pNixieDriver->setColons(colonMask);
		if (tick && callback != 0) {
			callback(hourSnap * 3600 + minSnap * 60 + secSnap);
		}
	}
}
