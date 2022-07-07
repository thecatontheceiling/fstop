//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basehlplayer.h"
#include "playerandobjectenumerator.h"
#include "engine/ivdebugoverlay.h"
#include "c_ai_basenpc.h"
#include "in_buttons.h"
#include "collisionutils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// How fast to avoid collisions with center of other object, in units per second
#define AVOID_SPEED 2000.0f
extern ConVar cl_forwardspeed;
extern ConVar cl_backspeed;
extern ConVar cl_sidespeed;

extern ConVar zoom_sensitivity_ratio;
extern ConVar default_fov;
extern ConVar sensitivity;

ConVar cl_npc_speedmod_intime( "cl_npc_speedmod_intime", "0.25", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
ConVar cl_npc_speedmod_outtime( "cl_npc_speedmod_outtime", "1.5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );

#ifdef HL2_PLAYERANIMSTATE
// ANIMSTATE: Don't alias here
#if defined( CHL2_Player )
#undef CHL2_Player	
#endif
#endif

IMPLEMENT_CLIENTCLASS_DT(C_BaseHLPlayer, DT_HL2_Player, CHL2_Player)
	RecvPropDataTable( RECVINFO_DT(m_HL2Local),0, &REFERENCE_RECV_TABLE(DT_HL2Local) ),
	RecvPropBool( RECVINFO( m_fIsSprinting ) ),
#ifdef HL2_PLAYERANIMSTATE
	RecvPropFloat(RECVINFO(m_angEyeAngles[0])),
	RecvPropFloat(RECVINFO(m_angEyeAngles[1])),
#endif
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_BaseHLPlayer )
	DEFINE_PRED_TYPEDESCRIPTION( m_HL2Local, C_HL2PlayerLocalData ),
	DEFINE_PRED_FIELD( m_fIsSprinting, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

//-----------------------------------------------------------------------------
// Purpose: Drops player's primary weapon
//-----------------------------------------------------------------------------
void CC_DropPrimary( void )
{
	C_BasePlayer *pPlayer = (C_BasePlayer *) C_BasePlayer::GetLocalPlayer();
	
	if ( pPlayer == NULL )
		return;

	pPlayer->Weapon_DropPrimary();
}

static ConCommand dropprimary("dropprimary", CC_DropPrimary, "dropprimary: Drops the primary weapon of the player.");

// link to the correct class.
#if !defined ( HL2MP ) && !defined ( PORTAL )
LINK_ENTITY_TO_CLASS( player, C_BaseHLPlayer );
#endif

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_BaseHLPlayer::C_BaseHLPlayer()
#ifdef HL2_PLAYERANIMSTATE
: m_iv_angEyeAngles("C_BaseHLPlayer::m_iv_angEyeAngles") //2051 animstate implementation
#endif
{
	AddVar( &m_Local.m_vecPunchAngle, &m_Local.m_iv_vecPunchAngle, LATCH_SIMULATION_VAR );
	AddVar( &m_Local.m_vecPunchAngleVel, &m_Local.m_iv_vecPunchAngleVel, LATCH_SIMULATION_VAR );
#ifdef HL2_PLAYERANIMSTATE
	m_PlayerAnimState = CreateHL2PlayerAnimState(this);
	m_hRagdoll.Set(NULL);
	m_angEyeAngles.Init();
	AddVar(&m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR);
	m_EntClientFlags |= ENTCLIENTFLAG_DONTUSEIK;
#endif
	m_flZoomStart		= 0.0f;
	m_flZoomEnd			= 0.0f;
	m_flZoomRate		= 0.0f;
	m_flZoomStartTime	= 0.0f;
	m_flSpeedMod		= cl_forwardspeed.GetFloat();
}

#ifdef HL2_PLAYERANIMSTATE
//-----------------------------------------------------------------------------
// DEstructor
//-----------------------------------------------------------------------------
C_BaseHLPlayer::~C_BaseHLPlayer(void)
{
	if (m_PlayerAnimState)
	{
		m_PlayerAnimState->Release();
	}
}

//animstate stuff.
void C_BaseHLPlayer::Initialize(void)
{
	m_headYawPoseParam = LookupPoseParameter("head_yaw");
	GetPoseParameterRange(m_headYawPoseParam, m_headYawMin, m_headYawMax);

	m_headPitchPoseParam = LookupPoseParameter("head_pitch");
	GetPoseParameterRange(m_headPitchPoseParam, m_headPitchMin, m_headPitchMax);

	CStudioHdr *hdr = GetModelPtr();
	for (int i = 0; i < hdr->GetNumPoseParameters(); i++)
	{
		SetPoseParameter(hdr, i, 0.0);
	}
}



CStudioHdr *C_BaseHLPlayer::OnNewModel(void)
{
	CStudioHdr *hdr = BaseClass::OnNewModel();

	Initialize();

	return hdr;
}

const QAngle& C_BaseHLPlayer::GetRenderAngles()
{
	if (IsRagdoll())
	{
		return vec3_angle;
	}
	else
	{
		return m_PlayerAnimState->GetRenderAngles();
	}
}

const QAngle& C_BaseHLPlayer::EyeAngles()
{
	if (IsLocalPlayer() && g_nKillCamMode == OBS_MODE_NONE)
	{
		return BaseClass::EyeAngles();
	}
	else
	{
		//C_BaseEntity *pEntity1 = g_eKillTarget1.Get();
		//C_BaseEntity *pEntity2 = g_eKillTarget2.Get();

		//Vector vLook = Vector( 0.0f, 0.0f, 0.0f );

		//if ( pEntity2 )
		//{
		//	vLook = pEntity1->GetAbsOrigin() - pEntity2->GetAbsOrigin();
		//	VectorNormalize( vLook );
		//}
		//else if ( pEntity1 )
		//{
		//	return BaseClass::EyeAngles();
		//	//vLook =  - pEntity1->GetAbsOrigin();
		//}

		//if ( vLook != Vector( 0.0f, 0.0f, 0.0f ) )
		//{
		//	VectorAngles( vLook, m_angEyeAngles );
		//}

		return m_angEyeAngles;
	}
}

void C_BaseHLPlayer::PreThink(void)
{
	QAngle vTempAngles = GetLocalAngles();

	if (IsLocalPlayer())
	{
		vTempAngles[PITCH] = EyeAngles()[PITCH];
	}
	else
	{
		vTempAngles[PITCH] = m_angEyeAngles[PITCH];
	}

	if (vTempAngles[YAW] < 0.0f)
	{
		vTempAngles[YAW] += 360.0f;
	}

	SetLocalAngles(vTempAngles);

	BaseClass::PreThink();
}

void C_BaseHLPlayer::OnPreDataChanged(DataUpdateType_t type)
{
	PreDataChanged_Backup.m_qEyeAngles = m_iv_angEyeAngles.GetCurrent();

	BaseClass::OnPreDataChanged(type);
}
// TODO: leftover portal animstate code. remove???
void C_BaseHLPlayer::ClientThink(void)
{
	//PortalEyeInterpolation.m_bNeedToUpdateEyePosition = true;
	Vector vForward;
	AngleVectors(GetLocalAngles(), &vForward);
}
void C_BaseHLPlayer::PostDataUpdate(DataUpdateType_t updateType)
{
	// C_BaseEntity assumes we're networking the entity's angles, so pretend that it
	// networked the same value we already have.
	SetNetworkAngles(GetLocalAngles());

	BaseClass::PostDataUpdate(updateType);
}

void C_BaseHLPlayer::UpdateClientSideAnimation(void)
{
	UpdateLookAt();

	// Update the animation data. It does the local check here so this works when using
	// a third-person camera (and we don't have valid player angles).
	if (this == C_BaseHLPlayer::GetLocalPlayer())
		m_PlayerAnimState->Update(EyeAngles()[YAW], m_angEyeAngles[PITCH]);
	else
		m_PlayerAnimState->Update(m_angEyeAngles[YAW], m_angEyeAngles[PITCH]);

	BaseClass::UpdateClientSideAnimation();
}

void C_BaseHLPlayer::DoAnimationEvent(PlayerAnimEvent_t event, int nData)
{
	m_PlayerAnimState->DoAnimationEvent(event, nData);
}

void C_BaseHLPlayer::SetLocalViewAngles(const QAngle &viewAngles)
{
	// Nothing
	if (engine->IsPlayingDemo())
		return;
	BaseClass::SetLocalViewAngles(viewAngles);
}

void C_BaseHLPlayer::SetViewAngles(const QAngle& ang)
{
	BaseClass::SetViewAngles(ang);

	if (engine->IsPlayingDemo())
	{
		pl.v_angle = ang;
	}
}

void C_BaseHLPlayer::Spawn(void)
{
	BaseClass::Spawn();

	QAngle vTempAngles = GetLocalAngles();
	vTempAngles[PITCH] = m_angEyeAngles[PITCH];

	SetLocalAngles(vTempAngles);

	// Zero out model pitch, blending takes care of all of it.
	SetLocalAnglesDim(X_INDEX, 0);

	if (this != GetLocalPlayer())
	{
		if (IsEffectActive(EF_DIMLIGHT))
		{
			int iAttachment = LookupAttachment("anim_attachment_RH");

			if (iAttachment < 0)
				return;

			Vector vecOrigin;
			QAngle eyeAngles = m_angEyeAngles;

			GetAttachment(iAttachment, vecOrigin, eyeAngles);

			Vector vForward;
			AngleVectors(eyeAngles, &vForward);

			trace_t tr;
			UTIL_TraceLine(vecOrigin, vecOrigin + (vForward * 200), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		}
	}
}

//-----------------------------------------------------------------------------
/**
* Orient head and eyes towards m_lookAt.
*/
void C_BaseHLPlayer::UpdateLookAt(void)
{
	// head yaw
	if (m_headYawPoseParam < 0 || m_headPitchPoseParam < 0)
		return;

	// This is buggy with dt 0, just skip since there is no work to do.
	if (gpGlobals->frametime <= 0.0f)
		return;

	// defaults if no portals are around
	Vector vPlayerForward;
	GetVectors(&vPlayerForward, NULL, NULL);
	Vector vCurLookTarget = EyePosition();

	if (!IsAlive())
	{
		m_viewtarget = EyePosition() + vPlayerForward*10.0f;
		return;
	}

	bool bNewTarget = false;


	// No other look targets, look straight ahead
	vCurLookTarget += vPlayerForward*10.0f;


	// Figure out where we want to look in world space.
	QAngle desiredAngles;
	Vector to = vCurLookTarget - EyePosition();
	VectorAngles(to, desiredAngles);
	QAngle aheadAngles;
	VectorAngles(vCurLookTarget, aheadAngles);

	// Figure out where our body is facing in world space.
	QAngle bodyAngles(0, 0, 0);
	bodyAngles[YAW] = GetLocalAngles()[YAW];

	m_flLastBodyYaw = bodyAngles[YAW];

	// Set the head's yaw.
	float desiredYaw = AngleNormalize(desiredAngles[YAW] - bodyAngles[YAW]);
	desiredYaw = clamp(desiredYaw, m_headYawMin, m_headYawMax);

	float desiredPitch = AngleNormalize(desiredAngles[PITCH]);
	desiredPitch = clamp(desiredPitch, m_headPitchMin, m_headPitchMax);

	if (bNewTarget)
	{
		m_flStartLookTime = gpGlobals->curtime;
	}

	float dt = (gpGlobals->frametime);
	float flSpeed = 1.0f - ExponentialDecay(0.7f, 0.033f, dt);

	m_flCurrentHeadYaw = m_flCurrentHeadYaw + flSpeed * (desiredYaw - m_flCurrentHeadYaw);
	m_flCurrentHeadYaw = AngleNormalize(m_flCurrentHeadYaw);
	SetPoseParameter(m_headYawPoseParam, m_flCurrentHeadYaw);

	m_flCurrentHeadPitch = m_flCurrentHeadPitch + flSpeed * (desiredPitch - m_flCurrentHeadPitch);
	m_flCurrentHeadPitch = AngleNormalize(m_flCurrentHeadPitch);
	SetPoseParameter(m_headPitchPoseParam, m_flCurrentHeadPitch);

	// This orients the eyes
	m_viewtarget = vCurLookTarget;
}


#endif

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::OnDataChanged( DataUpdateType_t updateType )
{
	// Make sure we're thinking
	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	BaseClass::OnDataChanged( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::Weapon_DropPrimary( void )
{
	engine->ServerCmd( "DropPrimary" );
}

float C_BaseHLPlayer::GetFOV()
{
	//Find our FOV with offset zoom value
	float flFOVOffset = BaseClass::GetFOV() + GetZoom();

	// Clamp FOV in MP
	int min_fov = ( gpGlobals->maxClients == 1 ) ? 5 : default_fov.GetInt();
	
	// Don't let it go too low
	flFOVOffset = MAX( min_fov, flFOVOffset );

	return flFOVOffset;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float C_BaseHLPlayer::GetZoom( void )
{
	float fFOV = m_flZoomEnd;

	//See if we need to lerp the values
	if ( ( m_flZoomStart != m_flZoomEnd ) && ( m_flZoomRate > 0.0f ) )
	{
		float deltaTime = (float)( gpGlobals->curtime - m_flZoomStartTime ) / m_flZoomRate;

		if ( deltaTime >= 1.0f )
		{
			//If we're past the zoom time, just take the new value and stop lerping
			fFOV = m_flZoomStart = m_flZoomEnd;
		}
		else
		{
			fFOV = SimpleSplineRemapVal( deltaTime, 0.0f, 1.0f, m_flZoomStart, m_flZoomEnd );
		}
	}

	return fFOV;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : FOVOffset - 
//			time - 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::Zoom( float FOVOffset, float time )
{
	m_flZoomStart		= GetZoom();
	m_flZoomEnd			= FOVOffset;
	m_flZoomRate		= time;
	m_flZoomStartTime	= gpGlobals->curtime;
}


//-----------------------------------------------------------------------------
// Purpose: Hack to zero out player's pitch, use value from poseparameter instead
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_BaseHLPlayer::DrawModel( int flags, const RenderableInstance_t &instance )
{
	// Not pitch for player
	QAngle saveAngles = GetLocalAngles();

	QAngle useAngles = saveAngles;
	useAngles[ PITCH ] = 0.0f;

	SetLocalAngles( useAngles );

	int iret = BaseClass::DrawModel( flags, instance );

	SetLocalAngles( saveAngles );

	return iret;
}

//-----------------------------------------------------------------------------
// Purpose: Helper to remove from ladder
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::ExitLadder()
{
	if ( MOVETYPE_LADDER != GetMoveType() )
		return;
	
	SetMoveType( MOVETYPE_WALK );
	SetMoveCollide( MOVECOLLIDE_DEFAULT );
	// Remove from ladder
	m_HL2Local.m_hLadder = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Determines if a player can be safely moved towards a point
// Input:   pos - position to test move to, fVertDist - how far to trace downwards to see if the player would fall,
//			radius - how close the player can be to the object, objPos - position of the object to avoid,
//			objDir - direction the object is travelling
//-----------------------------------------------------------------------------
bool C_BaseHLPlayer::TestMove( const Vector &pos, float fVertDist, float radius, const Vector &objPos, const Vector &objDir )
{
	trace_t trUp;
	trace_t trOver;
	trace_t trDown;
	float flHit1, flHit2;
	
	UTIL_TraceHull( GetAbsOrigin(), pos, GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trOver );
	if ( trOver.fraction < 1.0f )
	{
		// check if the endpos intersects with the direction the object is travelling.  if it doesn't, this is a good direction to move.
		if ( objDir.IsZero() ||
			( IntersectInfiniteRayWithSphere( objPos, objDir, trOver.endpos, radius, &flHit1, &flHit2 ) && 
			( ( flHit1 >= 0.0f ) || ( flHit2 >= 0.0f ) ) )
			)
		{
			// our first trace failed, so see if we can go farther if we step up.

			// trace up to see if we have enough room.
			UTIL_TraceHull( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, m_Local.m_flStepSize ), 
				GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trUp );

			// do a trace from the stepped up height
			UTIL_TraceHull( trUp.endpos, pos + Vector( 0, 0, trUp.endpos.z - trUp.startpos.z ), 
				GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trOver );

			if ( trOver.fraction < 1.0f )
			{
				// check if the endpos intersects with the direction the object is travelling.  if it doesn't, this is a good direction to move.
				if ( objDir.IsZero() ||
					IntersectInfiniteRayWithSphere( objPos, objDir, trOver.endpos, radius, &flHit1, &flHit2 ) && ( ( flHit1 >= 0.0f ) || ( flHit2 >= 0.0f ) ) )
				{
					return false;
				}
			}
		}
	}

	// trace down to see if this position is on the ground
	UTIL_TraceLine( trOver.endpos, trOver.endpos - Vector( 0, 0, fVertDist ), 
		MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &trDown );

	if ( trDown.fraction == 1.0f ) 
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Client-side obstacle avoidance
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::PerformClientSideObstacleAvoidance( float flFrameTime, CUserCmd *pCmd )
{
	// Don't avoid if noclipping or in movetype none
	switch ( GetMoveType() )
	{
	case MOVETYPE_NOCLIP:
	case MOVETYPE_NONE:
	case MOVETYPE_OBSERVER:
		return;
	default:
		break;
	}

	// Try to steer away from any objects/players we might interpenetrate
	Vector size = WorldAlignSize();

	float radius = 0.7f * sqrt( size.x * size.x + size.y * size.y );
	float curspeed = GetLocalVelocity().Length2D();

	//int slot = 1;
	//engine->Con_NPrintf( slot++, "speed %f\n", curspeed );
	//engine->Con_NPrintf( slot++, "radius %f\n", radius );

	// If running, use a larger radius
	float factor = 1.0f;

	if ( curspeed > 150.0f )
	{
		curspeed = MIN( 2048.0f, curspeed );
		factor = ( 1.0f + ( curspeed - 150.0f ) / 150.0f );

		//engine->Con_NPrintf( slot++, "scaleup (%f) to radius %f\n", factor, radius * factor );

		radius = radius * factor;
	}

	Vector currentdir;
	Vector rightdir;

	QAngle vAngles = pCmd->viewangles;
	vAngles.x = 0;

	AngleVectors( vAngles, &currentdir, &rightdir, NULL );
		
	bool istryingtomove = false;
	bool ismovingforward = false;
	if ( fabs( pCmd->forwardmove ) > 0.0f || 
		fabs( pCmd->sidemove ) > 0.0f )
	{
		istryingtomove = true;
		if ( pCmd->forwardmove > 1.0f )
		{
			ismovingforward = true;
		}
	}

	if ( istryingtomove == true )
		 radius *= 1.3f;

	CPlayerAndObjectEnumerator avoid( radius );
	partition->EnumerateElementsInSphere( PARTITION_CLIENT_SOLID_EDICTS, GetAbsOrigin(), radius, false, &avoid );

	// Okay, decide how to avoid if there's anything close by
	int c = avoid.GetObjectCount();
	if ( c <= 0 )
		return;

	//engine->Con_NPrintf( slot++, "moving %s forward %s\n", istryingtomove ? "true" : "false", ismovingforward ? "true" : "false"  );

	float adjustforwardmove = 0.0f;
	float adjustsidemove	= 0.0f;

	for ( int i = 0; i < c; i++ )
	{
		C_AI_BaseNPC *obj = dynamic_cast< C_AI_BaseNPC *>(avoid.GetObject( i ));

		if( !obj )
			continue;

		Vector vecToObject = obj->GetAbsOrigin() - GetAbsOrigin();

		float flDist = vecToObject.Length2D();
		
		// Figure out a 2D radius for the object
		Vector vecWorldMins, vecWorldMaxs;
		obj->CollisionProp()->WorldSpaceAABB( &vecWorldMins, &vecWorldMaxs );
		Vector objSize = vecWorldMaxs - vecWorldMins;

		float objectradius = 0.5f * sqrt( objSize.x * objSize.x + objSize.y * objSize.y );

		//Don't run this code if the NPC is not moving UNLESS we are in stuck inside of them.
		if ( !obj->IsMoving() && flDist > objectradius )
			  continue;

		if ( flDist > objectradius && obj->IsEffectActive( EF_NODRAW ) )
		{
			obj->RemoveEffects( EF_NODRAW );
		}

		Vector vecNPCVelocity;
		obj->EstimateAbsVelocity( vecNPCVelocity );
		float flNPCSpeed = VectorNormalize( vecNPCVelocity );

		Vector vPlayerVel = GetAbsVelocity();
		VectorNormalize( vPlayerVel );

		float flHit1, flHit2;
		Vector vRayDir = vecToObject;
		VectorNormalize( vRayDir );

		float flVelProduct = DotProduct( vecNPCVelocity, vPlayerVel );
		float flDirProduct = DotProduct( vRayDir, vPlayerVel );

		if ( !IntersectInfiniteRayWithSphere(
				GetAbsOrigin(),
				vRayDir,
				obj->GetAbsOrigin(),
				radius,
				&flHit1,
				&flHit2 ) )
			continue;

        Vector dirToObject = -vecToObject;
		VectorNormalize( dirToObject );

		float fwd = 0;
		float rt = 0;

		float sidescale = 2.0f;
		float forwardscale = 1.0f;
		bool foundResult = false;

		Vector vMoveDir = vecNPCVelocity;
		if ( flNPCSpeed > 0.001f )
		{
			// This NPC is moving. First try deflecting the player left or right relative to the NPC's velocity.
			// Start with whatever side they're on relative to the NPC's velocity.
			Vector vecNPCTrajectoryRight = CrossProduct( vecNPCVelocity, Vector( 0, 0, 1) );
			int iDirection = ( vecNPCTrajectoryRight.Dot( dirToObject ) > 0 ) ? 1 : -1;
			for ( int nTries = 0; nTries < 2; nTries++ )
			{
				Vector vecTryMove = vecNPCTrajectoryRight * iDirection;
				VectorNormalize( vecTryMove );
				
				Vector vTestPosition = GetAbsOrigin() + vecTryMove * radius * 2;

				if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
				{
					fwd = currentdir.Dot( vecTryMove );
					rt = rightdir.Dot( vecTryMove );
					
					//Msg( "PUSH DEFLECT fwd=%f, rt=%f\n", fwd, rt );
					
					foundResult = true;
					break;
				}
				else
				{
					// Try the other direction.
					iDirection *= -1;
				}
			}
		}
		else
		{
			// the object isn't moving, so try moving opposite the way it's facing
			Vector vecNPCForward;
			obj->GetVectors( &vecNPCForward, NULL, NULL );
			
			Vector vTestPosition = GetAbsOrigin() - vecNPCForward * radius * 2;
			if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
			{
				fwd = currentdir.Dot( -vecNPCForward );
				rt = rightdir.Dot( -vecNPCForward );

				if ( flDist < objectradius )
				{
					obj->AddEffects( EF_NODRAW );
				}

				//Msg( "PUSH AWAY FACE fwd=%f, rt=%f\n", fwd, rt );

				foundResult = true;
			}
		}

		if ( !foundResult )
		{
			// test if we can move in the direction the object is moving
			Vector vTestPosition = GetAbsOrigin() + vMoveDir * radius * 2;
			if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
			{
				fwd = currentdir.Dot( vMoveDir );
				rt = rightdir.Dot( vMoveDir );

				if ( flDist < objectradius )
				{
					obj->AddEffects( EF_NODRAW );
				}

				//Msg( "PUSH ALONG fwd=%f, rt=%f\n", fwd, rt );

				foundResult = true;
			}
			else
			{
				// try moving directly away from the object
				Vector vTestPosition = GetAbsOrigin() - dirToObject * radius * 2;
				if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
				{
					fwd = currentdir.Dot( -dirToObject );
					rt = rightdir.Dot( -dirToObject );
					foundResult = true;

					//Msg( "PUSH AWAY fwd=%f, rt=%f\n", fwd, rt );
				}
			}
		}

		if ( !foundResult )
		{
			// test if we can move through the object
			Vector vTestPosition = GetAbsOrigin() - vMoveDir * radius * 2;
			fwd = currentdir.Dot( -vMoveDir );
			rt = rightdir.Dot( -vMoveDir );

			if ( flDist < objectradius )
			{
				obj->AddEffects( EF_NODRAW );
			}

			//Msg( "PUSH THROUGH fwd=%f, rt=%f\n", fwd, rt );

			foundResult = true;
		}

		// If running, then do a lot more sideways veer since we're not going to do anything to
		//  forward velocity
		if ( istryingtomove )
		{
			sidescale = 6.0f;
		}

		if ( flVelProduct > 0.0f && flDirProduct > 0.0f )
		{
			sidescale = 0.1f;
		}

		float force = 1.0f;
		float forward = forwardscale * fwd * force * AVOID_SPEED;
		float side = sidescale * rt * force * AVOID_SPEED;

		adjustforwardmove	+= forward;
		adjustsidemove		+= side;
	}

	pCmd->forwardmove	+= adjustforwardmove;
	pCmd->sidemove		+= adjustsidemove;
	
	// Clamp the move to within legal limits, preserving direction. This is a little
	// complicated because we have different limits for forward, back, and side

	//Msg( "PRECLAMP: forwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );

	float flForwardScale = 1.0f;
	if ( pCmd->forwardmove > fabs( cl_forwardspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_forwardspeed.GetFloat() ) / pCmd->forwardmove;
	}
	else if ( pCmd->forwardmove < -fabs( cl_backspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_backspeed.GetFloat() ) / fabs( pCmd->forwardmove );
	}
	
	float flSideScale = 1.0f;
	if ( fabs( pCmd->sidemove ) > fabs( cl_sidespeed.GetFloat() ) )
	{
		flSideScale = fabs( cl_sidespeed.GetFloat() ) / fabs( pCmd->sidemove );
	}
	
	float flScale = MIN( flForwardScale, flSideScale );
	pCmd->forwardmove *= flScale;
	pCmd->sidemove *= flScale;

	//Msg( "POSTCLAMP: forwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );
}


void C_BaseHLPlayer::PerformClientSideNPCSpeedModifiers( float flFrameTime, CUserCmd *pCmd )
{
	if ( m_hClosestNPC == NULL )
	{
		if ( m_flSpeedMod != cl_forwardspeed.GetFloat() )
		{
			float flDeltaTime = (m_flSpeedModTime - gpGlobals->curtime);
			m_flSpeedMod = RemapValClamped( flDeltaTime, cl_npc_speedmod_outtime.GetFloat(), 0, m_flExitSpeedMod, cl_forwardspeed.GetFloat() );
		}
	}
	else
	{
		C_AI_BaseNPC *pNPC = dynamic_cast< C_AI_BaseNPC *>( m_hClosestNPC.Get() );

		if ( pNPC )
		{
			float flDist = (GetAbsOrigin() - pNPC->GetAbsOrigin()).LengthSqr();
			bool bShouldModSpeed = false; 

			// Within range?
			if ( flDist < pNPC->GetSpeedModifyRadius() )
			{
				// Now, only slowdown if we're facing & running parallel to the target's movement
				// Facing check first (in 2D)
				Vector vecTargetOrigin = pNPC->GetAbsOrigin();
				Vector los = ( vecTargetOrigin - EyePosition() );
				los.z = 0;
				VectorNormalize( los );
				Vector facingDir;
				AngleVectors( GetAbsAngles(), &facingDir );
				float flDot = DotProduct( los, facingDir );
				if ( flDot > 0.8 )
				{
					/*
					// Velocity check (abort if the target isn't moving)
					Vector vecTargetVelocity;
					pNPC->EstimateAbsVelocity( vecTargetVelocity );
					float flSpeed = VectorNormalize(vecTargetVelocity);
					Vector vecMyVelocity = GetAbsVelocity();
					VectorNormalize(vecMyVelocity);
					if ( flSpeed > 1.0 )
					{
						// Velocity roughly parallel?
						if ( DotProduct(vecTargetVelocity,vecMyVelocity) > 0.4  )
						{
							bShouldModSpeed = true;
						}
					} 
					else
					{
						// NPC's not moving, slow down if we're moving at him
						//Msg("Dot: %.2f\n", DotProduct( los, vecMyVelocity ) );
						if ( DotProduct( los, vecMyVelocity ) > 0.8 )
						{
							bShouldModSpeed = true;
						} 
					}
					*/

					bShouldModSpeed = true;
				}
			}

			if ( !bShouldModSpeed )
			{
				m_hClosestNPC = NULL;
				m_flSpeedModTime = gpGlobals->curtime + cl_npc_speedmod_outtime.GetFloat();
				m_flExitSpeedMod = m_flSpeedMod;
				return;
			}
			else 
			{
				if ( m_flSpeedMod != pNPC->GetSpeedModifySpeed() )
				{
					float flDeltaTime = (m_flSpeedModTime - gpGlobals->curtime);
					m_flSpeedMod = RemapValClamped( flDeltaTime, cl_npc_speedmod_intime.GetFloat(), 0, cl_forwardspeed.GetFloat(), pNPC->GetSpeedModifySpeed() );
				}
			}
		}
	}

	if ( pCmd->forwardmove > 0.0f )
	{
		pCmd->forwardmove = clamp( pCmd->forwardmove, -m_flSpeedMod, m_flSpeedMod );
	}
	else
	{
		pCmd->forwardmove = clamp( pCmd->forwardmove, -m_flSpeedMod, m_flSpeedMod );
	}
	pCmd->sidemove = clamp( pCmd->sidemove, -m_flSpeedMod, m_flSpeedMod );
   
	//Msg( "fwd %f right %f\n", pCmd->forwardmove, pCmd->sidemove );
}

//-----------------------------------------------------------------------------
// Purpose: Input handling
//-----------------------------------------------------------------------------
bool C_BaseHLPlayer::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{
	bool bResult = BaseClass::CreateMove( flInputSampleTime, pCmd );

	if ( !IsInAVehicle() )
	{
		PerformClientSideObstacleAvoidance( TICK_INTERVAL, pCmd );
		PerformClientSideNPCSpeedModifiers( TICK_INTERVAL, pCmd );
	}

	return bResult;
}