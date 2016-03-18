//////////////////////////////////////////////////////////////
// REMOTION T3D : FILTER PLUGINS							//
// Unreal T3D loader and saver 								//
/////////////////////////////////////////////////////////////
// VERSION    : CINEMA 4D R8								//
/////////////////////////////////////////////////////////////
// (c) 2002 Remotion										//	
/////////////////////////////////////////////////////////////
// HELP -> http://unreal.jall.org/tutorials/t3d.html		//
// HELP -> http://sourceforge.net/projects/lib3df			//
/////////////////////////////////////////////////////////////
//	Create -> 26.10.02	 T3D io						
//  TO DD:
/////////////////////////////////////////////////////////////

#include "c4d_file.h"
#include "c4d_memory.h"
#include "c4d_general.h"
#include "c4d_basebitmap.h"
//#include "c4d_symbols.h"
#include "c4d_resource.h"
#include "c4d_gui.h"
#include "c4d_basedocument.h"
#include "c4d_basetag.h"
#include "c4d_filterplugin.h"

#include "C4DPrintPublic.h"

#define T3D_LOADER_ID	1011246	 
#define T3D_SAVER_ID	1011247


//################### T3DLoaderData #########################
class T3DLoaderData : public SceneLoaderData
{
	public:
		virtual Bool Identify(PluginSceneLoader *node, const Filename &name, UChar *probe, Int32 size);
		virtual Int32 Load(PluginSceneLoader *node, const Filename &name, BaseDocument *doc, Int32 filterflags, String *error, BaseThread *bt);
		//virtual Bool Identify(PluginSceneLoader *node, const Filename &name, UChar *probe, Int32 size);
		//virtual Int32 Load(PluginSceneLoader *node, const Filename &name, BaseDocument *doc, Int32 filterflags, String *error, Bool *framescene);
		Bool ReadLine(BaseFile *bf, String *v);
		Bool ReadVector(String s,String &id,Vector v);
		//String ReplaceSpaces(String s);
		//Bool ReadPolygon();

		static NodeData *Alloc(void) { return NewObj(T3DLoaderData); }
};



//################### T3DSaverData #########################
class T3DSaverData : public SceneSaverData
{
	public:
		BaseFile	*file;
		virtual Int32 Save(PluginSceneSaver *node, const Filename &name, BaseDocument *doc, Int32 filterflags);
		Bool WriteLine(const String &v);
		Bool WritePolygon(const Vector &pa, const Vector &pb, const Vector &pc, const Vector &pd, const UVWStruct uv);
		Bool WriteObject(BaseObject *op, const Matrix &mg);

		static NodeData *Alloc(void) { return NewObj(T3DSaverData); }
};

//########### READ LINE DONT MODIFI!!!! ############
Bool T3DLoaderData::ReadLine(BaseFile *bf, String *v)
{
	Char ch,line[1024];
	Int32 i = 0, len = bf->TryReadBytes(&ch, 1);
	if (len == 0) return FALSE; // end of file
	while (len == 1 && ch != '\n' && ch != '\r') 
	{
		line[i++] = ch;
		len = bf->TryReadBytes(&ch, 1);
	}
#ifdef __PC //ONLY PC ?
	if (ch == '\r') 
	{
		len = bf->TryReadBytes(&ch, 1);
		if (len == 1 && ch != '\n') bf->Seek(-1);
	}
#endif
	v->SetCString(line, i);
	return TRUE;
}//Read Line

//################## WriteLine ######################
// critical MAC and WIN differences.
Bool T3DSaverData::WriteLine(const String &v)
{
	Bool ok;
	String s = v+"\r\n";//new line
	Int32 len  = s.GetCStringLen()+1;
	Char *mem = (Char*) GeAlloc(sizeof(Char)*len);
	if (!mem) {
		file->SetError(FILEERROR_MEMORY);
		return FALSE;
	}
	s.GetCString(mem,len);
	ok = file->WriteBytes(mem,len-1);//-1 ???
	DeleteMem(mem);
	return ok;
}//Write Line

Bool T3DLoaderData::ReadVector(String s,String &id,Vector v)
{

	return TRUE;
}
//-----------------------------------------------------------------------------
//############################ Identify ############################
Bool T3DLoaderData::Identify(PluginSceneLoader *node, const Filename &name, UChar *probe, Int32 size)
{
	//UInt32 *p=(UInt32*)probe,v1=p[0];	
	//lMotor(&v1); //MAC ?????
	//return v1==0x42466769;//"Begi" ???
	return TRUE;
}
//############################ Load ############################
Int32 T3DLoaderData::Load(PluginSceneLoader *node, const Filename &name, BaseDocument *doc, Int32 filterflags, String *error, BaseThread *bt)
{
	//print("T3D Load");

	BaseFile	*file = file->Alloc(); if (!file) return IMAGE_NOMEM; //???
	if (!file->Open(name,GE_READ,FILE_NODIALOG,GE_MOTOROLA)) return IMAGE_DISKERROR;

	/*
	String		s = ""; //String
	String		val; // VAlue
	String		ids; // ID
	//if (ReadLine(file,&s)) if (s != "Begin")  goto Error;
	while (ReadLine(file,&s)) 
	{
		if (GetIDVal(ids,val,s)) //ZU LANGSAM WAS??
		{
			print(ids+"|"+val);

		}
	}
	*/

//Error:

	file->Free(file);//???
	return 1;
}

Bool T3DSaverData::WritePolygon(const Vector &pa, const Vector &pb, const Vector &pc, const Vector &pd, const UVWStruct uv)
{
	WriteLine("   Begin Polygon");

	//WriteLine("      Origin   ");
	//WriteLine("      Normal   ");

	//WriteLine("      Pan U="+String::FloatToString(uv.a.x,5,6)+"  V="+String::FloatToString(uv.a.y,5,6));

	WriteLine("      TextureU -00001.000000,+00000.000000,+00000.000000");
	WriteLine("      TextureV +00000.000000,+00000.000000,-00001.000000");

	//WriteLine("      TextureU "+String::FloatToString(uv.x,5,6)+","+String::FloatToString(pa.y,5,6)+","+String::FloatToString(pa.z,5,6));
	//WriteLine("      TextureV "+String::FloatToString(uv.x,5,6)+","+String::FloatToString(pa.y,5,6)+","+String::FloatToString(pa.z,5,6));

	WriteLine("      Vertex   "+String::FloatToString(pa.x,5,6)+","+String::FloatToString(pa.y,5,6)+","+String::FloatToString(pa.z,5,6));
	WriteLine("      Vertex   "+String::FloatToString(pb.x,5,6)+","+String::FloatToString(pb.y,5,6)+","+String::FloatToString(pb.z,5,6));
	WriteLine("      Vertex   "+String::FloatToString(pc.x,5,6)+","+String::FloatToString(pc.y,5,6)+","+String::FloatToString(pc.z,5,6));
	WriteLine("      Vertex   "+String::FloatToString(pd.x,5,6)+","+String::FloatToString(pd.y,5,6)+","+String::FloatToString(pd.z,5,6));

	WriteLine("   End Polygon");

	return TRUE;
}

Bool T3DSaverData::WriteObject(BaseObject *op, const Matrix &mg)
{
	//Matrix mg;
	//while (op)
	if (op)
	{
		//mg=up*op->GetMl();
		if (op->GetType()==Opolygon)
		{
			Vector  *padr  = ToPoly(op)->GetPoint();
			CPolygon *vadr  = ToPoly(op)->GetPolygon();
			Int32    i,vcnt = ToPoly(op)->GetPolygonCount();
			UVWStruct uvw;
			UVWTag *uvwtag = (UVWTag*)op->GetTag(Tuvw);//if (!uvwtag)

			for (i=0; i<vcnt; i++)
			{
				if (uvwtag) uvw = uvwtag->Get(i);
				WritePolygon(padr[vadr[i].a]*mg,padr[vadr[i].b]*mg,padr[vadr[i].c]*mg,padr[vadr[i].d]*mg,uvw);
				//if (vadr[i].c!=vadr[i].d)
			}
		}
		//WriteObjects(stl,op->down(),mg);	
		//op=op->next();
		return TRUE;
	}else return FALSE;
}


//############################ Save ############################
Int32 T3DSaverData::Save(PluginSceneSaver *node, const Filename &name, BaseDocument *doc, Int32 filterflags)
{
	BaseContainer *data = node->GetDataInstance();
	//print("T3D Save");
	BaseObject *op = doc->GetActiveObject(); if (!op) return 0;

	file = file->Alloc(); if (!file) return IMAGE_NOMEM; //???
	if (!file->Open(name,GE_WRITE,FILE_NODIALOG,GE_INTEL)) return file->GetError();
	
	// Get polygon object
    ModelingCommandData md1; md1.op = op; md1.doc = doc;
    if (!SendModelingCommand(MCOMMAND_CURRENTSTATETOOBJECT, md1)){ return FILEERROR_MEMORY;}
  
    //Triangulate it
    ModelingCommandData md2; md2.op = md1.result_ex;  ///do not use result_ex
    if (!SendModelingCommand(MCOMMAND_TRIANGULATE, md2)){ return FILEERROR_MEMORY;}
	op = md2.op;if (!op){ return FILEERROR_MEMORY;} //Read Result

	Matrix	mg = op->GetMg();

	WriteLine("Begin PolyList");
		WriteObject(op,mg);
	WriteLine("End PolyList");

	//file->Close();
	BaseObject::Free(op);//?????
	file->Free(file);//?????????
	return 0;
}
//-----------------------------------------------------------------------------

//###################################################
Bool RegisterT3Dio(void)
{
	//String name=GeLoadString(IDS_STL); if (!name.Content()) return TRUE;
	//if (!RegisterSceneLoaderPlugin(T3D_LOADER_ID,"T3D Load",SCENEFILTER_OBJECTS,t3dld,"t3d")) return FALSE; ;
	if (!RegisterSceneSaverPlugin(T3D_SAVER_ID,"T3D",SCENEFILTER_OBJECTS,T3DSaverData::Alloc,nullptr,"t3d")) return FALSE;

	return TRUE;
}
