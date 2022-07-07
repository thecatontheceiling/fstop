#include "cbase.h"
#include "multiplayer/basenetworkedplayer_gamemove.h"

#if !defined(PORTAL) && !defined(HL2_DLL) && !defined(HL2_CLIENT_DLL)
static CNetworkedPlayerMovement g_GameMovement;
IGameMovement* g_pGameMovement = (IGameMovement*)&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CNetworkedPlayerMovement, IGameMovement,INTERFACENAME_GAMEMOVEMENT, g_GameMovement );
#endif

#ifdef CLIENT_DLL
#include "multiplayer/basenetworkedplayer_cl.h"
#else
#include "multiplayer/basenetworkedplayer.h"
#endif

bool CNetworkedPlayerMovement::CheckJumpButton()
{
	bool HasJumped = BaseClass::CheckJumpButton();

	if (HasJumped)
		static_cast<CBaseNetworkedPlayer*>(player)->DoAnimationEvent(PLAYERANIMEVENT_JUMP);

	return HasJumped;
}