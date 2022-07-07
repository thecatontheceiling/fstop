#include "cbase.h"

#include "fstop_render_targets.h"

#include "fstop_common.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PHOTO_RT_NAME "_rt_LargePhoto%d"

// create a photo render target
ITexture* CFSTOPRenderTargets::CreateLargePhotoRT( IMaterialSystem* pMaterialSystem, int n )
{
	char name[128];
	V_snprintf( name, 128, PHOTO_RT_NAME, n );
	Log_Msg( LOG_FSTOP, "Created Photo RT: %s\n", name );
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		&name[0],
		512, 512, RT_SIZE_OFFSCREEN,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SEPARATE,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR );
}

void CFSTOPRenderTargets::InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig )
{
	for ( int i = 0; i < FSTOP_NUM_PHOTOS; i++ )
	{
		m_PhotoTextures[i] = CreateLargePhotoRT( pMaterialSystem, i+1 );
		m_PhotoTextures[i]->IncrementReferenceCount();
	}

	BaseClass::InitClientRenderTargets( pMaterialSystem, pHardwareConfig );
}

void CFSTOPRenderTargets::ShutdownClientRenderTargets()
{
	for ( int i = 0; i < FSTOP_NUM_PHOTOS; i++ )
	{
		m_PhotoTextures[i]->Release();
	}

	BaseClass::ShutdownClientRenderTargets();
}

ITexture* CFSTOPRenderTargets::GetPhotoTexture(int n)
{
	if (n < FSTOP_NUM_PHOTOS) {
		return m_PhotoTextures[n];
	} else {
		char name[128];
		V_snprintf( name, 128, PHOTO_RT_NAME, n+1 );
		return g_pMaterialSystem->FindTexture( name, TEXTURE_GROUP_RENDER_TARGET );
	}
	
}

// expose IClientRenderTargets interface
static CFSTOPRenderTargets s_FSTOPRenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CFSTOPRenderTargets, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION, s_FSTOPRenderTargets );

// expose the global pointer
CFSTOPRenderTargets* g_pFSTOPRenderTargets = &s_FSTOPRenderTargets;

