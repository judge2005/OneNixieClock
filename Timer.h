/*
 * Timer.h
 *
 *  Created on: Sep 26, 2020
 *      Author: mpand
 */

#ifndef LIBRARIES_ONENIXIECLOCK_TIMER_H_
#define LIBRARIES_ONENIXIECLOCK_TIMER_H_

namespace ClockTimer {

class Timer {
public:
	Timer(unsigned long duration) : enabled(false), lastTick(0), duration(duration) {}
	Timer() : enabled(false), lastTick(0), duration(0) {}

	bool expired(unsigned long now) const {
		return now - lastTick >= duration;
	}
	void setDuration(unsigned long duration) {
		this->duration = duration;
	}
	void init(unsigned long now, unsigned long duration) {
		lastTick = now;
		this->duration = duration;
	}
	void reset(unsigned long duration) {
		lastTick += this->duration;
		this->duration = duration;
	}
	unsigned long getLastTick() const {
		return lastTick;
	}
	unsigned long getDuration() const {
		return duration;
	}

	bool isEnabled() const {
		return enabled;
	}

	void setEnabled(bool enabled) {
		this->enabled = enabled;
	}

private:
	bool enabled;
	unsigned long lastTick;
	unsigned long duration;
};

} /* namespace ClockTimer */

#endif /* LIBRARIES_ONENIXIECLOCK_TIMER_H_ */
