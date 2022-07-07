//==== Copyright © 2017-2018, Linus S. (PistonMiner), All rights reserved. ====//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_PLACEMENT_SHARED_H
#define WEAPON_PLACEMENT_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_portalbase.h"

#ifdef CLIENT_DLL
#define CWeaponPlacement C_WeaponPlacement
#endif

class CWeaponPlacement : public CWeaponPortalBase
{
	DECLARE_DATADESC();

public:
	CWeaponPlacement();
	DECLARE_CLASS( CWeaponPlacement, CWeaponPortalBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual void PrimaryAttack() override;
	virtual void SecondaryAttack() override;

	virtual void ItemPostFrame() override;
	virtual bool Deploy( void );
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL ) override; // to clean up the placement shadow
	virtual bool HasAmmo() override;
	virtual bool HasAnyAmmo() override;

	bool HasPhotographedObject(CBaseEntity *entity);
	bool RegisterPhoto(CBaseEntity *entity, const Vector &eyePosition, const QAngle &eyeAngle);
	bool HasPhotos() const;
	void ResetPhotos();
	void RemovePhoto(int index);

	int GetPhotoCount() const { return m_nPhotoCount; }
	// -1 = no photo (default). first photo is 0
	int GetSelectedPhoto() const { return m_iSelectedPhoto; }

	void SelectPhoto( int n );
	void SelectPreviousPhoto();
	void SelectNextPhoto();

	void SelectScaleLevel( int n );
	void SelectPreviousScaleLevel();
	void SelectNextScaleLevel();

#ifdef CLIENT_DLL
	virtual void ReceiveMessage( int classID, bf_read &msg ) override;
#endif

	DECLARE_ACTTABLE();

private:
	Vector CalculateWorldspacePosition();
	QAngle CalculateWorldspaceAngles();

private:
	CNetworkVar(int, m_nMaxPhotos);
	// -1 = no photo (default). first photo is 0
	CNetworkVar(int, m_iSelectedPhoto);
	CNetworkVar(int, m_nPhotoCount);
	//CNetworkHandle(CBaseAnimating, m_hPhotoShadow);
	CNetworkArray(EHANDLE, m_PhotoEntities, 16);
	//CNetworkArray(Vector, m_PhotoEyePositions, 16);
	CNetworkArray(QAngle, m_PhotoWorldAngles, 16);
	CNetworkVar(bool, m_bPlacing);
	CNetworkVar(int, m_iMinScaleLevel);
	CNetworkVar(int, m_iMaxScaleLevel);
	CNetworkVar(int, m_iSelectedScaleLevel);

private:
	CWeaponPlacement(const CWeaponPlacement &);
};

#endif // WEAPON_PLACEMENT_SHARED_H