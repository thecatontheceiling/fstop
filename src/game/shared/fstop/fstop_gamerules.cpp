//==== Copyright © 2017-2018, Lever Softworks, All rights reserved. ====//
//
// F-STOP Gamerules
//
//======================================================================//

#include "cbase.h"
#include "fstop_gamerules.h"

#include "vstdlib/ikeyvaluessystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

REGISTER_GAMERULES_CLASS( CFSTOPGameRules );

bool CFSTOPGameRules::Init()
{
	KeyValuesSystem()->SetKeyValuesExpressionSymbol( "FSTOP", true );
	return BaseClass::Init();
}