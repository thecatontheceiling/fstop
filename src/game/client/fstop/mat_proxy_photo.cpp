//==== Copyright © 2017-2019, Lever Softworks, All rights reserved. ====//
//
// Purpose: PhotoMaterial Proxy. Used to bind photo rendertargets to
//			the photo viewmodel's materials' $basetexture.
//
//======================================================================//

#include "cbase.h"
#include "imaterialproxydict.h"
#include "mat_proxy_photo.h"
#include "weapon_placement_shared.h"
#include "fstop_render_targets.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

EXPOSE_MATERIAL_PROXY( CPhotoMaterialProxy, PhotoMaterial );

bool CPhotoMaterialProxy::Init( IMaterial *pMaterial, KeyValues* pKeyValues )
{
	bool found = false;
	m_pBaseTexture = pMaterial->FindVar( "$basetexture", &found, false );
	if ( found ) m_pDefaultTexture = m_pBaseTexture->GetTextureValue();
	return found;
}

void CPhotoMaterialProxy::OnBind( C_BaseEntity *pEntity )
{
	if ( !pEntity ) return;

	C_BaseViewModel *pViewmodel = dynamic_cast<C_BaseViewModel *>( pEntity );
	
	if ( !pViewmodel ) return;

	C_WeaponPlacement *pPhoto = dynamic_cast<C_WeaponPlacement *>( pViewmodel->GetOwningWeapon() );

	if ( !pPhoto ) return;
	
	if ( !pPhoto->HasPhotos() ) return;

	ITexture *pTexture = g_pFSTOPRenderTargets->GetPhotoTexture( pPhoto->GetSelectedPhoto() );
	if ( !pTexture || pTexture->IsError() )
		m_pBaseTexture->SetTextureValue( m_pDefaultTexture );
	else
		m_pBaseTexture->SetTextureValue( pTexture );
}

IMaterial* CPhotoMaterialProxy::GetMaterial()
{
	return m_pBaseTexture->GetOwningMaterial();
}