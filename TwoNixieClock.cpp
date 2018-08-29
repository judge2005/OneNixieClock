/*
 * TwoNixieClock.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#include <TwoNixieClock.h>
#include <TimeLib.h>

void TwoNixieClock::loop(unsigned long nowMs) {
	if (clockMode) {
		doClock(nowMs);
	} else {
		doCount(nowMs);
	}
}

void TwoNixieClock::doCount(unsigned long nowMs) {
	pNixieDriver->setColons(0);

	if (countSpeed == 0) {
		return;
	}

	if (displayTimer.expired(nowMs)) {
		displayTimer.init(nowMs, 60000 / countSpeed);
		nixieDigit = (nixieDigit + 1) % 10;
		pNixieDriver->setMode(fadeMode);
		pNixieDriver->setNewNixieDigit(nixieDigit + nixieDigit * 16);
	}
}

void TwoNixieClock::doClock(unsigned long nowMs) {
	if (timePart == 6) {
		if (displayTimer.expired(nowMs)) {
			displayTimer.init(nowMs, 50);
			pNixieDriver->setColons(0);
			pNixieDriver->setMode(NixieDriver::NO_FADE);
			if (scrollback) {
				nixieDigit = (nixieDigit + 1) % 10;
			} else {
				nixieDigit = 10;	// Off
			}
			pNixieDriver->setNewNixieDigit(nixieDigit + nixieDigit * 16);
			if (++acpCount >= 10) {
				timePart = 0;
				time_t _now = now();
				monthSnap = month(_now);
				daySnap = day(_now);
				hourSnap = hour(_now);
				minSnap = minute(_now);
				secSnap = second(_now);
				displayTimer.init(nowMs, 0);
			}
		}
	} else {
		if (displayTimer.expired(nowMs)) {
			acpCount = 0;

			byte colonMask = 0;
			pNixieDriver->setMode(fadeMode);
			displayTimer.init(nowMs, digitsOn);

			switch (timePart) {
			case 0:
				colonMask = 1;
				if (fadeMode == NixieDriver::FADE_OUT)
					pNixieDriver->setMode(NixieDriver::NO_FADE_DELAY);
				if (fadeMode == NixieDriver::FADE_OUT_IN)
					pNixieDriver->setMode(NixieDriver::FADE_IN);
				if (timeMode != alternateTime) {
					if (twelveHour && hourSnap > 12) {
						nixieDigit = hourSnap - 12;
					} else {
						nixieDigit = hourSnap;
					}
				} else {
					if (dateFormat == 0) {
						nixieDigit = daySnap;
					} else {
						nixieDigit = monthSnap;
					}
				}
				break;
			case 2:
				colonMask = 2;
				if (timeMode != alternateTime) {
					nixieDigit = minSnap;
					if (!showSeconds) {
						timePart = 4;
					}
				} else {
					if (dateFormat == 0) {
						nixieDigit = monthSnap;
					} else {
						nixieDigit = daySnap;
					}
					timePart = 4;
				}
				break;
			case 4:
				colonMask = 3;
				nixieDigit = secSnap;
				break;
			default:
				colonMask = 0;
				nixieDigit = 0;	// Display nothing
				break;
			}
			// Convert to BCD
			nixieDigit = ((nixieDigit % 10) * 16) + (nixieDigit / 10);
			pNixieDriver->setNewNixieDigit(nixieDigit);
			pNixieDriver->setColons(colonMask);
			if (timePart == 4) {
				nixieDigit = 0;
			}
			timePart = (timePart + 2) % 7;
		}
	}
}

