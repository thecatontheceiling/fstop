//==== Copyright © 2017-2018, Lever Softworks, All rights reserved. ====//
//
// Purpose: 
//
//======================================================================//

#include "cbase.h"

#include "viewrender_camera.h"

#include "fstop_common.h"

#include "ivrenderview.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// from viewrender.cpp
extern void SetupCurrentView( const Vector &vecOrigin, const QAngle &angles, view_id_t viewID, bool bDrawWorldNormal = false, bool bCullFrontFaces = false );

struct AperturePhotoViewQueue_t
{
	EHANDLE hEnt;
	ITexture *pTexture;
	int iFailedTries;
};

CUtlVector<AperturePhotoViewQueue_t> g_AperturePhotoQueue;

void Aperture_QueuePhotoView( EHANDLE hPhotoEntity, ITexture *pRenderTarget )
{
	if ( pRenderTarget == NULL )
		return;

	AperturePhotoViewQueue_t temp;
	temp.hEnt = hPhotoEntity;
	temp.pTexture = pRenderTarget;
	temp.iFailedTries = 0;

	g_AperturePhotoQueue.AddToTail( temp );
}

//-----------------------------------------------------------------------------
// view of a single entity by itself
//-----------------------------------------------------------------------------
class CAperturePhotoView : public CSimpleWorldView
{
	DECLARE_CLASS( CAperturePhotoView, CSimpleWorldView );
public:
	CAperturePhotoView( CViewRender *pMainView );

	bool			Setup( C_BaseEntity *pTargetEntity, const CViewSetup &view, int *pClearFlags, SkyboxVisibility_t *pSkyboxVisible, ITexture *pRenderTarget = NULL );

	//Skybox drawing through portals with workarounds to fix area bits, position/scaling, view id's..........
	void			Draw();

#ifdef PORTAL
	virtual bool	ShouldDrawPortals() { return false; }
#endif

private:
	ITexture *m_pRenderTarget;
	C_BaseEntity *m_pTargetEntity;
};

CAperturePhotoView::CAperturePhotoView( CViewRender *pMainView ) : 
	CSimpleWorldView( pMainView ),
	m_pRenderTarget( NULL )
{
	//Log_Msg( LOG_FSTOP, "Photo View: Constructed\n" );
}

//-----------------------------------------------------------------------------
//  Originally a method of CViewRender
//-----------------------------------------------------------------------------
void ViewDrawPhoto( CViewRender* pView, ITexture *pRenderTarget, C_BaseEntity *pTargetEntity )
{
	CRefPtr<CAperturePhotoView> pPhotoView = new CAperturePhotoView( pView );
	int nClearFlags = VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH | VIEW_CLEAR_STENCIL;
	SkyboxVisibility_t nSkyboxVisible = SKYBOX_NOT_VISIBLE;

	CMatRenderContextPtr pRenderContext( materials );
	// pRenderContext->SetStencilEnable( false ); // FIXME: JDW

	//bool bIsDormant = pTargetEntity->IsDormant();
	//pTargetEntity->m_bDormant = false;

	//IClientRenderable *pEntRenderable = pTargetEntity->GetClientRenderable();

	bool bNoDraw = pTargetEntity->IsEffectActive( EF_NODRAW );
	if ( bNoDraw )
	{
		pTargetEntity->RemoveEffects( EF_NODRAW );
	}

	bool bHandle = pTargetEntity->GetRenderHandle() == INVALID_CLIENT_RENDER_HANDLE;
	if ( bHandle )
	{
		ClientLeafSystem()->AddRenderable( pTargetEntity, false, RENDERABLE_IS_OPAQUE, RENDERABLE_MODEL_UNKNOWN_TYPE );
		ClientLeafSystem()->RenderableChanged( pTargetEntity->m_hRender );
		ClientLeafSystem()->PreRender();
	}

	//bool bShouldDraw = pEntRenderable->ShouldDraw();
	//bool bIsVisible = pTargetEntity->IsVisible();

	Assert( pTargetEntity->ShouldDraw() );

	CViewSetup photoview = *(pView->GetViewSetup());
	photoview.width = pRenderTarget->GetActualWidth();
	photoview.height = pRenderTarget->GetActualHeight();
	photoview.x = 0;
	photoview.y = 0;
	//photoview.origin = pCameraEnt->GetAbsOrigin();
	//photoview.angles = pCameraEnt->GetAbsAngles();
	//photoview.fov = pCameraEnt->GetFOV();
	photoview.m_bOrtho = false;
	photoview.m_flAspectRatio = pRenderTarget->GetActualWidth() / pRenderTarget->GetActualHeight();
	//(*this) = photoview;

	SetupCurrentView( photoview.origin, photoview.angles, VIEW_MONITOR );

	Frustum frustum;
	render->Push3DView( /*pRenderContext,*/ photoview, VIEW_CLEAR_DEPTH | VIEW_CLEAR_COLOR, pRenderTarget, (VPlane *)frustum );

	//HACKHACK: need to setup a proper view
	if ( pPhotoView->Setup( pTargetEntity, photoview, &nClearFlags, &nSkyboxVisible, pRenderTarget ) == true )
	{
		pView->AddViewToScene( pPhotoView );
	}

	render->PopView( /*pRenderContext, */ frustum );

	//pTargetEntity->m_bDormant = bIsDormant;

	if ( bHandle )
	{
		ClientLeafSystem()->RemoveRenderable( pTargetEntity->GetRenderHandle() );
	}

	if ( bNoDraw )
	{
		pTargetEntity->AddEffects( EF_NODRAW );
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool CAperturePhotoView::Setup( C_BaseEntity *pTargetEntity, const CViewSetup &view, int *pClearFlags, SkyboxVisibility_t *pSkyboxVisible, ITexture *pRenderTarget )
{
	Assert( pTargetEntity != NULL );
	if ( pTargetEntity == NULL )
		return false;

	IClientRenderable *pEntRenderable = pTargetEntity->GetClientRenderable();

	Assert( pEntRenderable != NULL );
	if ( pEntRenderable == NULL )
		return false;

	m_pTargetEntity = pTargetEntity;

	VisibleFogVolumeInfo_t fogInfo;
	WaterRenderInfo_t waterInfo;

	memset( &fogInfo, 0, sizeof( VisibleFogVolumeInfo_t ) );
	memset( &waterInfo, 0, sizeof( WaterRenderInfo_t ) );
	waterInfo.m_bCheapWater = true;
	BaseClass::Setup( view, *pClearFlags, false, fogInfo, waterInfo, NULL );

	m_pRenderTarget = pRenderTarget;
	return true;
}

ConVar cl_camera_minimal_photos( "cl_camera_minimal_photos", "1", 0, "Draw just the targetted entity when taking a camera photo" );


void AddIClientRenderableToRenderList( IClientRenderable *pRenderable, CClientRenderablesList *pRenderablesList )
{
	CClientRenderablesList::CEntry renderableEntry;
	RenderGroup_t group = ClientLeafSystem()->GenerateRenderListEntry( pRenderable, renderableEntry );
	if ( group == RENDER_GROUP_COUNT )
		return;

	for ( int i = 0; i != pRenderablesList->m_RenderGroupCounts[group]; ++i ) {
		if ( pRenderablesList->m_RenderGroups[group][i].m_pRenderable == pRenderable )
			return; //already in the list
	}

	int iAddIndex = pRenderablesList->m_RenderGroupCounts[group];
	++pRenderablesList->m_RenderGroupCounts[group];
	pRenderablesList->m_RenderGroups[group][iAddIndex] = renderableEntry;
}

void GetAllChildRenderables( C_BaseEntity *pEntity, IClientRenderable **pKeepers, int &iKeepCount, int iKeepArraySize )
{
	IClientRenderable *pThisRenderable = pEntity->GetClientRenderable();

	//avoid duplicates and infinite recursion
	for ( int i = 0; i != iKeepCount; ++i ) {
		if ( pThisRenderable == pKeepers[i] )
			return;
	}

	pKeepers[iKeepCount++] = pThisRenderable;

	if ( pEntity->ParticleProp() ) {
		iKeepCount += pEntity->ParticleProp()->GetAllParticleEffectRenderables( &pKeepers[iKeepCount], iKeepArraySize - iKeepCount );
	}
	if ( pEntity->GetEffectEntity() != NULL ) {
		GetAllChildRenderables( pEntity->GetEffectEntity(), pKeepers, iKeepCount, iKeepArraySize );
	}
	for ( C_BaseEntity *pMoveChild = pEntity->FirstMoveChild(); pMoveChild != NULL; pMoveChild = pMoveChild->NextMovePeer() ) {
		GetAllChildRenderables( pMoveChild, pKeepers, iKeepCount, iKeepArraySize );
	}
}

ConVar cl_blur_test( "cl_blur_test", "0", 0, "Blurs entities that have had their photo taken" );
ConVar cl_photo_disable_model_alpha_writes( "cl_photo_disable_model_alpha_writes", "1", FCVAR_ARCHIVE, "Disallows the target entity in photos from writing to the photo's alpha channel" );
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CAperturePhotoView::Draw()
{
	CMatRenderContextPtr pRenderContext( materials );

	Vector vDiff = m_pTargetEntity->GetRenderOrigin() - origin;
	Vector vMins, vMaxs;
	m_pTargetEntity->GetRenderBounds( vMins, vMaxs );
	float fGoodDist = MAX( (vMaxs - vMins).Length() * (1.5f/2.0f), 20.0f );
	float fLength = vDiff.Length();
	if ( fLength > fGoodDist ) {
		//move the camera closer for a better view
#if 1 //use camera forward as offset direction
		Vector vCameraForward;
		AngleVectors( angles, &vCameraForward );

		origin = m_pTargetEntity->WorldSpaceCenter() - (vCameraForward * fGoodDist);
#else //use existing offset direction, but shorter
		origin = m_pTargetEntity->WorldSpaceCenter() - (vDiff * (fGoodDist / fLength));
#endif
		//Vector vCameraForward;
		//AngleVectors( angles, &vCameraForward );
		//origin += vCameraForward * ((fLength - fGoodDist) * vCameraForward.Dot( vDiff / fLength ));
	}

	render->Push3DView( /*pRenderContext,*/ *this, VIEW_CLEAR_DEPTH | VIEW_CLEAR_COLOR | VIEW_CLEAR_STENCIL, m_pRenderTarget, GetFrustum() );
	SetupCurrentView( origin, angles, VIEW_MONITOR );


	MDLCACHE_CRITICAL_SECTION();

	bool bDrawEverything = !cl_camera_minimal_photos.GetBool();
	//Build the world list for now because I don't want to track down crash bugs and this doesn't happen often.
	BuildWorldRenderLists( true, -1, true, false ); // @MULTICORE (toml 8/9/2006): Portal problem, not sending custom vis down
	if ( !bDrawEverything ) {
		memset( m_pRenderablesList->m_RenderGroupCounts, 0, sizeof( m_pRenderablesList->m_RenderGroupCounts ) );
	}

	BuildRenderableRenderLists( CurrentViewID() );

	IClientRenderable *keepHandles[MAX_EDICTS];
	int iKeepChildren = 0;
	GetAllChildRenderables( m_pTargetEntity, keepHandles, iKeepChildren, MAX_EDICTS );

	//set the target entity as the only entity in the renderables list
	{
		if ( !bDrawEverything )
			memset( m_pRenderablesList->m_RenderGroupCounts, 0, sizeof( m_pRenderablesList->m_RenderGroupCounts ) );

		for ( int i = 0; i != iKeepChildren; ++i ) {
			AddIClientRenderableToRenderList( keepHandles[i], m_pRenderablesList );
		}
	}

	engine->Sound_ExtraUpdate();	// Make sure sound doesn't stutter

	m_DrawFlags = m_pMainView->GetBaseDrawFlags() | DF_RENDER_UNDERWATER | DF_RENDER_ABOVEWATER;	// Don't draw water surface...

	//IMaterial *pPhotoBackground = materials->FindMaterial( "photos/photo_background", TEXTURE_GROUP_CLIENT_EFFECTS, false );
	//pRenderContext->DrawScreenSpaceQuad( pPhotoBackground );

	if ( cl_photo_disable_model_alpha_writes.GetBool() )
		pRenderContext->OverrideAlphaWriteEnable( true, false );

	if ( bDrawEverything )
		DrawWorld( /* pRenderContext, */ 0.0f );

	//DrawOpaqueRenderables( pRenderContext, RENDERABLES_RENDER_PATH_NORMAL, NULL );
	DrawOpaqueRenderables( false );
	if ( bDrawEverything ) {
#if defined( PORTAL )
		//DrawRecursivePortalViews();
#endif
		DrawTranslucentRenderables( false, false );
	} else {
		DrawTranslucentRenderablesNoWorld( false );
	}

	if ( cl_photo_disable_model_alpha_writes.GetBool() )
		pRenderContext->OverrideAlphaWriteEnable( false, false );

	//IMaterial *pPhotoForeground = materials->FindMaterial( "photos/photo_foreground", TEXTURE_GROUP_CLIENT_EFFECTS, false );
	//pRenderContext->DrawScreenSpaceQuad( pPhotoForeground );

	m_DrawFlags = 0;
	render->PopView( /* pRenderContext,*/ GetFrustum() );

	if ( cl_blur_test.GetBool() )
	{
		Log_Msg( LOG_FSTOP, "cl_blur_test: blurring object \"%s\"\n", m_pTargetEntity->GetEntityName() );
		m_pTargetEntity->SetBlurState( true );
	}
}