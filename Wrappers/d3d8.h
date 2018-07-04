#pragma once

#define VISIT_PROCS_D3D8(visit) \
	visit(Direct3D8EnableMaximizedWindowedModeShim, jmpaddrvoid) \
	visit(ValidatePixelShader, jmpaddr) \
	visit(ValidateVertexShader, jmpaddr) \
	visit(DebugSetMute, jmpaddr) \
	visit(Direct3DCreate8, jmpaddr)

#ifdef PROC_CLASS
PROC_CLASS(d3d8, dll, VISIT_PROCS_D3D8)
#endif
