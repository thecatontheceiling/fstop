//==== Copyright © 2018-2019, (Uncanny), All rights reserved. ====//
//
// Purpose: After Image Effect
//
//=============================================================================//

#include "cbase.h"
#include "prop_photo_shadow.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( prop_photo_shadow, CPropPhotoShadow );

BEGIN_DATADESC( CPropPhotoShadow )

END_DATADESC()

void CPropPhotoShadow::Spawn( void )
{
	AddEFlags( EF_NOFLASHLIGHT | EF_NOSHADOWDEPTH | EF_NOSHADOW );
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NOCLIP );
	SetRenderMode( kRenderTransColor );
	SetRenderAlpha( 0 );
}