//==== Copyright © 2018-2019, (Uncanny), All rights reserved. ====//
//
// Purpose: Photo Shadow for Placement
//
//=============================================================================//

#ifndef PHOTOSHADOW_H
#define PHOTOSHADOW_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CPropPhotoShadow C_PropPhotoShadow
#endif


class CPropPhotoShadow : public CBaseAnimating
{
public:
	DECLARE_CLASS(CPropPhotoShadow, CBaseAnimating);
	DECLARE_DATADESC();

	void Spawn(void);
};

#endif // PHOTOSHADOW_H