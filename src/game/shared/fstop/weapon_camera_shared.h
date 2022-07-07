//==== Copyright � 2017-2018, Linus S. (PistonMiner), All rights reserved. ====///
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_CAMERA_SHARED_H
#define WEAPON_CAMERA_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_portalbase.h"

#ifdef CLIENT_DLL
#define CWeaponCamera C_WeaponCamera
#endif

class CWeaponCamera : public CWeaponPortalBase
{
	DECLARE_DATADESC();

public:
	CWeaponCamera();
	DECLARE_CLASS( CWeaponCamera, CWeaponPortalBase );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual void Spawn() override;
	virtual void Think() override;

	virtual void Equip( CBaseCombatCharacter *pOwner );

	virtual void PrimaryAttack() override;
	virtual void SecondaryAttack() override;

	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo = nullptr ) override;

	// Viewfinder
	inline bool IsShowingViewfinder() { return m_bShowingViewfinder; }
	void SetShowingViewfinder( bool bShow );

	// Screenspace Math
	Vector2D GetScreenspacePosition( const Vector &worldSpacePosition );
	void GetObjectScreenspaceBoundingBox( const CBaseEntity *pEntity, Vector2D &min, Vector2D &max );

	// View Objects
	int GetNumObjectsInView();
	CBaseEntity *GetObjectInView( int index );
	int GetPrimaryObjectIndex();

#ifdef CLIENT_DLL

	// Server to client message received
	virtual void ReceiveMessage( int classID, bf_read &msg ) override;

	virtual void ClientThink() override;

	virtual bool ShouldDrawCrosshair() override;
#endif

	void TakePhoto( int photoNum = -1 );

	DECLARE_ACTTABLE();

private:
	void UpdateObjectsInView();

private:
	CNetworkVar( float, m_flFieldOfView );
	CNetworkVar( bool, m_bShowingViewfinder );
	CNetworkVar( int, m_nNumEntitiesInView );
	CNetworkArray( EHANDLE, m_hEntitiesInView, 16 );
	CNetworkVar( int, m_iPrimaryObject );

	float m_flCameraMinimumRange = 0.1f;
	float m_flCameraMaximumRange = 4096.f;
	float m_flAspectRatio = 16.f / 9.f;

private:
	CWeaponCamera( const CWeaponCamera & );
};

#endif // WEAPON_CAMERA_SHARED_H