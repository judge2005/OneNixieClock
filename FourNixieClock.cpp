/*
 * FourNixieClock.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#include <FourNixieClock.h>
#include <TimeLib.h>

void FourNixieClock::loop(unsigned long nowMs) {
	if (clockMode) {
		doClock(nowMs);
	} else {
		doCount(nowMs);
	}
}

void FourNixieClock::doCount(unsigned long nowMs) {
	pNixieDriver->setColons(0);

	if (countSpeed == 0) {
		return;
	}

	if (displayTimer.expired(nowMs)) {
		displayTimer.init(nowMs, 60000 / countSpeed);
		nixieDigit = (nixieDigit + 1) % 10;
		pNixieDriver->setMode(fadeMode == NixieDriver::NO_FADE_DELAY ? NixieDriver::NO_FADE : fadeMode);
		pNixieDriver->setNewNixieDigit(nixieDigit + nixieDigit * 16 + nixieDigit * 256 + nixieDigit * 4096);
	}
}

void FourNixieClock::doClock(unsigned long nowMs) {
	if (displayTimer.expired(nowMs)) {
		displayTimer.init(nowMs, 1000);

		pNixieDriver->setMode(fadeMode == NixieDriver::NO_FADE_DELAY ? NixieDriver::NO_FADE : fadeMode);
		time_t _now = now();
		monthSnap = month(_now);
		daySnap = day(_now);
		hourSnap = hour(_now);
		minSnap = minute(_now);
		secSnap = second(_now);

		uint32_t oldNixieDigit = nixieDigit;

		if (timeMode != alternateTime) {
			if (twelveHour) {
				if (hourSnap > 12) {
					nixieDigit = (hourSnap - 12) * 100;
				} else {
					nixieDigit = hourSnap * 100;
				}

				if (hourSnap == 0) {
					nixieDigit = 1200;
				}
			} else {
				nixieDigit = hourSnap * 100;
			}
			nixieDigit += minSnap;

			byte evenSec = secSnap & 1;
			if (secSnap < 15) {
				colonMask = 1;
				colonMask ^= evenSec;
			} else if (secSnap < 30) {
				colonMask = 3;
				colonMask ^= evenSec << 1;
			} else if (secSnap < 45) {
				colonMask = 7;
				colonMask ^= evenSec << 2;
			} else if (secSnap < 60) {
				colonMask = 15;
				colonMask ^= evenSec << 3;
			}
		} else {
			if (dateFormat == 0) {
				nixieDigit = daySnap * 100 + monthSnap;
			} else {
				nixieDigit = monthSnap * 100 + daySnap;
			}

			colonMask = 0;
		}
		// Convert to BCD
		nixieDigit = (nixieDigit % 10) * 4096
				+ ((nixieDigit / 10) % 10) * 256
				+ ((nixieDigit / 100) % 10) * 16
				+ ((nixieDigit / 1000) % 10);

		if (oldNixieDigit != nixieDigit) {
			pNixieDriver->setNewNixieDigit(nixieDigit);
		}

		pNixieDriver->setColons(colonMask);
	}
}

