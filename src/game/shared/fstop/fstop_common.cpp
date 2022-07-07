#include "cbase.h"

#include "fstop_common.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DEFINE_LOGGING_CHANNEL_NO_TAGS( LOG_FSTOP, "F-STOP", LCF_CONSOLE_ONLY, LS_MESSAGE, COLOR_ORANGE );

ConVar sv_camera_flash( "sv_camera_flash", "1", FCVAR_REPLICATED, " wash out the players screen,hide the object and stop it from colliding instead of leavng it there" );