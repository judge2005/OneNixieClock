/*
 * SixNixieClock.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#include <SixNixieClock.h>

void SixNixieClock::loop(unsigned long nowMs) {
	pNixieDriver->setDisplayOn(isOn() && hvOn && mov);

	if (clockMode) {
		doClock(nowMs);
	} else {
		doCount(nowMs);
	}
}

void SixNixieClock::setHV(bool hv) {
	this->hvOn = hv;
}

void SixNixieClock::setMov(bool mov) {
	this->mov = mov;
}

void SixNixieClock::setAlternateInterval(byte alternateInterval)
{
	this->alternateInterval = alternateInterval;
}

void SixNixieClock::setInEffect(byte effect)
{
	this->in_effect = effect;
}

void SixNixieClock::setOutEffect(byte effect)
{
	this->out_effect = effect;
}

void SixNixieClock::setCurrentEffect(byte effect)
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

uint32_t SixNixieClock::bcdEncode(uint32_t digits, bool isDate) {
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

void SixNixieClock::Bubble::init() {
}

byte SixNixieClock::Bubble::getDelay() {
	return 100;
}

bool SixNixieClock::Bubble::out(uint32_t target) {
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

bool SixNixieClock::Bubble::in(uint32_t target) {
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

const byte SixNixieClock::Divergence::RUN_LENGTH[] = {
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

byte SixNixieClock::Divergence::runLengths[6];

void SixNixieClock::Divergence::init() {
	for (int i=0; i<6; i++) {
		runLengths[i] = RUN_LENGTH[random(64)];
	}

	adjustRL = true;
	pulse = 25;
	savedBrightness = clock.brightness;
}

byte SixNixieClock::Divergence::getDelay() {
	return 25;
}

bool SixNixieClock::Divergence::out(uint32_t target) {
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

bool SixNixieClock::Divergence::in(uint32_t target) {
	return out(target);
}

void SixNixieClock::ScrollLeft::init() {
	iteration = 0;
}

byte SixNixieClock::ScrollLeft::getDelay() {
	return 100;
}

bool SixNixieClock::ScrollLeft::out(uint32_t target) {
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

bool SixNixieClock::ScrollLeft::in(uint32_t target) {
	return out(target);
}

void SixNixieClock::ScrollRight::init() {
	iteration = 0;
}

byte SixNixieClock::ScrollRight::getDelay() {
	return 100;
}

bool SixNixieClock::ScrollRight::out(uint32_t target) {
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

bool SixNixieClock::ScrollRight::in(uint32_t target) {
	return out(target);
}

void SixNixieClock::FadeLeft::init() {
	iteration = 1;
	clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_FADE_OUT, -1);
}

byte SixNixieClock::FadeLeft::getDelay() {
	return 50;
}

bool SixNixieClock::FadeLeft::out(uint32_t target) {
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

bool SixNixieClock::FadeLeft::in(uint32_t target) {
	return out(target);
}

void SixNixieClock::FadeRight::init() {
	iteration = 1;
	clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_FADE_OUT, 1);
}

byte SixNixieClock::FadeRight::getDelay() {
	return 50;
}

bool SixNixieClock::FadeRight::out(uint32_t target) {
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

bool SixNixieClock::FadeRight::in(uint32_t target) {
	return out(target);
}

void SixNixieClock::SlideLeft::init() {
	i = j = 0;
}

byte SixNixieClock::SlideLeft::getDelay() {
	return 50;
}

bool SixNixieClock::SlideLeft::out(uint32_t target) {
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

bool SixNixieClock::SlideLeft::in(uint32_t target) {
	return out(target);
}

void SixNixieClock::SlideRight::init() {
	i = j = 0;
}

byte SixNixieClock::SlideRight::getDelay() {
	return 50;
}

bool SixNixieClock::SlideRight::out(uint32_t target) {
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

bool SixNixieClock::SlideRight::in(uint32_t target) {
	return out(target);
}

void SixNixieClock::doCount(unsigned long nowMs) {

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

void SixNixieClock::doClock(unsigned long nowMs) {
	static bool wasDate = false;
	static bool switchedBack = false;
	static uint32_t stashedTime = 0;

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

		if ((timeMode != alternateTime) && !showDate) {
			uint32_t nextNixieDigit = stashedTime;

			if (stashedTime == 0) {
				displayTimer.init(nowMs, 1000 - realms);

				pNixieDriver->setMode(fadeMode);

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
			}

			byte evenSec = secSnap & 1;
			switch (pNixieDriver->getIndicator()) {
			case 0: colonMask = 0; break;
			case 1: colonMask = 1; break;
			case 2: colonMask = evenSec; break;
			default: colonMask = hourSnap >= 12;
			}

			if (wasDate) {
				colonMask = 0;
				tick = false;

				if (switchedBack) {
					switchedBack = false;
					setCurrentEffect(in_effect);
					pCurrentEffect->init();
				}
				stashedTime = nextNixieDigit;
				if (pCurrentEffect->in(nextNixieDigit)) {
					stashedTime = 0;
					wasDate = false;
				} else {
					pNixieDriver->setMode(NixieDriver::DisplayMode::NO_FADE_DELAY);
					displayTimer.init(nowMs, pCurrentEffect->getDelay());
				}
			} else {
				nixieDigit = nextNixieDigit;
			}
		} else {
			if (!wasDate) {
				setCurrentEffect(out_effect);
				pCurrentEffect->init();
			}

			switchedBack = wasDate = true;
			pNixieDriver->setMode(NixieDriver::DisplayMode::NO_FADE_DELAY);

			displayTimer.init(nowMs, pCurrentEffect->getDelay());
			uint32_t nextNixieDigit = 0;

			switch (dateFormat) {
			case 0:	// DD-MM-YY
				nextNixieDigit = daySnap * 10000L + monthSnap * 100L + yearSnap;
				break;
			case 1: // MM-DD-YY
				nextNixieDigit = monthSnap * 10000L + daySnap * 100L + yearSnap;
				break;
			default: // YY-MM-DD
				nextNixieDigit = yearSnap * 10000L + monthSnap * 100L + daySnap;
				break;
			}

			nextNixieDigit = bcdEncode(nextNixieDigit, true);
			pCurrentEffect->out(nextNixieDigit);
			colonMask = 0;
		}

		nixieDigit |= secSnap % 2 * 0x1000000;	// Hidden digit to force it to change every second.

		if (oldNixieDigit != nixieDigit) {
			pNixieDriver->setNewNixieDigit(nixieDigit);
		}

		pNixieDriver->setColons(colonMask);
		if (tick && callback != 0) {
			callback(hourSnap * 3600 + minSnap * 60 + secSnap);
		}
	}
}
