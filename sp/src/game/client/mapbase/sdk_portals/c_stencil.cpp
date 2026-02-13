// SDK Stencil Portals by Flopgop
// https://github.com/Flopgop/source-sdk-portals

#include "cbase.h"
#include "c_stencil.h"

static StencilTool s_StencilTool;
StencilTool* g_pStencilTool = &s_StencilTool;

void StencilTool::SetupInitialStencilRendering(IMatRenderContext* pRenderContext, int ref)
{
	pRenderContext->SetStencilEnable(true);
	pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_ALWAYS);
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_REPLACE); // pass = replace pixel
	pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP); // fail (not possible with comparison ALWAYS) = keep pixel
	pRenderContext->SetStencilZFailOperation(STENCILOPERATION_KEEP); // fail (not possible with comparison ALWAYS) = keep depth
	pRenderContext->SetStencilTestMask(0xFF);
	pRenderContext->SetStencilWriteMask(0xFF);
	pRenderContext->SetStencilReferenceValue(ref);
}

void StencilTool::SetStencilReferenceValue(IMatRenderContext* pRenderContext, int ref)
{
	pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_EQUAL);
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_INCR);
	pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilZFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilReferenceValue(ref);
}

void StencilTool::ClearDepthBuffer(IMatRenderContext* pRenderContext, int ref) 
{
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilReferenceValue(ref);
	pRenderContext->ClearBuffersObeyStencil(false, true);
}

void StencilTool::RestoreStencilMask(IMatRenderContext* pRenderContext, int ref)
{
	pRenderContext->SetStencilReferenceValue(ref);
	pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_EQUAL);
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_DECR);
	pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilZFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->PerformFullScreenStencilOperation();
}

void StencilTool::DisableInitialStencilRendering(IMatRenderContext* pRenderContext) {
	pRenderContext->SetStencilEnable(false);
	pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_NEVER);
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilZFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilTestMask(0xFF);
	pRenderContext->SetStencilWriteMask(0xFF);
	pRenderContext->SetStencilReferenceValue(0);
}
