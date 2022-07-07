//==== Copyright © 2017-2018, Lever Softworks, All rights reserved. ====//
//
// Purpose: F-STOP Common Code
//
//======================================================================//

#ifndef FSTOP_COMMON_H
#define FSTOP_COMMON_H
#ifdef _WIN32
#pragma once
#endif

#include "logging.h"

#define COLOR_ORANGE		Color(255, 127, 0, 255)

DECLARE_LOGGING_CHANNEL( LOG_FSTOP );

extern ConVar sv_camera_flash;

// (Currently hardcoded) maximum number of photo render targets
#define FSTOP_NUM_PHOTOS 3

#endif // FSTOP_COMMON_H