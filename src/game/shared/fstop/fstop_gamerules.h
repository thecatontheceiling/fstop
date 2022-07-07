//==== Copyright © 2017-2018, Lever Softworks, All rights reserved. ====//
//
// F-STOP Gamerules Header
//
//======================================================================//

#ifndef FSTOP_GAMERULES_H
#define FSTOP_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "portal_gamerules.h"

#ifdef CLIENT_DLL
	#define CFSTOPGameRules C_FSTOPGameRules
#endif

class CFSTOPGameRules : public CPortalGameRules
{
	DECLARE_CLASS( CFSTOPGameRules, CPortalGameRules );

	virtual bool	Init();
	

private:

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.

#endif
};

#endif