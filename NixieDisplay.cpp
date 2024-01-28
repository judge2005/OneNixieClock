/*
 * NixieDisplay.cpp
 *
 *  Created on: Nov 22, 2023
 *      Author: pandrews
 */

#include <NixieDisplay.h>

void NixieDisplay::init() {
	displayTimer.init(millis(), 0);
	nixieDigit = 0xcccccccc;
}

void NixieDisplay::setDigit(const uint32_t digit) {
	if (this->nixieDigit != digit) {
		if (countSpeed != 0) {
			displayTimer.init(millis(), 60000 / countSpeed);
		}
		nixieDigit = digit;
		pNixieDriver->setNewNixieDigit(nixieDigit);
	}
}

void NixieDisplay::setCountSpeed(byte countSpeed) {
	if (this->countSpeed != countSpeed) {
		if (countSpeed != 0) {
			if (this->countSpeed != 0) {
				int adj = 60000/countSpeed - 60000/this->countSpeed;

				displayTimer.init(millis(), adj > 0 ? adj : 0);
			} else {
				displayTimer.init(millis(), 60000 / countSpeed);
			}
		}
	}

	this->countSpeed = countSpeed;
}
