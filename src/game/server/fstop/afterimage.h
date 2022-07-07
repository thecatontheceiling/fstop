//==== Copyright © 2018-2019, (Uncanny), All rights reserved. ====//
//
// Purpose: After Image Effect
//
//=============================================================================//

#ifndef AFTERIMAGE_H
#define AFTERIMAGE_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CPropAfterImage C_PropAfterImage
#endif

// to allow other scripts to access the convar
class CPropAfterImage : public CBaseAnimating
{
public:
	DECLARE_CLASS(CPropAfterImage, CBaseAnimating);
	DECLARE_DATADESC();

	CBaseEntity *m_hTeleportingEntity; //the entity we're exchanging Fade with

	void Spawn(void);

	void FadeThink(void);

private:
	RenderMode_t InitialMode;
};

#endif // AFTERIMAGE_H