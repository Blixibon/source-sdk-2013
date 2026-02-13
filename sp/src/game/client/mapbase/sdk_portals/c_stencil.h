// SDK Stencil Portals by Flopgop
// https://github.com/Flopgop/source-sdk-portals

#ifndef C_STENCIL_H
#define C_STENCIL_H
#include "materialsystem/imaterialsystem.h"

class StencilTool {
public:
	void SetupInitialStencilRendering(IMatRenderContext* pRenderContext, int ref);
	void SetStencilReferenceValue(IMatRenderContext* pRenderContext, int ref);
	void ClearDepthBuffer(IMatRenderContext* pRenderContext, int ref);
	void RestoreStencilMask(IMatRenderContext* pRenderContext, int ref);
	void DisableInitialStencilRendering(IMatRenderContext* pRenderContext);
};

extern StencilTool* g_pStencilTool;
#endif
