#ifndef BASE_TRANSITION_H__
#define BASE_TRANSITION_H__

#include <Arduino.h>
#include <GC9A01A_t3n.h>
#include "MathUtil.h"

class BaseTransition {
public:
	BaseTransition(){};

	virtual void init( GC9A01A_t3n tft );
	virtual void restart( GC9A01A_t3n tft, uint_fast16_t color );
	virtual void perFrame( GC9A01A_t3n tft, FrameParams frameParams );
	virtual boolean isComplete();
};

void BaseTransition::init( GC9A01A_t3n tft ) {
	// Extend me
}

void BaseTransition::restart( GC9A01A_t3n tft, uint_fast16_t color ) {
	// Extend me
}

void BaseTransition::perFrame( GC9A01A_t3n tft, FrameParams frameParams ) {
	// Extend me
}

boolean BaseTransition::isComplete() {
	// Extend me
	return false;
}

#endif
