//==== Copyright © 2017-2018, Linus S. (PistonMiner), All rights reserved. ====//
//
// Purpose: F-Stop cleanser that removes photos
//
//=============================================================================//

#include "cbase.h"
#include "triggers.h"

#include "weapon_placement_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CTriggerFStopCleanser : public CBaseTrigger
{
public:
	DECLARE_CLASS(CTriggerFStopCleanser, CBaseTrigger);

	virtual void Spawn() override;
	virtual void Touch(CBaseEntity *pOther) override;

	DECLARE_DATADESC();
};

BEGIN_DATADESC(CTriggerFStopCleanser)
END_DATADESC()

LINK_ENTITY_TO_CLASS(trigger_fstop_cleanser, CTriggerFStopCleanser);

void CTriggerFStopCleanser::Spawn()
{
	BaseClass::Spawn();
	InitTrigger();
}

void CTriggerFStopCleanser::Touch(CBaseEntity *pOther)
{
	if (!PassesTriggerFilters(pOther))
		return;

	if (pOther->IsPlayer())
	{
		CBasePlayer *player = dynamic_cast<CBasePlayer *>(pOther);
		if (player)
		{
			CWeaponPlacement *placement = dynamic_cast<CWeaponPlacement *>(player->Weapon_OwnsThisType("weapon_placement"));
			if (placement && placement->HasPhotos())
			{
				placement->ResetPhotos();
			}
		}
	}
}
