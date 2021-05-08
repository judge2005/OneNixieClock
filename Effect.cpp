/*
 * Effect.cpp
 *
 *  Created on: Sep 26, 2020
 *      Author: mpand
 */

#include <Effect.h>

namespace ClockEffects {

void NoAnimation::init(unsigned long nowMs) {
}

void NoAnimation::setCurrent(uint32_t digits) {
	current = digits;
}

uint32_t NoAnimation::getCurrent() {
	return current;
}

uint16_t NoAnimation::getDelay(unsigned long nowMs) {
	return 0;
}

bool NoAnimation::out(unsigned long nowMs, uint32_t target) {
	current = target;
	return true;
}

bool NoAnimation::in(unsigned long nowMs, uint32_t target) {
	current = target;
	return true;
}

void EffectBase::init(unsigned long nowMs) {
	current = 0xcccccccc;
	effectTimer.setEnabled(true);
	effectTimer.init(nowMs, 0);
}

void EffectBase::setCurrent(uint32_t digits) {
	current = digits;
}

uint32_t EffectBase::getCurrent() {
	return current;
}

uint16_t EffectBase::getDelay(unsigned long nowMs) {
	return nowMs - effectTimer.getLastTick() + period;
}

void FadeLeft::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
	iteration = 1;
	clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_FADE_OUT, -1);
}

bool FadeLeft::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (iteration == 2 && clock.getNixieDriver()->animationDone()) {
		clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_NONE, 0);
		effectTimer.setEnabled(false);
	} else if (iteration == 1 && clock.getNixieDriver()->animationDone()) {
		current = target;
		iteration = 2;
		clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_FADE_IN, -1);
	}

	if (effectTimer.isEnabled()) {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool FadeLeft::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

void FadeRight::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
	iteration = 1;
	clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_FADE_OUT, 1);
}

bool FadeRight::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (iteration == 2 && clock.getNixieDriver()->animationDone()) {
		clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_NONE, 0);
		effectTimer.setEnabled(false);
	} else if (iteration == 1 && clock.getNixieDriver()->animationDone()) {
		current = target;
		iteration = 2;
		clock.getNixieDriver()->setAnimation(NixieDriver::Animation::ANIMATION_FADE_IN, 1);
	}

	if (effectTimer.isEnabled()) {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool FadeRight::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

void SimpleFadeLeft::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
	iteration = 0;
}

bool SimpleFadeLeft::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (iteration >= numDigits) {
		// Feed the target in from the 'left'
		int shift = 4 * (numDigits*2 - iteration - 1);
		current = target & (0xffffff << shift) & 0xffffff;
		current = (current | (0xcccccc >> (4 * (iteration - numDigits + (7-numDigits))))) & 0xffffff;
	} else {
		// Feed spaces in from the 'left'
		int shift = 4 * (iteration+(6-numDigits));
		current = current & ~(0xf00000 >> shift) & 0xffffff;
		current = current | (0xc00000 >> shift);
	}

	iteration++;

	if (iteration == numDigits * 2) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool SimpleFadeLeft::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

void SimpleFadeRight::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
	iteration = 0;
}

bool SimpleFadeRight::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (iteration >= numDigits) {
		// Feed the target in from the 'right'
		int shift = 4 * (numDigits - iteration + 5);
		current = target & 0xffffff >> shift ;
		current = (current | (0xcccccc << (4 * (iteration-numDigits+1)))) & 0xffffff;

	} else {
		// Feed spaces in from the 'right'
		int shift = 4 * iteration;
		current = current & ~(0xf << shift) & 0xffffff;
		current = current | (0xc << shift);
	}

	iteration++;

	if (iteration == numDigits * 2) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool SimpleFadeRight::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

void ScrollRight::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
	iteration = 0;
}

bool ScrollRight::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (numDigits == 4) {
		target = (target & 0xffff) | 0xcccc0000;
	} else if (numDigits == 6) {
		target = (target & 0xffffff) | 0xcc000000;
	}

	if (iteration >= numDigits + 1) {
		// We have displayed all-spaces, start shifting target in from the 'left'
		current = target >> (4 * (numDigits*2-iteration));
		current |= (0xcccccc << (4 * (iteration-numDigits))) & 0xffffff;
	} else {
		// Shift current digits to the 'right', pad with spaces
		current = current << 4;
		current = (current & 0xfffff0) | 0xc;
	}

	iteration++;

	if (iteration == (numDigits * 2) + 1) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool ScrollRight::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

void ScrollLeft::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
	iteration = 0;
}

bool ScrollLeft::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (iteration >= numDigits + 1) {
		// We have displayed all-spaces, start shifting target in from the 'left'
		current = target << (4 * (numDigits*2-iteration));
		current |= 0xcccccc >> (4 * (iteration-numDigits+(6-numDigits)));
	} else {
		// Shift current digits to the 'left', pad with spaces
		current = current >> 4;
		current = (current & (0xffffff >> (4 * (7-numDigits)))) | (0xc << ((numDigits-1) * 4));
	}

	iteration++;

	if (iteration == (numDigits * 2) + 1) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool ScrollLeft::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

void SlideLeft::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
	i = j = 0;
}

bool SlideLeft::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (i >= numDigits) {

		if (j == 0) {
			i++;
			j = (numDigits*2) + 1 - i;
		}
		j--;
		byte l = (numDigits*2) - i;
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
		uint32_t targetSrcMask = 0xf << (4*(i-(numDigits + 1)));
		uint32_t dstMask = (0xf << (4*(numDigits - 1))) >> (4*m);
		uint32_t oldDstMask = (dstMask << 4) & 0xffffff;
		uint32_t digit = (target & targetSrcMask) << (4*j);
		// Set old position to blank
		if (oldDstMask != 0) {
			current = (current & ~oldDstMask) | (oldDstMask & 0xcccccc);
		}
		// Or in new digit
		current = ((current & ~dstMask) | digit) & 0xffffff;
	} else {
		/*
		 * i increments more slowly. i.e.
		 * for (int i=0; i<numDigits; i++) {
		 *   for (int j=0; j<=i; j++) {
		 *   }
		 * }
		 */
		// Shift current digits to the 'left', pad with spaces
		// Select the correct digit:
		uint32_t srcMask = 0xf << (4*(i - j));
		uint32_t dstMask = srcMask >> 4;
		uint32_t digit = current & srcMask;
		// Set old position to blank
		current = (current & ~srcMask) | (srcMask & 0xcccccc);
		// Or in new digit
		if (dstMask != 0) {
			current = (current & ~dstMask) | (digit >> 4);
		}

		j++;
		if (j > i) {
			i++;
			j = 0;
		}
	}

	if (i == numDigits*2) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool SlideLeft::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

void SlideRight::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
	i = j = 0;
}

bool SlideRight::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (i >= numDigits) {

		if (j == 0) {
			i++;
			j = (numDigits*2) + 1 - i;
		}
		j--;
		byte l = numDigits*2 - i;
		byte m = l-j;

		/*
		 * l decrements more slowly. i.e.
		 * for (int l=numDigits - 1; l>=0; l--) {
		 *   for (int m=0; m<l; m++) {
		 *   }
		 * }
		 */
		// Shift current digits to the 'left', pad with spaces
		// Select the correct digit:
		uint32_t targetSrcMask = (0xf << ((numDigits-1) * 4)) >> (4*(i-(numDigits + 1)));
		uint32_t dstMask = 0xf << (4*m);
		uint32_t oldDstMask = (dstMask >> 4) & 0xffffff;
		uint32_t digit = (target & targetSrcMask) >> (4*j);
		// Set old position to blank
		if (oldDstMask != 0) {
			current = (current & ~oldDstMask) | (oldDstMask & 0xcccccc);
		}
		// Or in new digit
		current = ((current & ~dstMask) | digit) & 0xffffff;
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
		uint32_t srcMask = (0xf << (4*(numDigits - 1))) >> (4*(i - j));
		uint32_t dstMask = (srcMask << 4) & 0xffffff;
		uint32_t digit = current & srcMask;
		// Set old position to blank
		current = (current & ~srcMask) | (srcMask & 0xcccccc);
		// Or in new digit
		if (dstMask != 0) {
			current = (current & ~dstMask) | (digit << 4);
		}

		j++;
		if (j > i) {
			i++;
			j = 0;
		}
	}

	if (i == numDigits * 2) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool SlideRight::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

void Bubble::init(unsigned long nowMs) {
	EffectBase::init(nowMs);
}

bool Bubble::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (numDigits == 4) {
		target = (target & 0xffff) | 0xcccc0000;
	} else if (numDigits == 6) {
		target = (target & 0xffffff) | 0xcc000000;
	}

	// Rotate current digits to nextNixieDigit
	byte digits[8];

	for (int i=numDigits; i < 8; i++) {
		digits[i] = 0xc;
	}

	for (int i=0; i<numDigits; i++) {
		byte cd = current >> (i*4) & 0xf;	// current digit
		byte nd = target >> (i*4) & 0xf;	// next digit
		digits[i] = cd;
		if (cd != nd) {
			if (cd == 0xc) {
				digits[i] = nd;
			} else {
				digits[i] = (cd + 1) % 10;
			}
		}
	}

	current =
			  digits[0]
			+ (digits[1] << 4)
			+ (digits[2] << 8)
			+ (digits[3] << 12)
			+ (digits[4] << 16)
			+ (digits[5] << 20)
			+ (digits[6] << 24)
			+ (digits[7] << 28)
		;

	if (current == target) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return false;
}

bool Bubble::in(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

	if (numDigits == 4) {
		target = (target & 0xffff) | 0xcccc0000;
	} else if (numDigits == 6) {
		target = (target & 0xffffff) | 0xcc000000;
	}

	// Rotate current digits to nextNixieDigit
	byte digits[8];
	for (int i=numDigits; i < 8; i++) {
		digits[i] = 0xc;
	}

	for (int i=0; i<numDigits; i++) {
		byte cd = current >> (i*4) & 0xf;	// current digit
		byte nd = target >> (i*4) & 0xf;	// next digit
		digits[i] = cd;
		if (cd != nd) {
			if (nd == 0xc) {
				digits[i] = nd;
			} else {
				digits[i] = (cd + 9) % 10;
			}
		}
	}

	current =
			  digits[0]
			+ (digits[1] << 4)
			+ (digits[2] << 8)
			+ (digits[3] << 12)
			+ (digits[4] << 16)
			+ (digits[5] << 20)
			+ (digits[6] << 24)
			+ (digits[7] << 28)
		;

	if (current == target) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return false;
}

const uint8_t Divergence::RUN_LENGTH[] = {
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

uint8_t Divergence::runLengths[8];

void Divergence::init(unsigned long nowMs) {
	EffectBase::init(nowMs);

	for (int i=0; i<8; i++) {
		runLengths[i] = RUN_LENGTH[random(64L)];
	}

	adjustRL = true;
	pulse = 25;
	savedBrightness = clock.getBrightness();
}

bool Divergence::out(unsigned long nowMs, uint32_t target) {
	if (!effectTimer.isEnabled()) {
		return true;	// Done
	}

	if (!effectTimer.expired(nowMs)) {
		return false;	// Call again
	}

#ifdef notdef
	if (adjustRL) {
		adjustRL = false;

		// Adjust run lengths by difference between current and target
		for (int i=0; i<6; i++) {
			// nixieDigit is BCD encoded. nextNixieDigit isn't.
			uint8_t cd = current >> (i*4) & 0xf;	// current digit
			uint8_t nd = target >> (i*4) & 0xf;	// next digit

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
	uint8_t digits[8];
	bool allEqual = true;
	for (int i=0; i<8; i++) {
		uint8_t cd = current >> (i*4) & 0xf;	// current digit
		uint8_t nd = target >> (i*4) & 0xf;	// next digit
		digits[i] = cd;

		if (runLengths[i] - 1 != 0) {
			runLengths[i] -= 1;
			allEqual = false;

			if (cd == 0xc) {
				digits[i] = 0;	// If the current digit is a space, seed it with a number
			} else {
				digits[i] = (cd + 1) % 10;
			}
		} else  {
			// To be sure as this could be a space
			digits[i] = nd;
		}
	}

	current =
			  digits[0]
			+ (digits[1] << 4)
			+ (digits[2] << 8)
			+ (digits[3] << 12)
			+ (digits[4] << 16)
			+ (digits[5] << 20)
			+ (digits[6] << 24)
			+ (digits[7] << 28)
		;

	if (allEqual && pulse != 0) {
		pulse -= 1;
		if (pulse == 0) {
			clock.unlockBrightness(savedBrightness);
		} else if (pulse < 20){
			uint8_t pulseBrightness = savedBrightness * 2.5;
			if (pulseBrightness > 100) {
				pulseBrightness = 100;
			}
			clock.lockBrightness(pulseBrightness);
		}
	}

	bool done = allEqual && (pulse == 0);

	if (done) {
		effectTimer.setEnabled(false);
	} else {
		effectTimer.init(nowMs, period);
	}

	return done;
}

bool Divergence::in(unsigned long nowMs, uint32_t target) {
	return out(nowMs, target);
}

} /* namespace ClockEffects */
