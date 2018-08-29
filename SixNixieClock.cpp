/*
 * SixNixieClock.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#include <SixNixieClock.h>
#include <TimeLib.h>

void SixNixieClock::loop(unsigned long nowMs) {
	pNixieDriver->setDisplayOn(isOn() && hvOn);

	if (clockMode) {
		doClock(nowMs);
	} else {
		doCount(nowMs);
	}
}

void SixNixieClock::setHV(bool hv) {
	this->hvOn = hv;
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
	byte mode = random(0, 6);	// Return 0 through 5
	if (effect != 5) {
		mode = effect;	// Not random after all
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
		pNixieDriver->setNewNixieDigit(
				nixieDigit +
				((nixieDigit + 1) % 10) * 0x10 +
				((nixieDigit + 2) % 10) * 0x100 +
				((nixieDigit + 3) % 10) * 0x1000 +
				((nixieDigit + 4) % 10) * 0x10000 +
				((nixieDigit + 5) % 10) * 0x100000);
	}
}

void SixNixieClock::doClock(unsigned long nowMs) {
	static bool wasDate = false;
	static bool switchedBack = false;
	static uint32_t stashedTime = 0;

	if (displayTimer.expired(nowMs)) {
		time_t _now = now();

		uint32_t oldNixieDigit = nixieDigit;

		secSnap = second(_now);
		hourSnap = hour(_now);
		minSnap = minute(_now);
		yearSnap = year(_now) % 100;
		monthSnap = month(_now);
		daySnap = day(_now);
		bool showDate = (alternateInterval != 0) && ((minSnap % alternateInterval) == 0) && (secSnap >= 45) && (secSnap <= 52);

		if ((timeMode != alternateTime) && !showDate) {
			uint32_t nextNixieDigit = stashedTime;

			if (stashedTime == 0) {
				displayTimer.init(nowMs, 1000);

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
			}

			byte evenSec = secSnap & 1;
			switch (pNixieDriver->getIndicator()) {
			case 0: colonMask = 0; break;
			case 1: colonMask = 1; break;
			case 2: colonMask = evenSec; break;
			default: colonMask = isPM(_now);
			}

			if (wasDate) {
				colonMask = 0;

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
	}
}
