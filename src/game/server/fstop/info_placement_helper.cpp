//==== Copyright © 2017-2019, Lever Softworks, All rights reserved. ====//
//
// Purpose: info_placement_helper
//
//======================================================================//

#include "cbase.h"
#include "info_placement_helper.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( info_placement_helper, CInfoPlacementHelper );

BEGIN_DATADESC( CInfoPlacementHelper )

	DEFINE_KEYFIELD( m_bUseAngles, FIELD_BOOLEAN, "snap_to_helper_angles" ),
	DEFINE_KEYFIELD( m_flRadius, FIELD_FLOAT, "radius" ),

END_DATADESC()
