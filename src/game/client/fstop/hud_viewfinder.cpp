//==== Copyright © 2017-2018, Linus S. (PistonMiner), All rights reserved. ====//
//
// Purpose: F-Stop Camera viewfinder HUD overlay
//
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "sdk_player_shared.h"
#include "view_scene.h"

#include "fstop_common.h"
#include "weapon_camera_shared.h"
#include "weapon_placement_shared.h"

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

class CHudViewfinder : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudViewfinder, vgui::Panel);

public:
	CHudViewfinder(const char *pElementName);

	virtual void Init();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void Paint();
	virtual bool ShouldDraw();

private:
	int m_CornerTexture;
	int m_CircleTexture;
};

DECLARE_HUDELEMENT_DEPTH(CHudViewfinder, 100); // Last layer

CHudViewfinder::CHudViewfinder(const char *pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudViewfinder")
{
	vgui::Panel *pParent = GetClientMode()->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_PLAYERDEAD);
}

void CHudViewfinder::Init()
{
	m_CornerTexture = vgui::surface()->CreateNewTextureID();
	m_CircleTexture = vgui::surface()->CreateNewTextureID();

	vgui::surface()->DrawSetTextureFile(m_CornerTexture, "HUD/camera_viewfinder_ul",		 true, false);
	vgui::surface()->DrawSetTextureFile(m_CircleTexture, "HUD/camera_viewfinder_halfcircle", true, false);

	UnregisterForRenderGroup("global");
}

void CHudViewfinder::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	// Transparency
	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	int width, height;
	GetHudSize(width, height);
	SetBounds(0, 0, width, height);
}

void CHudViewfinder::Paint()
{
	// With HDR mode 3, this brightens up the entire image for some reason.
	// Presumably related to bloom. Doesn't seem to break without.
	// UpdateRefractTexture();

	int screenWidth, screenHeight;
	GetHudSize(screenWidth, screenHeight);

	float xWide = screenWidth;
	float yTall = screenHeight;
	float xMid = xWide / 2;
	float yMid = yTall / 2;

	// Quarter-screen quads
	vgui::Vertex_t vertices[4];
	vgui::surface()->DrawSetColor(0, 0, 0, 255);
	vgui::surface()->DrawSetTexture(m_CornerTexture);
	// Top left
	vertices[0].Init({ 0,     0		}, { 0.f, 0.f });
	vertices[1].Init({ xMid,  0		}, { 1.f, 0.f });
	vertices[2].Init({ xMid,  yMid	}, { 1.f, 1.f });
	vertices[3].Init({ 0,     yMid	}, { 0.f, 1.f });
	vgui::surface()->DrawTexturedPolygon(4, vertices);
	// Top right
	vertices[0].Init({ xMid,  0     }, { 1.f, 0.f });
	vertices[1].Init({ xWide, 0     }, { 0.f, 0.f });
	vertices[2].Init({ xWide, yMid  }, { 0.f, 1.f });
	vertices[3].Init({ xMid,  yMid  }, { 1.f, 1.f });
	vgui::surface()->DrawTexturedPolygon(4, vertices);
	// Bottom right
	vertices[0].Init({ xMid,  yMid  }, { 1.f, 1.f });
	vertices[1].Init({ xWide, yMid  }, { 0.f, 1.f });
	vertices[2].Init({ xWide, yTall }, { 0.f, 0.f });
	vertices[3].Init({ xMid,  yTall }, { 1.f, 0.f });
	vgui::surface()->DrawTexturedPolygon(4, vertices);
	// Bottom left
	vertices[0].Init({ 0,     yMid  }, { 0.f, 1.f });
	vertices[1].Init({ xMid,  yMid  }, { 1.f, 1.f });
	vertices[2].Init({ xMid,  yTall }, { 1.f, 0.f });
	vertices[3].Init({ 0,     yTall }, { 0.f, 0.f });
	vgui::surface()->DrawTexturedPolygon(4, vertices);

	// Draw central circles
	float radius = screenHeight * 0.15f;
	vgui::surface()->DrawOutlinedCircle(xMid, yMid, radius, 64);
	vgui::surface()->DrawOutlinedCircle(xMid, yMid, radius * 1.25f, 64);

	// Draw grill
	float grillRadius = screenHeight * 0.05f;
	vgui::surface()->DrawSetTexture(m_CircleTexture);
	// Lower
	vgui::surface()->DrawSetColor(0, 0, 0, 64);
	vertices[0].Init({ xMid - grillRadius, yMid               }, { 0.f, 1.f });
	vertices[1].Init({ xMid + grillRadius, yMid               }, { 1.f, 1.f });
	vertices[2].Init({ xMid + grillRadius, yMid + grillRadius }, { 1.f, 0.f });
	vertices[3].Init({ xMid - grillRadius, yMid + grillRadius }, { 0.f, 0.f });
	vgui::surface()->DrawTexturedPolygon(4, vertices);
	// Upper
	vgui::surface()->DrawSetColor(0, 0, 0, 32);
	vertices[0].Init({ xMid - grillRadius, yMid - grillRadius }, { 1.f, 0.f });
	vertices[1].Init({ xMid + grillRadius, yMid - grillRadius }, { 0.f, 0.f });
	vertices[2].Init({ xMid + grillRadius, yMid               }, { 0.f, 1.f });
	vertices[3].Init({ xMid - grillRadius, yMid               }, { 1.f, 1.f });
	vgui::surface()->DrawTexturedPolygon(4, vertices);

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (pPlayer)
	{
		C_WeaponCamera *pCamera = dynamic_cast<C_WeaponCamera *>(pPlayer->GetActiveWeapon());
		if (pCamera)
		{
			wchar_t formatBuf[128];
			for (int i = 0; i < pCamera->GetNumObjectsInView(); ++i)
			{
				const Vector2D cBorder = Vector2D(0.005f, 0.005f);

				CBaseEntity *pEntity = pCamera->GetObjectInView(i);
				// This can happen if this is an entity that is not networked.
				// Should not matter for finished product.
				if (!pEntity)
					continue;
				if (sv_camera_flash.GetBool())
				{
					CWeaponPlacement *place = dynamic_cast<CWeaponPlacement *>(pPlayer->Weapon_OwnsThisType("weapon_placement"));

					// only continue if the object has not been photographed yet
					if (place->HasPhotographedObject(pEntity))
						continue;
				}

				Vector2D mins, maxs;
				pCamera->GetObjectScreenspaceBoundingBox(pEntity, mins, maxs);
				mins -= cBorder;
				maxs += cBorder;

				// Bounds check
				if (mins.x < 0.f || mins.y < 0.f || maxs.x > 1.f || maxs.y > 1.f)
					continue;

				mins *= Vector2D(xWide, yTall);
				maxs *= Vector2D(xWide, yTall);
				
				float flBracketSize = MIN((maxs.x - mins.x) / 2, (maxs.y - mins.y) / 2);
				flBracketSize = clamp(flBracketSize, 0.f, 100.f);

				if (i == pCamera->GetPrimaryObjectIndex())
				{
					vgui::surface()->DrawSetColor(Color(255, 0, 0, 200));
				}
				else
				{
					vgui::surface()->DrawSetColor(Color(0, 0, 0, 200));
				}
				vgui::surface()->DrawLine(mins.x, mins.y, mins.x + flBracketSize, mins.y); // Upper left
				vgui::surface()->DrawLine(mins.x, mins.y, mins.x, mins.y + flBracketSize);
				vgui::surface()->DrawLine(maxs.x, mins.y, maxs.x - flBracketSize, mins.y); // Upper right
				vgui::surface()->DrawLine(maxs.x, mins.y, maxs.x, mins.y + flBracketSize);
				vgui::surface()->DrawLine(maxs.x, maxs.y, maxs.x - flBracketSize, maxs.y); // Lower right
				vgui::surface()->DrawLine(maxs.x, maxs.y, maxs.x, maxs.y - flBracketSize);
				vgui::surface()->DrawLine(mins.x, maxs.y, mins.x + flBracketSize, maxs.y); // Lower left
				vgui::surface()->DrawLine(mins.x, maxs.y, mins.x, maxs.y - flBracketSize);
				
				g_pVGuiLocalize->ConvertANSIToUnicode(pEntity->GetClassname(), formatBuf, sizeof(formatBuf));
				vgui::surface()->DrawSetTextColor(0, 0, 0, 200);
				vgui::surface()->DrawSetTextPos(mins.x, mins.y);
				vgui::surface()->DrawPrintText(formatBuf, wcslen(formatBuf));
			}
		}
	}

}

bool CHudViewfinder::ShouldDraw()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	C_WeaponCamera *pCamera = dynamic_cast<C_WeaponCamera *>(pPlayer->GetActiveWeapon());
	if (!pCamera)
		return false;

	if (!pCamera->IsShowingViewfinder())
		return false;

	return CHudElement::ShouldDraw();
}

