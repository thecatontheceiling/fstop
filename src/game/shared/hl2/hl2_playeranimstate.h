//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HL2_PLAYERANIMSTATE_H
#define HL2_PLAYERANIMSTATE_H
#ifdef _WIN32
#pragma once
#endif


#include "convar.h"
#include "..\shared\multiplayer\multiplayer_animstate.h"


#if defined( CLIENT_DLL )
class C_BaseHLPlayer;
#define CHL2_Player C_BaseHLPlayer
#else
class CHL2_Player;
#endif

//enum PlayerAnimEvent_t
//{
//	PLAYERANIMEVENT_FIRE_GUN=0,
//	PLAYERANIMEVENT_THROW_GRENADE,
//	PLAYERANIMEVENT_ROLL_GRENADE,
//	PLAYERANIMEVENT_JUMP,
//	PLAYERANIMEVENT_RELOAD,
//	PLAYERANIMEVENT_SECONDARY_ATTACK,
//
//	PLAYERANIMEVENT_HS_NONE,
//	PLAYERANIMEVENT_CANCEL_GESTURES,	// cancel current gesture
//
//	PLAYERANIMEVENT_COUNT
//};

// ------------------------------------------------------------------------------------------------ //
// CPlayerAnimState declaration.
// ------------------------------------------------------------------------------------------------ //
class CHL2PlayerAnimState : public CMultiPlayerAnimState
{
public:
	
	DECLARE_CLASS(CHL2PlayerAnimState, CMultiPlayerAnimState);

	CHL2PlayerAnimState();
	CHL2PlayerAnimState(CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData);
	~CHL2PlayerAnimState();

	void InitHL2(CHL2_Player *pPlayer);
	CHL2_Player *GetHL2Player(void)							{ return m_pHL2Player; }

	virtual void ClearAnimationState();

	virtual Activity TranslateActivity( Activity actDesired );

	void	DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	void    Teleport(const Vector *pNewOrigin, const QAngle *pNewAngles, CHL2_Player* pPlayer);

	bool	HandleMoving( Activity &idealActivity );
	bool	HandleJumping( Activity &idealActivity );
	bool	HandleDucking( Activity &idealActivity );

private:
	
	CHL2_Player   *m_pHL2Player;
	bool		m_bInAirWalk;

	float		m_flHoldDeployedPoseUntilTime;
};


CHL2PlayerAnimState* CreateHL2PlayerAnimState(CHL2_Player *pPlayer);


// If this is set, then the game code needs to make sure to send player animation events
// to the local player if he's the one being watched.
extern ConVar cl_showanimstate;


#endif // HL2_PLAYERANIMSTATE_H
