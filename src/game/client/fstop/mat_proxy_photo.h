//==== Copyright © 2017-2019, Lever Softworks, All rights reserved. ====//
//
// Purpose: PhotoMaterial Proxy. Used to bind photo rendertargets to
//			the photo viewmodel's materials' $basetexture.
//
//======================================================================//

#ifndef MAT_PROXY_PHOTO_H
#define MAT_PROXY_PHOTO_H
#ifdef _WIN32
#pragma once
#endif

#include "ProxyEntity.h"
#include "materialsystem/imaterialvar.h"

class CPhotoMaterialProxy : public CEntityMaterialProxy
{
public:
	//CPhotoMaterialProxy();
	//virtual	~CPhotoMaterialProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues* pKeyValues );
	virtual void OnBind( C_BaseEntity *pEntity );

	virtual IMaterial *GetMaterial();

private:
	ITexture* m_pDefaultTexture;
	IMaterialVar *m_pBaseTexture;
};



#endif // MAT_PROXY_PHOTO_H
