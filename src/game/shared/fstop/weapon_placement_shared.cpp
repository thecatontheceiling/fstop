//==== Copyright © 2017-2018, Linus S. (PistonMiner), All rights reserved. ====//
//
// Purpose: F-Stop Placement weapon
//
//=============================================================================//

#include "cbase.h"
//#include "convar.h"
#ifdef GAME_DLL
#include "info_placement_helper.h"
#include "physobj.h"
#endif
#include "weapon_placement_shared.h"
//#include "sdk_player_shared.h"
//#include "func_photographable_shared.h"

#include "in_buttons.h"

//#include <mathlib/mathlib.h>
//#include <mathlib/camera.h>
//#include <mathlib/ssemath.h>

//#include "vcollide_parse.h"
//#include "physics_saverestore.h"

#include "fstop_common.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// entity messages
#define PLACEMENT_MSG_PLACE_OBJECT 0 // args: 1 byte, photo number

ConVar sv_placement_range( "sv_placement_range", "512", FCVAR_REPLICATED );
ConVar sv_placement_rotate_speed( "sv_placement_rotate_speed", "90", FCVAR_REPLICATED, "Rotate speed in degrees per second" );


acttable_t	CWeaponPlacement::m_acttable[] =
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

IMPLEMENT_ACTTABLE(CWeaponPlacement);

CWeaponPlacement::CWeaponPlacement()
{
	m_nMaxPhotos = 3;
	m_iMinScaleLevel = -3;
	m_iMaxScaleLevel = 3;
}

void CWeaponPlacement::PrimaryAttack()
{
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Switch modes
	m_bPlacing = !m_bPlacing;
	if (!m_bPlacing) // only reset the scale if you swap away
	{
		m_iSelectedScaleLevel = 0;
	}
}

void CWeaponPlacement::SecondaryAttack()
{
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

	if (m_bPlacing)
	{
		// Place selected photo at selected scale
		CBasePlayer *player = GetPlayerOwner();
		if (!player)
			return;

		// Make sure that valid photo is selected
		if (m_iSelectedPhoto == -1)
		{
			Log_Msg( LOG_FSTOP, "Invalid photo selection!\n" );
			return;
		}

		// Make sure that photographed objects still exists
		CBaseEntity *targetEntity = m_PhotoEntities[m_iSelectedPhoto];
		if (!targetEntity)
		{
			Log_Msg( LOG_FSTOP, "Not all photo objects exist!\n" );
			return;
		}

		// Calculate world space transform
		Vector worldSpacePosition = CalculateWorldspacePosition();
		QAngle worldSpaceAngle = CalculateWorldspaceAngles();

		// Line of sight?
		trace_t losTrace;
		UTIL_TraceLine(player->EyePosition(), worldSpacePosition, MASK_VISIBLE, player, COLLISION_GROUP_NONE, &losTrace);
		if (losTrace.startsolid || losTrace.fraction != 1.0)
		{
			Log_Msg( LOG_FSTOP, "Target placement location is obstructed!\n" );
			return;
		}
#ifdef GAME_DLL
		// look for info_placement_helper
		CBaseEntity *potentialHelpers[128];
		int potentialHelperCount = UTIL_EntitiesInSphere(potentialHelpers, ARRAYSIZE(potentialHelpers), worldSpacePosition, 64.f, 0);
		for (int i = 0; i < potentialHelperCount; ++i)
		{
			CInfoPlacementHelper *pHelper = dynamic_cast<CInfoPlacementHelper *>( potentialHelpers[i] );
			if ( pHelper && UTIL_DistApprox( worldSpacePosition, pHelper->GetAbsOrigin() ) <= pHelper->GetRadius() )
			{
				worldSpacePosition = pHelper->GetAbsOrigin();
				if ( pHelper->UseAngles() )
					worldSpaceAngle = pHelper->GetAbsAngles();
				break;
			}
		}
#endif

		CBaseAnimating *pInstance = dynamic_cast<CBaseAnimating *>(targetEntity);

#ifdef GAME_DLL
		// Teleport
		targetEntity->Teleport(&worldSpacePosition, &worldSpaceAngle, &vec3_origin);
#endif
		// wake up object
		IPhysicsObject *temper = targetEntity->VPhysicsGetObject();
		temper->EnableMotion( true );
		temper->Wake();


		pInstance->SetRenderMode( kRenderNormal );
		pInstance->SetSolid( SOLID_VPHYSICS );
		pInstance->SetMoveType( MOVETYPE_VPHYSICS );

#ifdef GAME_DLL
		EntityMessageBegin( this );
			WRITE_BYTE( PLACEMENT_MSG_PLACE_OBJECT );
			WRITE_BYTE( m_iSelectedPhoto );
		MessageEnd();
#endif
		
		RemovePhoto(m_iSelectedPhoto);
		m_iSelectedScaleLevel = 0; // reset the scale level since we are resetting
		// End placement
		m_bPlacing = false;
	}
}


void CWeaponPlacement::ItemPostFrame()
{
	BaseClass::ItemPostFrame();
	CBasePlayer *pPlayer = GetPlayerOwner();
	if (pPlayer)
	{
		if (pPlayer->m_nButtons & IN_ZOOM_IN)
		{
			if (!m_bPlacing)
				SelectPreviousPhoto();
			else
				SelectNextScaleLevel();
		}
		else if (pPlayer->m_nButtons & IN_ZOOM_OUT)
		{
			if (!m_bPlacing)
				SelectNextPhoto();
			else
				SelectPreviousScaleLevel();
		}
	}

	if (m_iSelectedPhoto != -1 && m_PhotoEntities[m_iSelectedPhoto])
	{
#ifdef GAME_DLL
		CBaseEntity *pEntity = m_PhotoEntities[m_iSelectedPhoto];

		// update position / rotation
		Vector worldPosition = CalculateWorldspacePosition();
		QAngle worldAngles = CalculateWorldspaceAngles();
		pEntity->Teleport( &worldPosition, &worldAngles, &vec3_origin );
#endif

		//NDebugOverlay::BoxAngles(CalculateWorldspacePosition(m_PhotoEyePositions[m_iSelectedPhoto] * powf(2.f, m_iSelectedScaleLevel)),
		//						 m_PhotoEntities[m_iSelectedPhoto]->CollisionProp()->OBBMins() * powf(2.f, m_iSelectedScaleLevel),
		//						 m_PhotoEntities[m_iSelectedPhoto]->CollisionProp()->OBBMaxs() * powf(2.f, m_iSelectedScaleLevel),
		//						 CalculateWorldspaceAngles(m_PhotoWorldAngles[m_iSelectedPhoto]),
		//						 0, m_bPlacing ? 0 : 255, m_bPlacing ? 255 : 0, 128,
		//						 0.f);
	}
}

bool CWeaponPlacement::Deploy()
{
	// TODO: this should be *after* deployment
	// or otherwise find a way to run ItemPostFrame during deploy anim

	if ( m_iSelectedPhoto < 0 ) m_iSelectedPhoto = 0;
	SelectPhoto( m_iSelectedPhoto );

	return BaseClass::Deploy();
}

bool CWeaponPlacement::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if ( BaseClass::Holster( pSwitchingTo ) )
	{
		SelectPhoto( -1 );
		return true;
	}
	return false;
}

//Has ammo needs to exist so you can't select the weapon
bool CWeaponPlacement::HasAmmo()
{
	return m_nPhotoCount > 0;
}

//Has any ammo needs to exist to swap away from the weapon
bool CWeaponPlacement::HasAnyAmmo()
{
	return m_nPhotoCount > 0;
}

bool CWeaponPlacement::HasPhotographedObject(CBaseEntity *entity)
{
	// do not let the player photograph an object more than once, issues with other photos otherwise
	for (int i = 0; i < m_nPhotoCount; ++i)
	{
		CBaseEntity * temp = m_PhotoEntities[i];
		if (temp == entity) {
			return true;
		}
	}
	return false;
}


bool CWeaponPlacement::RegisterPhoto(CBaseEntity *entity, const Vector &eyePosition, const QAngle &eyeAngle)
{
	if (HasPhotographedObject(entity))
	{
		Log_Msg(LOG_FSTOP, "Player has already photographed this object\n");
		return false;
	}

	if (m_nPhotoCount < m_nMaxPhotos)
	{
		int index = m_nPhotoCount++;
		m_iSelectedPhoto = index;			// make sure the last photo is the one the player switches to
		m_PhotoEntities.Set(index, entity);
		//m_PhotoEyePositions.Set(index, eyePosition);
		m_PhotoWorldAngles.Set(index, entity->GetAbsAngles());

		Log_Msg( LOG_FSTOP, "Photo %d registered: pos { %.2f / %.2f / %.2f }, rot { %.2f / %.2f / %.2f }\n",
			index,
			eyePosition.x, eyePosition.y, eyePosition.z,
			eyeAngle.x, eyeAngle.y, eyeAngle.z );
		Log_Msg( LOG_FSTOP, "Entity: \"%s\" [%s]\n", entity->GetEntityName(), entity->GetClassname() );
		return true;
	}
	else
	{
		Log_Msg( LOG_FSTOP, "Maximum photos reached!\n" );
		return false;
	}
}

bool CWeaponPlacement::HasPhotos() const
{
	return m_nPhotoCount > 0;
}

void CWeaponPlacement::ResetPhotos()
{
	// remove the photos themselves
	for (int i = 0; i < m_nPhotoCount; ++i) {
		RemovePhoto(0);
	}
	m_iSelectedScaleLevel = 0;
	SelectPhoto( -1 );
	m_nPhotoCount = 0;
}

void CWeaponPlacement::RemovePhoto(int index)
{
	if (index >= m_nPhotoCount || index < 0)
		return;

	// Fill the gap
	for (int i = index; i < m_nPhotoCount - 1; ++i)
	{
		m_PhotoEntities.Set(i, m_PhotoEntities.Get(i + 1));
		//m_PhotoEyePositions.Set(i, m_PhotoEyePositions.Get(i + 1));
		m_PhotoWorldAngles.Set(i, m_PhotoWorldAngles.Get(i + 1));
	}

	Log_Msg( LOG_FSTOP, "RemovePhoto: Removing photo %d, photo count was %d\n", index, m_nPhotoCount );
	m_nPhotoCount--;

	m_bPlacing = false;

	int nextPhoto = m_iSelectedPhoto;
	m_iSelectedPhoto = -1; // changing from null photo
	
	if (m_nPhotoCount == 0)
	{
		// auto switch back to the camera if no photos are left
		SelectPhoto( -1 );
	}
	else // photo count is 1 or more
	{
		// decrease photo id until we have found a valid one
		while ( nextPhoto >= m_nPhotoCount ) nextPhoto--;
		SelectPhoto( nextPhoto );
	}
}

void CWeaponPlacement::SelectPhoto( int n )
{
	if ( n > m_nPhotoCount ) {
		return;
	}
	
	// reset scale of previous object
	SelectScaleLevel( 0 );

	CBaseEntity *pEntity;

	// clean up currently selected entity
	if (m_iSelectedPhoto >= 0)
	{
		pEntity = m_PhotoEntities[m_iSelectedPhoto];
		pEntity->SetRenderMode( kRenderNone );
	}

	// selecting negative photo = no photo
	if ( m_nPhotoCount <= 0 || n < 0 ) {
		m_iSelectedPhoto = -1;
		return;
	}

	SelectScaleLevel( 0 );

	m_iSelectedPhoto = n;
	Log_Msg( LOG_FSTOP, "Selected photo %d\n", m_iSelectedPhoto );

	pEntity = m_PhotoEntities[m_iSelectedPhoto];

	pEntity->SetRenderMode( kRenderNormal );

}

void CWeaponPlacement::SelectPreviousPhoto()
{
	// if we're selecting a photo before photo 0
	if (m_iSelectedPhoto <= 0)
	{
		// loop back to last photo
		SelectPhoto( m_nPhotoCount - 1 );
	}
	else
	{
		// select previous photo
		SelectPhoto( m_iSelectedPhoto - 1 );
	}
}

void CWeaponPlacement::SelectNextPhoto()
{
	// if we're selecting a photo before photo 0
	if ( m_iSelectedPhoto >= m_nPhotoCount -1 )
	{
		// loop back to first photo
		SelectPhoto( 0 );
	}
	else
	{
		// select previous photo
		SelectPhoto( m_iSelectedPhoto + 1 );
	}
}

void CWeaponPlacement::SelectScaleLevel( int n )
{
	if ( n > m_iMaxScaleLevel || n < m_iMinScaleLevel ) return;
	m_iSelectedScaleLevel = n;

#ifdef GAME_DLL
	CBaseEntity *pEntity = m_PhotoEntities[m_iSelectedPhoto];
	if ( pEntity )
	{
		CBaseAnimating *pInstance = dynamic_cast<CBaseAnimating *>( pEntity );
		if ( pInstance )
		{
			float newScale = powf( 2.f, m_iSelectedScaleLevel );

			// Don't scale NPCs, perhaps use oobb
			if ( pInstance->MyCombatCharacterPointer() )
				return;

			pInstance->SetModelScale( newScale );
			pInstance->UpdateModelScale();

			UTIL_CreateScaledPhysObject( pInstance, newScale );
		}
	}
#endif
}

void CWeaponPlacement::SelectPreviousScaleLevel()
{
	SelectScaleLevel( m_iSelectedScaleLevel - 1 );
	Log_Msg( LOG_FSTOP, "Scale level %d\n", m_iSelectedScaleLevel );
}

void CWeaponPlacement::SelectNextScaleLevel()
{
	SelectScaleLevel( m_iSelectedScaleLevel + 1 );
	Log_Msg( LOG_FSTOP, "Scale level %d\n", m_iSelectedScaleLevel );
}

Vector CWeaponPlacement::CalculateWorldspacePosition()
{
	CBasePlayer *player = GetPlayerOwner();
	CBaseEntity *entity = m_PhotoEntities[m_iSelectedPhoto];

	if ( !player || !entity )
		return vec3_origin;

	Vector forward;
	player->EyeVectors( &forward );

	Vector start = player->Weapon_ShootPosition();
	Vector end = start + forward * sv_placement_range.GetInt();

	trace_t trace;
	
	UTIL_TraceEntity( entity, start, end, MASK_SHOT, player, COLLISION_GROUP_NONE, &trace );
	if ( trace.DidHit() )
	{
		return trace.endpos + trace.plane.normal;
	}
	else
	{
		return trace.endpos;
	}
}

QAngle CWeaponPlacement::CalculateWorldspaceAngles()
{
	/*CBasePlayer *player = GetPlayerOwner();
	if (!player)
		return vec3_angle;

	Quaternion quatEyeToWorld;
	AngleQuaternion(player->EyeAngles(), quatEyeToWorld);
	
	Quaternion quatRotation;
	AngleQuaternion(eyeAngles, quatRotation);
	QuaternionMult(quatEyeToWorld, quatRotation, quatRotation);

	QAngle worldAngles;
	QuaternionAngles(quatRotation, worldAngles);
	return worldAngles;*/

	QAngle ang = m_PhotoWorldAngles[m_iSelectedPhoto];
	ang.y += fmod( (gpGlobals->curtime * sv_placement_rotate_speed.GetFloat()), 360 );

	return ang;
}

#ifdef CLIENT_DLL
void CWeaponPlacement::ReceiveMessage( int classID, bf_read &msg )
{
	int messageType = msg.ReadByte();
	int i = msg.ReadByte();
	CBaseEntity *pEntity = m_PhotoEntities[i];
	switch ( messageType )
	{
		case PLACEMENT_MSG_PLACE_OBJECT:
			if ( !pEntity ) return;
			pEntity->SetBlurState( false );
			break;
		default:
			break;
	}

	BaseClass::ReceiveMessage( classID, msg );
}

#endif


IMPLEMENT_NETWORKCLASS_ALIASED(WeaponPlacement, DT_WeaponPlacement)

BEGIN_NETWORK_TABLE(CWeaponPlacement, DT_WeaponPlacement)
#ifdef CLIENT_DLL
	RecvPropInt(RECVINFO(m_nMaxPhotos)),
	RecvPropInt(RECVINFO(m_iSelectedPhoto)),
	RecvPropInt(RECVINFO(m_nPhotoCount)),
	//RecvPropEHandle(RECVINFO(m_hPhotoShadow)),
	RecvPropArray3(RECVINFO_ARRAY(m_PhotoEntities), RecvPropEHandle(RECVINFO(m_PhotoEntities[0]))),
	//RecvPropArray3(RECVINFO_ARRAY(m_PhotoEyePositions), RecvPropVector(RECVINFO(m_PhotoEyePositions[0]))),
	RecvPropArray3(RECVINFO_ARRAY(m_PhotoWorldAngles), RecvPropQAngles(RECVINFO(m_PhotoWorldAngles[0]))),
	RecvPropBool(RECVINFO(m_bPlacing)),
	RecvPropInt(RECVINFO(m_iMinScaleLevel)),
	RecvPropInt(RECVINFO(m_iMaxScaleLevel)),
	RecvPropInt(RECVINFO(m_iSelectedScaleLevel)),
#else
	SendPropInt(SENDINFO(m_nMaxPhotos)),
	SendPropInt(SENDINFO(m_iSelectedPhoto)),
	SendPropInt(SENDINFO(m_nPhotoCount)),
	//SendPropEHandle(SENDINFO(m_hPhotoShadow)),
	SendPropArray3(SENDINFO_ARRAY3(m_PhotoEntities), SendPropEHandle(SENDINFO_ARRAY(m_PhotoEntities))),
	//SendPropArray3(SENDINFO_ARRAY3(m_PhotoEyePositions), SendPropVector(SENDINFO_ARRAY(m_PhotoEyePositions), SPROP_COORD)),
	SendPropArray3( SENDINFO_ARRAY3( m_PhotoWorldAngles ), SendPropQAngles( SENDINFO_ARRAY( m_PhotoWorldAngles ), SPROP_COORD ) ),
	SendPropBool(SENDINFO(m_bPlacing)),
	SendPropInt(SENDINFO(m_iMinScaleLevel)),
	SendPropInt(SENDINFO(m_iMaxScaleLevel)),
	SendPropInt(SENDINFO(m_iSelectedScaleLevel)),
#endif
END_NETWORK_TABLE()

BEGIN_DATADESC(CWeaponPlacement)
	DEFINE_KEYFIELD(m_nMaxPhotos, FIELD_INTEGER, "PhotoCapacity"),
	DEFINE_FIELD(m_iSelectedPhoto, FIELD_INTEGER),
	DEFINE_FIELD(m_nPhotoCount, FIELD_INTEGER),
	//DEFINE_FIELD(m_hPhotoShadow, FIELD_EHANDLE),
	DEFINE_AUTO_ARRAY(m_PhotoEntities, FIELD_EHANDLE),
	//DEFINE_AUTO_ARRAY(m_PhotoEyePositions, FIELD_VECTOR),
	DEFINE_AUTO_ARRAY(m_PhotoWorldAngles, FIELD_VECTOR),
	DEFINE_FIELD(m_bPlacing, FIELD_BOOLEAN),
	DEFINE_KEYFIELD(m_iMinScaleLevel, FIELD_INTEGER, "MinScaleLevel"),
	DEFINE_KEYFIELD(m_iMaxScaleLevel, FIELD_INTEGER, "MaxScaleLevel"),
	DEFINE_FIELD(m_iSelectedScaleLevel, FIELD_INTEGER),
END_DATADESC()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponPlacement)
	DEFINE_PRED_FIELD(m_nMaxPhotos, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_iSelectedPhoto, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_nPhotoCount, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	//DEFINE_PRED_FIELD(m_hPhotoShadow, FIELD_EHANDLE,FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_ARRAY(m_PhotoEntities, FIELD_EHANDLE, 16, FTYPEDESC_INSENDTABLE),
	//DEFINE_PRED_ARRAY(m_PhotoEyePositions, FIELD_VECTOR, 16, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_ARRAY(m_PhotoWorldAngles, FIELD_VECTOR, 16, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_bPlacing, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_iMinScaleLevel, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_iMaxScaleLevel, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_iSelectedScaleLevel, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_placement, CWeaponPlacement);
PRECACHE_WEAPON_REGISTER(weapon_placement);