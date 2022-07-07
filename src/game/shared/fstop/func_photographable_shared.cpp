//==== Copyright © 2017-2018, Linus S. (PistonMiner), All rights reserved. ====//
//
// Purpose: Generic photographable object
//
//=============================================================================//

#include "cbase.h"
#include "func_photographable_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void CFuncPhotographable::Spawn()
{
	SetModel(STRING(GetModelName()));
	SetRenderMode(kRenderNone);
	SetSolid(SOLID_VPHYSICS);
	AddSolidFlags(FSOLID_NOT_SOLID);
}

IMPLEMENT_NETWORKCLASS_ALIASED(FuncPhotographable, DT_FuncPhotographable)

BEGIN_NETWORK_TABLE(CFuncPhotographable, DT_FuncPhotographable)
END_NETWORK_TABLE()

BEGIN_DATADESC(CFuncPhotographable)
END_DATADESC()

LINK_ENTITY_TO_CLASS(func_photographable, CFuncPhotographable);