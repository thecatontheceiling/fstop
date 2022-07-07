//==== Copyright © 2017-2018, Lever Softworks, All rights reserved. ====//
//
// Purpose: F-STOP Render Targets
//
//======================================================================//

#ifndef FSTOP_RENDER_TARGETS_H
#define FSTOP_RENDER_TARGETS_H
#ifdef _WIN32
#pragma once
#endif

#include "portal_render_targets.h"
#include "fstop_common.h"

class CFSTOPRenderTargets : public CPortalRenderTargets
{
	// no networked vars
	DECLARE_CLASS_GAMEROOT( CFSTOPRenderTargets, CPortalRenderTargets );
public:
	virtual void InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig );
	virtual void ShutdownClientRenderTargets();

	ITexture* GetPhotoTexture( int n );

private:
	ITexture* CreateLargePhotoRT( IMaterialSystem* pMaterialSystem, int n );

	ITexture* m_PhotoTextures[FSTOP_NUM_PHOTOS];
};

extern CFSTOPRenderTargets* g_pFSTOPRenderTargets;

#endif // FSTOP_RENDER_TARGETS_H