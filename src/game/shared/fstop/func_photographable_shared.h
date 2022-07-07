//==== Copyright © 2017-2018, Linus S. (PistonMiner), All rights reserved. ====//
//
// Purpose: 
//
//=============================================================================//

#ifndef FUNC_PHOTOGRAPHABLE_SHARED_H
#define FUNC_PHOTOGRAPHABLE_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CFuncPhotographable C_FuncPhotographable
#endif

class CFuncPhotographable : public CBaseEntity
{
	DECLARE_DATADESC();

public:
	virtual void Spawn() override;

	DECLARE_CLASS(CFuncPhotographable, CBaseEntity);
	DECLARE_NETWORKCLASS();
};

#endif // FUNC_PHOTOGRAPHABLE_SHARED_H