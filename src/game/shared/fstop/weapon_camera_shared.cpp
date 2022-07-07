//==== Copyright © 2017-2018, Linus S. (PistonMiner), All rights reserved. ====//
//
// Purpose: F-Stop Camera weapon
//
//=============================================================================//

#include "cbase.h"
#include "weapon_camera_shared.h"
//#include "sdk_player_shared.h"
#include "weapon_placement_shared.h"

#include "in_buttons.h"

#include <mathlib/mathlib.h>
#include <mathlib/camera.h>
#include <mathlib/ssemath.h>

#ifdef CLIENT_DLL
#include "iviewrender.h"
#include "fstop/viewrender_camera.h"
#include "fstop/fstop_render_targets.h"
#endif

#ifdef GAME_DLL
#include "enginecallback.h"
#include "triggers.h"
#endif

#include "fstop_common.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// entity messages
#define CAMERA_MSG_TAKE_PHOTO 0 // args: 1 byte, photo number

acttable_t	CWeaponCamera::m_acttable[] =
{
	{ ACT_MP_STAND_IDLE,				ACT_MP_STAND_PRIMARY,					false },
	{ ACT_MP_RUN,						ACT_MP_RUN_PRIMARY,						false },
	{ ACT_MP_CROUCH_IDLE,				ACT_MP_CROUCH_PRIMARY,					false },
	{ ACT_MP_CROUCHWALK,				ACT_MP_CROUCHWALK_PRIMARY,				false },
	{ ACT_MP_JUMP_START,				ACT_MP_JUMP_START_PRIMARY,				false },
	{ ACT_MP_JUMP_FLOAT,				ACT_MP_JUMP_FLOAT_PRIMARY,				false },
	{ ACT_MP_JUMP_LAND,					ACT_MP_JUMP_LAND_PRIMARY,				false },
	{ ACT_MP_AIRWALK,					ACT_MP_AIRWALK_PRIMARY,					false },
};

IMPLEMENT_ACTTABLE(CWeaponCamera);

CWeaponCamera::CWeaponCamera()
{
	m_bShowingViewfinder = false;
	m_iPrimaryObject = -1;
	m_nNumEntitiesInView = 0;
	m_flFieldOfView = 60.0f;
}

void CWeaponCamera::Spawn()
{
	BaseClass::Spawn();

	

	SetThink(&CWeaponCamera::Think);
	SetNextThink( gpGlobals->curtime + 0.1f );
	
#ifdef CLIENT_DLL	
	
	//SetNextClientThink( CLIENT_THINK_ALWAYS );
#endif
}

void CWeaponCamera::Think()
{

	if (m_bShowingViewfinder)
	{
		UpdateObjectsInView();
	}

	SetNextThink(gpGlobals->frametime + 0.1f);
}

void CWeaponCamera::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );
#ifdef GAME_DLL
	CBasePlayer* pPlayer = GetPlayerOwner();
	if ( pPlayer && !pPlayer->HasNamedPlayerItem( "weapon_placement" ) )
	{
		DevMsg( "weapon_camera: giving weapon_placement to player.\n" );
		pPlayer->GiveNamedItem( "weapon_placement" );
		pPlayer->SelectItem( "weapon_camera" );
	}
#endif
}

void CWeaponCamera::PrimaryAttack()
{
	// Need to be zoomed in to do anything
	if (!m_bShowingViewfinder)
		return;

	// Take a photo
	TakePhoto();

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
}

void CWeaponCamera::SecondaryAttack()
{
	SetShowingViewfinder(!m_bShowingViewfinder);
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}

bool CWeaponCamera::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	// Unzoom
	SetShowingViewfinder(false);

	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponCamera::SetShowingViewfinder(bool bShow)
{
	m_bShowingViewfinder = bShow;

	CBasePlayer *player = GetPlayerOwner();
	if (player)
	{
#ifndef CLIENT_DLL
		player->ShowViewModel(!bShow);
#endif
		player->SetFOV(this, bShow ? m_flFieldOfView : 0, 0.2f);
	}
}

Vector2D CWeaponCamera::GetScreenspacePosition(const Vector &worldSpacePosition)
{
	Camera_t camera;
	camera.m_origin = GetPlayerOwner()->EyePosition();
	camera.m_angles = GetPlayerOwner()->EyeAngles();
	camera.m_flZNear = m_flCameraMinimumRange;
	camera.m_flZFar = m_flCameraMaximumRange;
	camera.m_flFOVX = m_flFieldOfView;

	Vector2D effectiveScreenSize;
	effectiveScreenSize.x = 16;
	effectiveScreenSize.y = effectiveScreenSize.x / m_flAspectRatio;

	Vector2D screenPosition;
	ComputeScreenSpacePosition(&screenPosition, worldSpacePosition, camera, effectiveScreenSize.x, effectiveScreenSize.y);
	// After a long debugging session I come to this coordinate fixup.
	screenPosition = screenPosition / effectiveScreenSize * 0.75f + Vector2D(0.125f, 0.125f);

	return screenPosition;
}

static void GenerateBoxVertices(const Vector &vOrigin, const QAngle& angles, const Vector &vMins, const Vector &vMaxs, Vector pVerts[8])
{
	// Build a rotation matrix from orientation
	matrix3x4_t fRotateMatrix;
	AngleMatrix(angles, fRotateMatrix);

	Vector vecPos;
	for (int i = 0; i < 8; ++i)
	{
		vecPos[0] = (i & 0x1) ? vMaxs[0] : vMins[0];
		vecPos[1] = (i & 0x2) ? vMaxs[1] : vMins[1];
		vecPos[2] = (i & 0x4) ? vMaxs[2] : vMins[2];

		VectorRotate(vecPos, fRotateMatrix, pVerts[i]);
		pVerts[i] += vOrigin;
	}
}

void CWeaponCamera::GetObjectScreenspaceBoundingBox(const CBaseEntity *pEntity, Vector2D &min, Vector2D &max)
{
	Vector vertices[8];
	const CCollisionProperty *pCollision = pEntity->CollisionProp();
	GenerateBoxVertices(pCollision->GetCollisionOrigin(), pCollision->GetCollisionAngles(), pCollision->OBBMins(), pCollision->OBBMaxs(), vertices);

	Vector2D screenMin = Vector2D(FLT_MAX, FLT_MAX);
	Vector2D screenMax = Vector2D(-FLT_MAX, -FLT_MAX);
	for (int i = 0; i < 8; ++i)
	{
		Vector2D vertexPosition = GetScreenspacePosition(vertices[i]);
		Vector2DMin(screenMin, vertexPosition, screenMin);
		Vector2DMax(screenMax, vertexPosition, screenMax);
	}
	min = screenMin;
	max = screenMax;
}

int CWeaponCamera::GetNumObjectsInView()
{
	return m_nNumEntitiesInView;
}

CBaseEntity *CWeaponCamera::GetObjectInView(int index)
{
	return m_hEntitiesInView[index];
}

int CWeaponCamera::GetPrimaryObjectIndex()
{
	return m_iPrimaryObject;
}

void CWeaponCamera::UpdateObjectsInView()
{
	CBasePlayer *player = GetPlayerOwner();
	if (!player)
		return;

	float flAspectRatio = 16.f / 9.f;

	// Broad pass using UTIL_EntitiesInBox
	QAngle viewAngles = player->EyeAngles();
	Vector viewPosition = player->EyePosition();
	Vector viewForwardDirection;
	AngleVectors(viewAngles, &viewForwardDirection);

	float width = tanf(DEG2RAD(m_flFieldOfView) / 2.f) * m_flCameraMaximumRange * 2.f;
	CBaseEntity *potentialEntities[128];
	int potentialEntityCount = UTIL_EntitiesInSphere(potentialEntities, ARRAYSIZE(potentialEntities),
													 viewPosition + viewForwardDirection * m_flCameraMaximumRange,
													 width, 0);

	Frustum_t cameraTargetFrustum;
	GeneratePerspectiveFrustum(viewPosition, viewAngles,
							   m_flCameraMinimumRange, m_flCameraMaximumRange,
							   m_flFieldOfView * 1.1f, flAspectRatio,
							   cameraTargetFrustum);
	
	// Fine pass using frustum
	m_nNumEntitiesInView = 0;
	for (int i = 0; i < potentialEntityCount; ++i)
	{
		// Photographable object?
		if (!FClassnameIs(potentialEntities[i], "prop_physics"))
			continue;

		// In view frustum?
		Vector min, max;
		potentialEntities[i]->CollisionProp()->WorldSpaceSurroundingBounds(&min, &max);
		if (cameraTargetFrustum.CullBox(min, max))
			continue;

		// Fully on-screen?
		Vector2D screenMin, screenMax;
		GetObjectScreenspaceBoundingBox(potentialEntities[i], screenMin, screenMax);
		if (screenMin.x < 0.f || screenMin.y < 0.f || screenMax.x > 1.f || screenMax.y > 1.f)
			continue;

		// Line of sight?
		trace_t losTrace;
		UTIL_TraceLine(player->EyePosition(), potentialEntities[i]->GetAbsOrigin(), MASK_VISIBLE, player, COLLISION_GROUP_NONE, &losTrace);
		if (losTrace.fraction != 1.0 && losTrace.m_pEnt != potentialEntities[i])
			continue;

#ifndef CLIENT_DLL
		// Fizzler blocking?
		CBaseEntity *blockers[1024];
		Ray_t ray;
		ray.Init(player->EyePosition(), potentialEntities[i]->GetAbsOrigin());
		int blockerCount = UTIL_EntitiesAlongRay(blockers, ARRAYSIZE(blockers), ray, 0);
		bool blocked = false;
		for (int j = 0; j < blockerCount; ++j)
		{
			if (FClassnameIs(blockers[j], "trigger_fstop_cleanser"))
			{
				CBaseTrigger *trigger = dynamic_cast<CBaseTrigger *>(blockers[j]);
				if (trigger && !trigger->m_bDisabled)
				{
					blocked = true;
					break;
				}
			}
		}
		if (blocked)
			continue;

		
#endif

		// Passed all checks
		m_hEntitiesInView.Set(m_nNumEntitiesInView++, potentialEntities[i]);
		if (m_nNumEntitiesInView >= m_hEntitiesInView.Count())
			break;
	}

	// Find primary object
	m_iPrimaryObject = -1;
	float minObjectDist = FLT_MAX;
	for (int i = 0; i < m_nNumEntitiesInView; ++i)
	{
		float currObjectDist = GetScreenspacePosition(m_hEntitiesInView[i]->GetAbsOrigin()).DistTo(Vector2D(0.5f, 0.5f));
		if (currObjectDist < minObjectDist)
		{
			minObjectDist = currObjectDist;
			m_iPrimaryObject = i;
		}
	}
}

#ifdef CLIENT_DLL

void CWeaponCamera::ReceiveMessage( int classID, bf_read &msg )
{
	int messageType = msg.ReadByte();
	switch (messageType)
	{
		case CAMERA_MSG_TAKE_PHOTO:
			TakePhoto(msg.ReadByte());
			break;
		default:
			break;
	}

	BaseClass::ReceiveMessage( classID, msg );
}

void CWeaponCamera::ClientThink()
{
	BaseClass::ClientThink();
}


bool CWeaponCamera::ShouldDrawCrosshair()
{
	return !IsShowingViewfinder();
}

#endif

void CWeaponCamera::TakePhoto(int photoNum)
{

	CBasePlayer *player = GetPlayerOwner();
	if (!player)
		return;

	// We can only take a photograph of one entity at a time
	int objectIndex = GetPrimaryObjectIndex();
	if (objectIndex == -1)
	{
		Log_Msg( LOG_FSTOP, "Nothing to photograph!\n" );
		return;
	}

	CBaseEntity *object = GetObjectInView(objectIndex);

#ifdef CLIENT_DLL
	
	ITexture *pTexture = g_pFSTOPRenderTargets->GetPhotoTexture( photoNum );
	if (pTexture == nullptr || !pTexture || pTexture->IsError())
	{
		Log_Warning( LOG_FSTOP, "Warning: Run out of render targets. Photo %d will be empty.\n", photoNum );
		return;
	}

	ViewDrawPhoto( (CViewRender*)GetViewRenderInstance(), pTexture, object );
#endif

#ifdef GAME_DLL
	// Produce eyespace transform
	Quaternion quatWorldToEye;
	AngleQuaternion(player->EyeAngles(), quatWorldToEye);
	QuaternionInvert(quatWorldToEye, quatWorldToEye);

	Vector positionEyespace;
	VectorRotate(object->GetAbsOrigin() - player->EyePosition(), quatWorldToEye, positionEyespace);
	
	Quaternion quatRotation;
	AngleQuaternion(object->GetAbsAngles(), quatRotation);
	QuaternionMult(quatWorldToEye, quatRotation, quatRotation);

	QAngle angleEyespace;
	QuaternionAngles(quatRotation, angleEyespace);
	
	CWeaponPlacement *place = dynamic_cast<CWeaponPlacement *>(player->Weapon_OwnsThisType("weapon_placement"));
	if (place)
	{
		bool success = place->RegisterPhoto( object, positionEyespace, angleEyespace );
		if (success)
		{
			EntityMessageBegin( this );
				WRITE_BYTE( CAMERA_MSG_TAKE_PHOTO );
				WRITE_BYTE( place->GetPhotoCount() - 1 );
			MessageEnd();

			if (sv_camera_flash.GetBool())
			{
				
				Log_Msg( LOG_FSTOP, "[DISAPPEARING] SOLID: %d, MOVETYPE: %d, RENDERMODE: %d\n",
						 object->GetSolid(),
						 object->GetMoveType(),
						 object->GetRenderMode() );
				
				// make the object dissapear
				object->SetSolid(SOLID_NONE);
				object->SetMoveType(MOVETYPE_NOCLIP);
				object->SetRenderMode(kRenderNone);

				IPhysicsObject *pObject = object->VPhysicsGetObject();
				pObject->EnableMotion( false );

				color32 white = { 255,255,255,255 };
				UTIL_ScreenFade(player, white, 0.3f, 0.5f, FFADE_IN);
			}

			WeaponSound( SINGLE );
		}
	}
	else
	{
		Log_Warning( LOG_FSTOP, "Player doesn't have CWeaponPlacement!!!\n" );
	}
#endif
}

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCamera, DT_WeaponCamera )

BEGIN_NETWORK_TABLE( CWeaponCamera, DT_WeaponCamera )
#ifdef CLIENT_DLL
	RecvPropFloat( RECVINFO( m_flFieldOfView ) ),
	RecvPropBool( RECVINFO( m_bShowingViewfinder ) ),
	RecvPropInt( RECVINFO( m_nNumEntitiesInView ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_hEntitiesInView ), RecvPropEHandle( RECVINFO( m_hEntitiesInView[0] ) ) ),
	RecvPropInt( RECVINFO( m_iPrimaryObject ) ),
#else
	SendPropFloat( SENDINFO( m_flFieldOfView ) ),
	SendPropBool( SENDINFO( m_bShowingViewfinder ) ),
	SendPropInt( SENDINFO( m_nNumEntitiesInView ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_hEntitiesInView ), SendPropEHandle( SENDINFO_ARRAY( m_hEntitiesInView ) ) ),
	SendPropInt( SENDINFO( m_iPrimaryObject ) ),
#endif
END_NETWORK_TABLE()

BEGIN_DATADESC( CWeaponCamera )
	DEFINE_KEYFIELD( m_flFieldOfView, FIELD_FLOAT, "FieldOfView" ),
	DEFINE_FIELD( m_bShowingViewfinder, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nNumEntitiesInView, FIELD_INTEGER ),
	DEFINE_ARRAY( m_hEntitiesInView, FIELD_EHANDLE, 16 ),
	DEFINE_FIELD( m_iPrimaryObject, FIELD_INTEGER ),
END_DATADESC()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponCamera )
	DEFINE_PRED_FIELD( m_flFieldOfView, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bShowingViewfinder, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nNumEntitiesInView, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_ARRAY( m_hEntitiesInView, FIELD_EHANDLE, 16, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iPrimaryObject, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_camera, CWeaponCamera);
PRECACHE_WEAPON_REGISTER(weapon_camera);