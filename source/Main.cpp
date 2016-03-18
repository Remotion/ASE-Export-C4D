/////////////////////////////////////////////////////////////
// CINEMA SDK : MAIN MODULE																 //
/////////////////////////////////////////////////////////////
// VERSION    : CINEMA 4D																	 //
/////////////////////////////////////////////////////////////
// (c) 1989-2002 MAXON Computer GmbH, all rights reserved	 //
/////////////////////////////////////////////////////////////

// Starts the plugin registration

#include "c4d.h"

// forward declarations
Bool RegisterASEio(void);
Bool RegisterT3Dio(void);
Bool RegisterIDTagTag(void);

Bool PluginStart(void)
{

	Int32 err = 0;
	if (!RegisterASEio()) err++;
	if (!RegisterT3Dio()) err++;
	if (!RegisterIDTagTag()) err++;

	if(err==0){
		GePrint("// Remotion 2003-2008: ASE,T3D Export");
		GeDebugOut(" ASE,T3D Export registred! ");
	}else{
		GeDebugOut(" ASE,T3D Export ERROR %i",err);
	}

	return TRUE;
}

void PluginEnd(void)
{
}

Bool PluginMessage(Int32 id, void *data)
{
	//use the following lines to set a plugin priority
	//
	switch (id)
	{
		case C4DPL_INIT_SYS:
			if (!resource.Init()) return FALSE; // don't start plugin without resource
			return TRUE;

		case C4DMSG_PRIORITY: 
			return TRUE;
	}

	return FALSE;
}
