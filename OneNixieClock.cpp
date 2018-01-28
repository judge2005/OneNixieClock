/*
 * OneNixieClock.cpp
 *
 *  Created on: Dec 3, 2017
 *      Author: Paul Andrews
 */

#include <OneNixieClock.h>
#include <TimeLib.h>

void OneNixieClock::init() {
	timePart = 0;
	nextNixieDisplay = 0;
	nixieDigit = 0;
}

void OneNixieClock::setClockMode(bool clockMode) {
	if (clockMode != this->clockMode) {
		this->clockMode = clockMode;
		timePart = 0;
		acpCount = 0;
		time_t _now = now();
		monthSnap = month(_now);
		daySnap = day(_now);
		hourSnap = hour(_now);
		minSnap = minute(_now);
		secSnap = second(_now);
		nextNixieDisplay = millis();
	}
}

void OneNixieClock::setOnOff(byte on, byte off) {
	this->on = on;
	this->off = off;
}

bool OneNixieClock::isOn() {
	if (!clockMode) {
		return true;
	}

	if (on == off) {
		return true;
	}

	if (on < off) {
		return hourSnap >= on && hourSnap < off;
	}

	if (on > off) {
		return !(hourSnap >= off && hourSnap < on);
	}
}

void OneNixieClock::setDigit(byte digit) {
	if (!clockMode && this->nixieDigit != digit) {
		if (countSpeed != 0) {
			nextNixieDisplay = millis() + 60000 / countSpeed;
		}
		nixieDigit = digit;
		nixieDriver.setNewNixieDigit(nixieDigit);
	}
}

void OneNixieClock::setCountSpeed(byte countSpeed) {
	if (!clockMode && this->countSpeed != countSpeed) {
		if (countSpeed != 0) {
			if (this->countSpeed != 0) {
				int adj = 60000/countSpeed - 60000/this->countSpeed;

				nextNixieDisplay += adj;
			} else {
				nextNixieDisplay = millis() + 60000 / countSpeed;
			}
		}
	}

	this->countSpeed = countSpeed;
}

void OneNixieClock::loop(unsigned long nowMs) {
	if (clockMode) {
		doClock(nowMs);
	} else {
		doCount(nowMs);
	}
}

void OneNixieClock::doCount(unsigned long nowMs) {
	nixieDriver.setColons(0);

	if (countSpeed == 0) {
		return;
	}

	if (nowMs >= nextNixieDisplay) {
		nextNixieDisplay = nowMs + 60000 / countSpeed;
		nixieDigit = (nixieDigit + 1) % 10;
		nixieDriver.setMode(fadeMode);
		nixieDriver.setNewNixieDigit(nixieDigit);
	}
}

void OneNixieClock::doClock(unsigned long nowMs) {
	if (timePart == 6) {
		if (nowMs >= nextACP) {
			nixieDriver.setColons(0);
			nixieDriver.setMode(NixieDriver::NO_FADE);
			nextACP = nowMs + 50 - (nowMs - nextACP);
			if (scrollback) {
				nixieDigit = (nixieDigit + 1) % 10;
			} else {
				nixieDigit = 10;	// Off
			}
			nixieDriver.setNewNixieDigit(nixieDigit);
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
			nixieDriver.setMode(fadeMode);
			nextNixieDisplay = nowMs + digitsOn - (nowMs - nextNixieDisplay);
			switch (timePart) {
			case 0:
				colonMask = 1;
				if (fadeMode == NixieDriver::FADE_OUT)
					nixieDriver.setMode(NixieDriver::NO_FADE_DELAY);
				if (fadeMode == NixieDriver::FADE_OUT_IN)
					nixieDriver.setMode(NixieDriver::FADE_IN);
				if (timeMode) {
					if (twelveHour && hourSnap > 12) {
						nixieDigit = (hourSnap - 12) / 10;
					} else {
						nixieDigit = hourSnap / 10;
					}
				} else {
					if (dateFormat == 0) {
						nixieDigit = daySnap / 10;
					} else {
						nixieDigit = monthSnap / 10;
					}
				}
				break;
			case 1:
				colonMask = 1;
				if (timeMode) {
					if (twelveHour && hourSnap > 12) {
						nixieDigit = (hourSnap - 12) % 10;
					} else {
						nixieDigit = hourSnap % 10;
					}
				} else {
					if (dateFormat == 0) {
						nixieDigit = daySnap % 10;
					} else {
						nixieDigit = monthSnap % 10;
					}
				}
				break;
			case 2:
				colonMask = 2;
				if (timeMode) {
					nixieDigit = minSnap / 10;
				} else {
					if (dateFormat == 0) {
						nixieDigit = monthSnap / 10;
					} else {
						nixieDigit = daySnap / 10;
					}
				}
				break;
			case 3:
				colonMask = 2;
				if (timeMode) {
					nixieDigit = minSnap % 10;

					if (!showSeconds) {
						timePart = 5;
					}
				} else {
					if (dateFormat == 0) {
						nixieDigit = monthSnap % 10;
					} else {
						nixieDigit = daySnap % 10;
					}
					timePart = 5;
				}
				break;
			case 4:
				colonMask = 3;
				nixieDigit = secSnap / 10;
				break;
			case 5:
				colonMask = 3;
				nixieDigit = secSnap % 10;
				break;
			default:
				colonMask = 0;
				nixieDigit = 0;	// Display nothing
				break;
			}
			nixieDriver.setNewNixieDigit(nixieDigit);
			nixieDriver.setColons(colonMask);
			if (timePart == 5) {
				nextACP = nextNixieDisplay;
			}
			timePart = (timePart + 1) % 7;
		}
	}
}

