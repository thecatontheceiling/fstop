//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#if !defined( C_BASEHLPLAYER_H )
#define C_BASEHLPLAYER_H
#ifdef _WIN32
#pragma once
#endif
//#define HL2_PLAYERANIMSTATE //uncomment for global hl2_playeranimstate across projects.

#include "c_baseplayer.h"
#include "c_hl2_playerlocaldata.h"
#include "multiplayer/basenetworkedplayer_cl.h"
#ifdef HL2_PLAYERANIMSTATE
#include "hl2_playeranimstate.h"
#endif
class C_BaseHLPlayer : public C_BaseNetworkedPlayer
{
public:
	DECLARE_CLASS( C_BaseHLPlayer, C_BaseNetworkedPlayer);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

						C_BaseHLPlayer();

						
						
						virtual void		OnDataChanged(DataUpdateType_t updateType);
#ifdef HL2_PLAYERANIMSTATE
						//DM -hl2 portal-animstate reimplementation.
						//to use, define HL2_PLAYERANIMSTATE in preprocessor defs.
						~C_BaseHLPlayer(void); //redundant from portal
						void ClientThink(void);
						virtual void		OnPreDataChanged(DataUpdateType_t type);
						virtual void		PostDataUpdate(DataUpdateType_t updateType);

						
						virtual void			PreThink(void);
						virtual void UpdateClientSideAnimation();
						void DoAnimationEvent(PlayerAnimEvent_t event, int nData);
						QAngle GetAnimEyeAngles(void) { return m_angEyeAngles; }
						virtual const QAngle&	EyeAngles();
						virtual const QAngle& GetRenderAngles();
						CInterpolatedVar< QAngle >	m_iv_angEyeAngles;
						virtual void Spawn(void);
						CHL2PlayerAnimState *m_PlayerAnimState;
						QAngle	m_angEyeAngles;
						int	m_headYawPoseParam;
						int	m_headPitchPoseParam;
						float m_headYawMin;
						float m_headYawMax;
						float m_headPitchMin;
						float m_headPitchMax;
						void	UpdateLookAt(void);
						void	Initialize(void);
						virtual CStudioHdr*		OnNewModel(void);
						// Used by prediction, sets the view angles for the player
						virtual void SetLocalViewAngles(const QAngle &viewAngles);
						virtual void SetViewAngles(const QAngle &ang);
						struct PreDataChanged_Backup_t
						{
							//Vector					m_ptPlayerPosition;
							QAngle					m_qEyeAngles;
						} PreDataChanged_Backup;

						float m_flLastBodyYaw;
						float m_flCurrentHeadYaw;
						float m_flCurrentHeadPitch;
						float m_flStartLookTime;
#endif

						

						

	void				Weapon_DropPrimary( void );
		
	float				GetFOV();
	void				Zoom( float FOVOffset, float time );
	float				GetZoom( void );
	bool				IsZoomed( void )	{ return m_HL2Local.m_bZooming; }

	//Tony; minor cosmetic really, fix confusion by simply renaming this one; everything calls IsSprinting(), and this isn't really even used.
	bool				IsSprintActive( void ) { return (m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_SPRINT) != 0; }
	bool				IsFlashlightActive( void ) { return (m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_FLASHLIGHT) != 0; }
	bool				IsBreatherActive( void ) { return (m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_BREATHER) != 0; }

	virtual int			DrawModel( int flags, const RenderableInstance_t &instance );

	LadderMove_t		*GetLadderMove() { return &m_HL2Local.m_LadderMove; }
	virtual void		ExitLadder();
	bool				IsSprinting() const { return m_fIsSprinting; }

	virtual bool ShouldRegenerateOriginFromCellBits() const{return true;}

	// Input handling
	virtual bool	CreateMove( float flInputSampleTime, CUserCmd *pCmd );
	void			PerformClientSideObstacleAvoidance( float flFrameTime, CUserCmd *pCmd );
	void			PerformClientSideNPCSpeedModifiers( float flFrameTime, CUserCmd *pCmd );

	bool				IsWeaponLowered( void ) { return m_HL2Local.m_bWeaponLowered; }

public:

	C_HL2PlayerLocalData		m_HL2Local;
	EHANDLE				m_hClosestNPC;
	float				m_flSpeedModTime;
	bool				m_fIsSprinting;

private:
	C_BaseHLPlayer( const C_BaseHLPlayer & ); // not defined, not accessible
	
	bool				TestMove( const Vector &pos, float fVertDist, float radius, const Vector &objPos, const Vector &objDir );

	float				m_flZoomStart;
	float				m_flZoomEnd;
	float				m_flZoomRate;
	float				m_flZoomStartTime;

	bool				m_bPlayUseDenySound;		// Signaled by PlayerUse, but can be unset by HL2 ladder code...
	float				m_flSpeedMod;
	float				m_flExitSpeedMod;


friend class CHL2GameMovement;
};

#ifdef HL2_PLAYERANIMSTATE
static inline C_BaseHLPlayer* GetLocalPortalPlayer()
{
	return (C_BaseHLPlayer*)C_BasePlayer::GetLocalPlayer();
}

static inline C_BaseHLPlayer* GetLocalPlayer()
{
	return (C_BaseHLPlayer*)C_BasePlayer::GetLocalPlayer();
}
#endif


#endif
