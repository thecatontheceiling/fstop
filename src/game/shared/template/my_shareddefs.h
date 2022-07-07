//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: Here are tags to enable/disable things throughout the source code.
//
//=============================================================================//

#ifndef MY_SHAREDDEFS_H
#define MY_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

// THIS IS CALLED FROM SHAREDDEFS.H. PLACE YOUR NEW SHAREDDEFS.H HERE!
// This exist so you can easily list and manage additonal shareddef files 
// Without editing base code.

// Our Game-based shareddefs go here.
#ifdef FSTOP_DLL
#include "fstop_shareddefs.h"
#else
#ifdef PORTAL 
#include "portal_shareddefs.h" 
#elif defined(HL2_DLL) || defined(HL2_CLIENT_DLL) 
#include "hl2_shareddefs.h" 
#else 
#include "sdk_shareddefs.h"
#endif 
#endif

#endif // MY_SHAREDDEFS_H