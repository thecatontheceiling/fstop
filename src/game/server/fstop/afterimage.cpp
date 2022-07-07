//==== Copyright © 2018-2019, (Uncanny), All rights reserved. ====//
//
// Purpose: After Image Effect
//
//=============================================================================//

#include "cbase.h"
#include "afterimage.h"
#include "fstop_common.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_afterimage_fade( "sv_afterimage_fade", "3", FCVAR_REPLICATED, "how much to change the afterimage fade each think", true, 0.1f, false, 10 );
LINK_ENTITY_TO_CLASS( prop_afterimage, CPropAfterImage );

// Start of our data description for the class
BEGIN_DATADESC( CPropAfterImage )

	// Save/restore our active state
	DEFINE_FIELD( m_hTeleportingEntity, FIELD_EHANDLE ),

	// Declare our think function
	DEFINE_THINKFUNC( FadeThink ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CPropAfterImage::Spawn( void )
{
	SetModel( STRING( m_hTeleportingEntity->GetModelName() ) );
	SetAbsOrigin( m_hTeleportingEntity->GetAbsOrigin() );
	SetAbsAngles( m_hTeleportingEntity->GetAbsAngles() );

	CBaseEntity *temp = m_hTeleportingEntity;
	InitialMode = temp->GetRenderMode();
	temp->SetRenderMode( kRenderTransColor );
	temp->SetRenderAlpha( 0 );

	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NOCLIP );
	SetRenderMode( kRenderTransColor );
	if ( sv_camera_flash.GetBool() )
	{
		// don't show the after image if the object is supposed to stay invisible
		// can't simply have the after image not be spawned at all in this case as it controls the fading of the placed object
		SetRenderMode( kRenderNone );
		m_hTeleportingEntity->SetSolid( SOLID_VPHYSICS );
		m_hTeleportingEntity->SetMoveType( MOVETYPE_VPHYSICS );
	}
	else
	{
		SetRenderAlpha( 255 );
	}
	SetThink( &CPropAfterImage::FadeThink );
	SetNextThink( gpGlobals->curtime + 0.01f );
}

//-----------------------------------------------------------------------------
// Purpose: Think function to Fade out the effect and Fade in the actual object
//-----------------------------------------------------------------------------
void CPropAfterImage::FadeThink( void )
{
	int a = GetRenderAlpha() - sv_afterimage_fade.GetInt();
	if ( a > 0 )
	{
		SetRenderAlpha( a );
		m_hTeleportingEntity->SetRenderAlpha( 255 - a );
		SetNextThink( gpGlobals->curtime + 0.001f );
	}
	else
	{
		// completely hide the after image
		SetRenderMode( kRenderNone );
		CBaseEntity *temp = m_hTeleportingEntity;
		if ( sv_camera_flash.GetBool() )
		{
			// make sure the placed object is completely visible after placement, the stored mode is inaccurate due to forcing it to be invisible
			temp->SetRenderMode( kRenderNormal );
		}
		else
		{
			temp->SetRenderMode( InitialMode ); // completely show the entity
		}
		IPhysicsObject *aftertemper = temp->VPhysicsGetObject();
		aftertemper->EnableMotion( true );
		aftertemper->Wake();
		SetThink( NULL ); // get ready to die
		UTIL_Remove( this ); // die
	}
}