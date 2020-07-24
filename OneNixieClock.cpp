/*
 * OneNixieClock.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#include <OneNixieClock.h>
#include <TimeLib.h>

void OneNixieClock::loop(unsigned long nowMs) {
	if (clockMode) {
		doClock(nowMs);
	} else {
		doCount(nowMs);
	}
}

void OneNixieClock::setScrollBackDelay(byte delay) {
	scrollBackDelay = delay;
}

void OneNixieClock::doCount(unsigned long nowMs) {
	pNixieDriver->setColons(0);

	if (countSpeed == 0) {
		return;
	}

	if (displayTimer.expired(nowMs)) {
		displayTimer.init(nowMs, 60000 / countSpeed);

		nixieDigit = (nixieDigit + 1) % numDigits;
		pNixieDriver->setMode(fadeMode);
		pNixieDriver->setNewNixieDigit(nixieDigit);
	}
}

uint32_t OneNixieClock::getDigit() {
	uint32_t digit = 0;

	switch (timePart) {
	case 0:
		if (fadeMode == NixieDriver::FADE_OUT)
			pNixieDriver->setMode(NixieDriver::NO_FADE_DELAY);
		if (fadeMode == NixieDriver::FADE_OUT_IN)
			pNixieDriver->setMode(NixieDriver::FADE_IN);
		// Fall through
	case 6:
		if (timeMode != alternateTime) {
			if (twelveHour && hourSnap > 12) {
				digit = (hourSnap - 12) / 10;
			} else {
				digit = hourSnap / 10;
			}
		} else {
			if (dateFormat == 0) {
				digit = daySnap / 10;
			} else {
				digit = monthSnap / 10;
			}
		}
		break;
	case 1:
		if (timeMode != alternateTime) {
			if (twelveHour && hourSnap > 12) {
				digit = (hourSnap - 12) % 10;
			} else {
				digit = hourSnap % 10;
			}
		} else {
			if (dateFormat == 0) {
				digit = daySnap % 10;
			} else {
				digit = monthSnap % 10;
			}
		}
		break;
	case 2:
		if (timeMode != alternateTime) {
			digit = minSnap / 10;
		} else {
			if (dateFormat == 0) {
				digit = monthSnap / 10;
			} else {
				digit = daySnap / 10;
			}
		}
		break;
	case 3:
		if (timeMode != alternateTime) {
			digit = minSnap % 10;

			if (!showSeconds) {
				timePart = 5;
			}
		} else {
			if (dateFormat == 0) {
				digit = monthSnap % 10;
			} else {
				digit = daySnap % 10;
			}
			timePart = 5;
		}
		break;
	case 4:
		digit = secSnap / 10;
		break;
	case 5:
		digit = secSnap % 10;
		break;
	default:
		digit = 0;	// Display nothing
		break;
	}

	return digit;
}

void OneNixieClock::doClock(unsigned long nowMs) {
	if (timePart == 6) {
		if (displayTimer.expired(nowMs)) {
			pNixieDriver->setColons(0);
			time_t _now = now();
			monthSnap = month(_now);
			daySnap = day(_now);
			hourSnap = hour(_now);
			minSnap = minute(_now);
			secSnap = second(_now);
			uint32_t nextClockDigit = getDigit();
			if (pNixieDriver->setTransition(scrollback ? 2 : 1, nextClockDigit)) {
				pNixieDriver->setMode(NixieDriver::NO_FADE);
				displayTimer.reset(scrollBackDelay);
//				Serial.printf("A: %d:%d\n", displayTimer.getLastTick(), displayTimer.getDuration());
			} else {
				displayTimer.reset(0);
				pNixieDriver->setTransition(0, nextClockDigit);
				timePart = 0;
			}
		}
	}

	if (timePart != 6) {
		if (displayTimer.expired(nowMs)) {
			displayTimer.init(nowMs, digitsOn);
			byte colonMask = 0;
			switch (timePart) {
			case 0:
			case 1:
				colonMask = 1;
				break;
			case 2:
			case 3:
				colonMask = 2;
				break;
			case 4:
			case 5:
				colonMask = 3;
				break;
			default:
				colonMask = 0;
				break;
			}

			nixieDigit = getDigit();	// May change timepart

			pNixieDriver->setMode(fadeMode);
			pNixieDriver->setTransition(0, nixieDigit);
			pNixieDriver->setNewNixieDigit(nixieDigit);
			acpCount = 0;

			pNixieDriver->setColons(colonMask);
			timePart = (timePart + 1) % 7;
//			if (timePart == 6) {
//				Serial.printf("B: %d:%d\n", displayTimer.getLastTick(), displayTimer.getDuration());
//			}
		}
	}
}

