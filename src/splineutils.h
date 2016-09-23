#pragma once

#include <nap/entity.h>
#include <ofVec3f.h>
#include <ofColor.h>

// Default spline color
extern const ofFloatColor gDefaultSplineColor;
extern const ofFloatColor gDefaultBlackColor;
extern const ofFloatColor gSwitchColor;

// Adds a spline at the given world space coordinates
nap::Entity& addSpline(nap::Entity& parent, const ofVec3f& worldPos);
