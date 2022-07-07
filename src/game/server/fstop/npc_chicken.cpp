//==== Copyright © 2017-2019, Lever Softworks, All rights reserved. ====//
//
// Purpose: F-STOP C++ File
//
//======================================================================//

#include "cbase.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CHICKEN_MODEL "models/chicken/chicken.mdl"

//=========================================================
//=========================================================
class CNPC_Chicken : public CAI_BaseNPC
{
	DECLARE_CLASS( CNPC_Chicken, CAI_BaseNPC );

public:
	void	Precache( void );
	void	Spawn( void );
	Class_T Classify( void );

	DECLARE_DATADESC();

	DEFINE_CUSTOM_AI;
};

LINK_ENTITY_TO_CLASS( npc_chicken, CNPC_Chicken );


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Chicken )
END_DATADESC()


//---------------------------------------------------------
// Custom AI
//---------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( npc_chicken, CNPC_Chicken )
AI_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_Chicken::Precache( void )
{
	PrecacheModel( STRING( GetModelName() ) );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_Chicken::Spawn( void )
{
	if ( !GetModelName() )
	{
		// if no model then set to default
		SetModelName( MAKE_STRING( CHICKEN_MODEL ) );
	}

	Precache();
	SetModel( STRING( GetModelName() ) );
	SetHullType( HULL_TINY );
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	m_iHealth			= 20;
	m_flFieldOfView		= 0.5;
	m_NPCState			= NPC_STATE_NONE;

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_MOVE_JUMP );

	NPCInit();
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Chicken::Classify( void )
{
	return	CLASS_NONE;
}
