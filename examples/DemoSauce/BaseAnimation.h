#ifndef BASE_ANIMATION_H__
#define BASE_ANIMATION_H__

#include <Arduino.h>
#include <GC9A01A_t3n.h>
#include "MathUtil.h"

class BaseAnimation {
public:
	BaseAnimation(){};

	virtual void init( GC9A01A_t3n tft );
	virtual uint_fast16_t bgColor( void );
	virtual void reset( GC9A01A_t3n tft );
	virtual String title();

	virtual boolean willForceTransition( void );
	virtual boolean forceTransitionNow( void );

	virtual void perFrame( GC9A01A_t3n tft, FrameParams frameParams );
};

void BaseAnimation::init( GC9A01A_t3n tft ) {
	// Extend me
}

uint_fast16_t BaseAnimation::bgColor( void ) {
	// Extend me
	return 0xf81f;	// Everyone loves magenta
}

void BaseAnimation::reset( GC9A01A_t3n tft ) {
	// Extend me
}

String BaseAnimation::title() {
	return "BaseAnimation";
}

boolean BaseAnimation::willForceTransition( void ) {
	return false;	// Default: SuperTFT will transition animations automatically
}

boolean BaseAnimation::forceTransitionNow( void ) {
	// Extend me
	return false;	// Default: SuperTFT will transition animations automatically
}

void BaseAnimation::perFrame( GC9A01A_t3n tft, FrameParams frameParams ) {
	// Extend me
}

#endif
