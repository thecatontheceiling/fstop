//==== Copyright © 2017-2019, Lever Softworks, All rights reserved. ====//
//
// Purpose: F-STOP C++ File
//
//======================================================================//

#include "cbase.h"
#include "npc_android.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BOT_MODEL_MALE "models/bot_male/bot_male.mdl"
#define BOT_MODEL_FEMALE "models/bot_fem/bot_fem.mdl"

// Activities
int ACT_ANDROID_RISE_FROM_GROUND;
int ACT_DROID_SWATLEFTLOW;
int ACT_DROID_SWATLEFTMID;
int ACT_DROID_SWATRIGHTLOW;
int ACT_DROID_SWATRIGHTMID;
int ACT_DROID_JUMPSWIPE;
int ACT_DROID_MELEE_CHOP;

LINK_ENTITY_TO_CLASS( npc_android, CNPC_Android );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Android )

	DEFINE_FIELD( m_iDeleteThisField, FIELD_INTEGER ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Precache
//-----------------------------------------------------------------------------
void CNPC_Android::Precache( void )
{
	PrecacheModel( STRING( GetModelName() ) );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_Android::Spawn( void )
{
	if ( !GetModelName() )
	{
		// if no model then set to default
		SetModelName( MAKE_STRING( BOT_MODEL_MALE ) );
	}

	Precache();
	SetModel( STRING( GetModelName() ) );
	SetHullType( HULL_HUMAN );
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_MECH );
	m_iHealth			= 20;
	m_flFieldOfView		= 0.5;
	m_NPCState			= NPC_STATE_NONE;
	
	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_MOVE_JUMP );

	NPCInit();
}


//-----------------------------------------------------------------------------
// Don't have a class currently
//-----------------------------------------------------------------------------
Class_T	CNPC_Android::Classify( void )
{
	return	CLASS_NONE;
}

void CNPC_Android::GatherConditions( )
{
	BaseClass::GatherConditions();
}


AI_BEGIN_CUSTOM_NPC( npc_android, CNPC_Android )
	
	// Tasks
	DECLARE_TASK( TASK_ANDROID_EXPRESS_ANGER )
	DECLARE_TASK( TASK_ANDROID_ATTACK_DOOR )
	DECLARE_TASK( TASK_ANDROID_DIE )

	// Conditions
	DECLARE_CONDITION( COND_ANDROID_ANGRY )

	// Activities
	DECLARE_ACTIVITY( ACT_ANDROID_RISE_FROM_GROUND )
	DECLARE_ACTIVITY( ACT_DROID_SWATLEFTLOW )
	DECLARE_ACTIVITY( ACT_DROID_SWATLEFTMID )
	DECLARE_ACTIVITY( ACT_DROID_SWATRIGHTLOW )
	DECLARE_ACTIVITY( ACT_DROID_SWATRIGHTMID )
	DECLARE_ACTIVITY( ACT_DROID_JUMPSWIPE )
	DECLARE_ACTIVITY( ACT_DROID_MELEE_CHOP )

	// Anim Events (None)

	// Schedules

AI_END_CUSTOM_NPC()
