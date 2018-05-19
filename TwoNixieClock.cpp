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

	if (nowMs >= nextNixieDisplay) {
		nextNixieDisplay = nowMs + 60000 / countSpeed;
		nixieDigit = (nixieDigit + 1) % 10;
		pNixieDriver->setMode(fadeMode);
		pNixieDriver->setNewNixieDigit(nixieDigit + nixieDigit * 16);
	}
}

void TwoNixieClock::doClock(unsigned long nowMs) {
	if (timePart == 6) {
		if (nowMs >= nextACP) {
			pNixieDriver->setColons(0);
			pNixieDriver->setMode(NixieDriver::NO_FADE);
			nextACP = nowMs + 50 - (nowMs - nextACP);
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
				nextNixieDisplay = nextACP;
			}
		}
	} else {
		if (nowMs >= nextNixieDisplay) {
			acpCount = 0;
			if (nextNixieDisplay + digitsOn < nowMs) {
				nextNixieDisplay = nowMs;
			}

			byte colonMask = 0;
			pNixieDriver->setMode(fadeMode);
			nextNixieDisplay = nowMs + digitsOn - (nowMs - nextNixieDisplay);
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
				nextACP = nextNixieDisplay;
			}
			timePart = (timePart + 2) % 7;
		}
	}
}

