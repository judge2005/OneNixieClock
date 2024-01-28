/*
 * BambuDisplay.h
 *
 *  Created on: Nov 22, 2023
 *      Author: pandrews
 */

#ifndef LIBRARIES_ONENIXIECLOCK_BAMBUDISPLAY_H_
#define LIBRARIES_ONENIXIECLOCK_BAMBUDISPLAY_H_

#include <NixieDisplay.h>

namespace Bambu {
	const uint8_t off = 0;
	const uint8_t idle = 1;
	const uint8_t heating = 2;
	const uint8_t cleaning = 3;
	const uint8_t scanning = 4;
	const uint8_t leveling = 5;
	const uint8_t calibrating = 6;
	const uint8_t printing = 7;
	const uint8_t error = 8;
	const uint8_t unknown = 255;
}

class BambuDisplay: public NixieDisplay {
public:
	BambuDisplay(NixieDriver* pNixieDriver) :
		NixieDisplay(pNixieDriver) {
	}

	virtual void loop(unsigned long nowMs);

	void setState(const uint8_t state) { this->state = state; }
	uint8_t getState() { return state; }

protected:
	uint8_t state = Bambu::off;
};
#endif /* LIBRARIES_ONENIXIECLOCK_BAMBUDISPLAY_H_ */
