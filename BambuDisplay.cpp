/*
 * BambuDisplay.cpp
 *
 *  Created on: Nov 22, 2023
 *      Author: pandrews
 */

#include <BambuDisplay.h>

void BambuDisplay::loop(unsigned long nowMs) {
	if (state != Bambu::off) {
		if (displayTimer.expired(nowMs)) {
			pNixieDriver->setDisplayOn(true);
			pNixieDriver->setMode(fadeMode);
			pNixieDriver->setNewNixieDigit(nixieDigit);

			unsigned long q = nowMs / 250;	// quarter-seconds
			unsigned long h = nowMs / 500;	// half-seconds
			unsigned long s = nowMs / 1000;	// seconds
			switch (state) {
			case Bambu::printing:
				pNixieDriver->setColons(6 + (q % 4));	// Rotate once per second
				break;
			case Bambu::idle:
				pNixieDriver->setColons(6 + (s % 4));	// Rotate once every four seconds
				break;
			case Bambu::heating:
			case Bambu::cleaning:
			case Bambu::scanning:
			case Bambu::leveling:
			case Bambu::calibrating:
				pNixieDriver->setColons(4 + (s % 2));	// Alternate left right once every two seconds
				break;
			case Bambu::error:
				pNixieDriver->setColons(h % 2);	// on/off once per second
				break;
			default:
				pNixieDriver->setColons(2 + (s % 2));	// top/bottom once every two seconds
				break;
			}

			displayTimer.init(nowMs, 250);
		}
	}
}

