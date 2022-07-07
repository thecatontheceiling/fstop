//==== Copyright © 2017-2019, Lever Softworks, All rights reserved. ====//
//
// Purpose: info_placement_helper
//
//======================================================================//

#ifndef INFO_PLACEMENT_HELPER_H
#define INFO_PLACEMENT_HELPER_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"

class CInfoPlacementHelper : public CPointEntity
{
public:
	DECLARE_CLASS( CInfoPlacementHelper, CPointEntity );
	DECLARE_DATADESC();

	bool UseAngles() { return m_bUseAngles; }	// cognate to ShouldUseHelperAngles

	float GetRadius() { return m_flRadius; }	// cognate to GetTargetRadius

private:
	bool m_bUseAngles;
	float m_flRadius;

};

#endif // INFO_PLACEMENT_HELPER_H
