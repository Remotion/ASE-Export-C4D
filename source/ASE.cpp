//////////////////////////////////////////////////////////////
// REMOTION ASE : FILTER PLUGINS							//
// ASE loader and saver 								//
/////////////////////////////////////////////////////////////
// VERSION    : CINEMA 4D R8								//
/////////////////////////////////////////////////////////////
// (c) 2002 Remotion										//	
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
#include "c4d_baseselect.h"
#include "Ttexture.h"

#include "C4DPrintPublic.h"
#include "aseexport.h"

#define ASE_LOADER_ID	1011244
#define ASE_SAVER_ID	1011245

//-------------------------------
#include "TIDTag.h"
#define DI_IDTAG_TAG_ID 1011248
//-------------------------------

//-----------------------------------------
struct ASEEdata{
	ASEEdata(void);
	Bool ReadSetting(BaseContainer *data);

	Bool savenormals;
	Bool reversenormals;
	Bool matObjecPrefix;
	Bool joinob;

	Int32	commadigits;
};
ASEEdata::ASEEdata(void)
{
	savenormals = FALSE;
	reversenormals = TRUE;
	matObjecPrefix = TRUE;
	joinob	= FALSE;

	commadigits = 6;
}
Bool ASEEdata::ReadSetting(BaseContainer *data)
{
	savenormals = data->GetBool(ASEE_SAVENORMALS);
	reversenormals	= data->GetBool(ASEE_REVERSENORMALS);
	matObjecPrefix	= data->GetBool(ASEE_MATOBPREFIX);
	joinob = data->GetBool(ASEE_JOINOBJECTS);

	commadigits = data->GetInt32(ASEE_COMMADIGITS);
	return TRUE;
}
//-----------------------------------------

//################### T3DLoaderData #########################
class ASELoaderData : public SceneLoaderData
{
	private:

	public:
		virtual Bool Identify(PluginSceneLoader *node, const Filename &name, UChar *probe, Int32 size);
		virtual Int32 Load(PluginSceneLoader *node, const Filename &name, BaseDocument *doc, Int32 filterflags, String *error, BaseThread *bt);

		Bool ReadLine(BaseFile *bf, String *v);
		Bool ReadVector(String s,String &id,Vector v);
		//String ReplaceSpaces(String s);
		//Bool ReadPolygon();

		static NodeData *Alloc(void) { return NewObj(ASELoaderData); }
};
//################### T3DSaverData #########################
class ASESaverData : public SceneSaverData
{
	private:	
		Char		*mem;
		BaseFile	*file;
		Int32 CountTriangles(CPolygon *vadr, Int32 vcnt, Int32 &quadcnt);
		Int32 CountObjects(PolygonObject *sorc, Int32 cnt);
		Int32 GetMatIDCount(BaseContainer  *idtagdata, BaseDocument *doc);
		Bool GetSelList(BaseContainer  *idtagdata, BaseDocument *doc, BaseSelect **sel);
		Int32 GetID(Int32 i);
		Bool WriteLine(const String &v);
		Bool WriteObject(BaseDocument *doc, PolygonObject *sorc, Int32 &materialNumber, ASEEdata &aseedata);
		Bool WriteMaterialList(BaseDocument *doc, PolygonObject *sorc, ASEEdata &aseedata);
		Bool WriteMaterials(BaseDocument *doc, PolygonObject *sorc, Int32 &materialNumber, ASEEdata &aseedata);
		Bool WriteScene(BaseDocument *doc);
	public:
		virtual Bool Init(GeListNode *node);
		virtual void Free(GeListNode *node);
		virtual Int32 Save(PluginSceneSaver *node, const Filename &name, BaseDocument *doc, Int32 filterflags);

		static NodeData *Alloc(void) { return NewObj(ASESaverData); }
};
//########### READ LINE DONT MODIFI!!!! ############
inline Bool ASELoaderData::ReadLine(BaseFile *bf, String *v)
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

Bool ASESaverData::Init(GeListNode *node)
{
	BaseContainer	*data = ((PluginSceneSaver*)node)->GetDataInstance();
	data->SetBool(ASEE_SAVENORMALS,FALSE);
	data->SetBool(ASEE_REVERSENORMALS,TRUE);
	data->SetBool(ASEE_MATOBPREFIX,TRUE);
	data->SetBool(ASEE_JOINOBJECTS,FALSE);

	data->SetInt32(ASEE_COMMADIGITS,6);

	mem = (Char*) GeAlloc(sizeof(Char)*2048);//2048 ???
	if (!mem) return FALSE;

	return TRUE;
}
void ASESaverData::Free(GeListNode *node)
{
	DeleteMem(mem);
}
//################## WriteLine ######################
// critical MAC and WIN differences.
inline Bool ASESaverData::WriteLine(const String &v)
{
	Bool ok;
	String s = v+"\r\n";//new line
	Int32 len  = s.GetCStringLen()+1; if (len>2048) return FALSE;
	s.GetCString(mem,len);
	ok = file->WriteBytes(mem,len-1);
	return ok;
}//Write Line

//-----------------------------------------------------------------------------
//############################ Identify ############################
Bool ASELoaderData::Identify(PluginSceneLoader *node, const Filename &name, UChar *probe, Int32 size)
{
	//UInt32 *p=(UInt32*)probe,v1=p[0];	
	//lMotor(&v1); //MAC ?????
	//return v1==0x42466769;//"Begi" ???
	return TRUE;
}
//############################ Load ############################
Int32 ASELoaderData::Load(PluginSceneLoader *node, const Filename &name, BaseDocument *doc, Int32 filterflags, String *error, BaseThread *bt)
{
	BaseFile	*file = BaseFile::Alloc(); if (!file) return IMAGE_NOMEM; //???
	if (!file->Open(name,GE_READ,FILE_NODIALOG,GE_MOTOROLA)) return IMAGE_DISKERROR;
	BaseFile::Free(file);//???
	return 1;
}

//#########################################################
Int32 ASESaverData::CountTriangles(CPolygon *vadr, Int32 vcnt, Int32 &quadcnt)//WORK
{	
	Int32 tricnt = 0;
	Int32 i;
	for (i=0; i<vcnt; i++)
	{	
		tricnt++;
		if (vadr[i].c!=vadr[i].d) {
			quadcnt++;
			tricnt++;
		}
	}
	return tricnt;
}
//#########################################################
PluginTag *GetPlugTag(BaseObject *ob,Int32 id)//OK
{
	PluginTag *tag;
	Int32 tn = 0;
	//tag = (PluginTag*)ob->GetFirstTag();if (tag) print(tag->GetNodeID());//TEST
	do{
		tag = (PluginTag*)ob->GetTag(Tplugin,tn);
		if (!tag) break;
		tn++;
	}while(tag->GetNodeID()!=id); //GetNodeID ???

return tag;
}
//#########################################################
Int32 ASESaverData::GetMatIDCount(BaseContainer  *idtagdata, BaseDocument *doc)//WORK
{
	if (!idtagdata) return 1;
	Int32	matcnt = 1;

	for (Int32 c=0; c<64; c++)
	{
		if (!idtagdata->GetLink(ASEIDT_MTLID+c,doc,Tpolygonselection)) break;
		matcnt++;
	}
	return matcnt % 64;
}
//#########################################################
Bool ASESaverData::GetSelList(BaseContainer  *idtagdata, BaseDocument *doc, BaseSelect **sel)
{
	if (!idtagdata) return FALSE;
	SelectionTag  *seltag = nullptr;
	for (Int32 c=0; c<64; c++)
	{
		if (idtagdata->GetLink(ASEIDT_MTLID+c,doc))
		{
			seltag = (SelectionTag*)idtagdata->GetLink(ASEIDT_MTLID+c,doc,Tpolygonselection);
			if (seltag) {
				sel[c] = seltag->GetBaseSelect();
			}
		}else{
			sel[c] = nullptr;
		}
	}

	return TRUE;
}
//#########################################################
Bool ASESaverData::WriteObject(BaseDocument *doc, PolygonObject *sorc, Int32 &materialNumber, ASEEdata &aseedata)
{
	if (sorc && sorc->GetType()==Opolygon)
	{
		String		opname = sorc->GetName();

		

		Int32	vc = 1;
 		Int32	nc = aseedata.commadigits;
		Matrix	mg = sorc->GetMg();//?????????	
			
		Vector		*padr	= sorc->GetPoint();	//if (!padr) return FALSE;
		Int32		pcnt	= sorc->GetPointCount();
		CPolygon   *vadr	= sorc->GetPolygon(); //if (!vadr) return FALSE;
		Int32		vcnt	= sorc->GetPolygonCount();

		if (padr && vadr){

			//ID TAG ?????
			PluginTag		*idtag = GetPlugTag(sorc,DI_IDTAG_TAG_ID);
			BaseContainer   *idtagdata = nullptr; 
			Int32			matcnt = 1;
			if (idtag) {
				idtagdata = idtag->GetDataInstance();
				matcnt = GetMatIDCount(idtagdata,doc);//TEST
				if (idtagdata->GetBool(ASEIDT_COLLISION)) opname = "MCDCX_"+opname;
			}	
			BaseSelect		*sel[64];
			GetSelList(idtagdata,doc,sel);

			print("ASE Export Obj: "+opname+" ",materialNumber);

			Int32		num		= 0;
			Int32		vn		= 0;

			Int32		quadcnt = 0;
			Int32		tricnt	= CountTriangles(vadr,vcnt,quadcnt);
			Vector		pos;

			UVWStruct	uvw;
			Vector		u;
			Int32		i,c;

			Int32		matid;
			Int32		smoid   = 1;//TO DO ???
			Int32		curtime = 0;//TO DO ???

			ObjectColorProperties colorProp;//Color 
			sorc->GetColorProperties(&colorProp);//Color 
			UVWTag *uvwtag = (UVWTag*)sorc->GetTag(Tuvw);//if (!uvwtag)

			
			WriteLine("*GEOMOBJECT {");
			
			WriteLine("	*NODE_NAME \""+opname+"\"");
			WriteLine("	*NODE_TM {");

			WriteLine("		*NODE_NAME \""+opname+"\"");//jedes ding (node) hat einen namen
			//WriteLine("		*INHERIT_POS   "+String::FloatToString(mg.off.x)+"   "+String::FloatToString(mg.off.y)+"   "+String::FloatToString(mg.off.z));//m.off
			//WriteLine("		*INHERIT_ROT 0 0 0"); //!!!!
			//WriteLine("		*INHERIT_SCL 0 0 0"); //!!!!
			//??? ROW0,1,2,3 ???  diese 4 zeilen geben die 3x4 tranformation matrix an 
			WriteLine("		*TM_ROW	   "+String::FloatToString(mg.v1.x)+"   "+String::FloatToString(mg.v1.y)+"   "+String::FloatToString(mg.v1.z));//m.off
			WriteLine("		*TM_ROW1   "+String::FloatToString(mg.v2.x)+"   "+String::FloatToString(mg.v2.y)+"   "+String::FloatToString(mg.v2.z));//m.off
			WriteLine("		*TM_ROW2   "+String::FloatToString(mg.v3.x)+"   "+String::FloatToString(mg.v3.y)+"   "+String::FloatToString(mg.v3.z));//m.off
			WriteLine("		*TM_ROW3   "+String::FloatToString(mg.off.x)+"   "+String::FloatToString(mg.off.y)+"   "+String::FloatToString(mg.off.z));//m.off
			
			WriteLine("		*TM_POS   "+String::FloatToString(mg.off.x)+"   "+String::FloatToString(mg.off.y)+"   "+String::FloatToString(mg.off.z));//m.off //position des objekts
			
			//WriteLine("		*TM_ROTAXIS 0.000000	0.000000	0.000000"); //rotations quaternion 
			//WriteLine("		*TM_ROTANGLE 0.000000"); //rotations quaternion 
			//WriteLine("		*TM_SCALE 1.000000	1.000000	1.000000"); //vergrösserungs faktor, für jede achse separat
			//WriteLine("		*TM_SCALEAXIS 0.000000	0.000000	0.000000"); //diese zwei zeile sind mir ein bisschen unklar. wahrscheinlich kann man durch schärung eine roation wiedergeben, und die sind die nummern um wieviel geschärt wird.
			//WriteLine("		*TM_SCALEAXISANG 0.000000"); //????!!!!
			
			WriteLine("	}");

			
			WriteLine("	*MESH {");
			WriteLine("		*TIMEVALUE "+String::IntToString(curtime));
			WriteLine("		*MESH_NUMVERTEX "+String::IntToString(pcnt));
			WriteLine("		*MESH_NUMFACES "+String::IntToString(tricnt));//TriCnt

			
			//VERTEXES
			WriteLine("		*MESH_VERTEX_LIST {");
			for (i=0; i<pcnt; i++){
				pos = padr[i]*mg;
				WriteLine("			*MESH_VERTEX    "+String::IntToString(i)+
					"   "+String::FloatToString(pos.x,vc,nc)+
					"   "+String::FloatToString(pos.z,vc,nc)+//TEST z ???
					"   "+String::FloatToString(pos.y,vc,nc));//TESt y ???	
			}
			WriteLine("		}");

			
			//POLYGONS
			WriteLine("		*MESH_FACE_LIST {");
			num = 0;
			for (i=0; i<vcnt; i++)
			{
				matid = 0;
				if (idtag){ 
					for (c=0; c<matcnt; c++) if (sel[c] && sel[c]->IsSelected(i)) {matid = c+1; break;}
				}

				WriteLine("			*MESH_FACE "+String::IntToString(num++)+
				   ":    A: "+String::IntToString(vadr[i].a)+
					"    B: "+String::IntToString(vadr[i].b)+
					"    C: "+String::IntToString(vadr[i].c)+
					"    AB: 1    BC: 1    CA: 0    *MESH_SMOOTHING "+String::IntToString(smoid)+
					"	*MESH_MTLID "+String::IntToString(matid));// //????!!!!
				
				if (vadr[i].c!=vadr[i].d){
					WriteLine("			*MESH_FACE "+String::IntToString(num++)+
					":    A: "+String::IntToString(vadr[i].a)+
						"    B: "+String::IntToString(vadr[i].c)+
						"    C: "+String::IntToString(vadr[i].d)+
						"    AB: 1    BC: 1    CA: 0    *MESH_SMOOTHING "+String::IntToString(smoid)+
						"	*MESH_MTLID "+String::IntToString(matid));// //????!!!!
				}
			}
			WriteLine("		}");
			
			
			//VERTEX NORMAL DONT WORK ????
			if (aseedata.savenormals){
				WriteLine("		*MESH_NORMALS {");
				num = 0;
				Vector	norm;
				
				for (i=0; i<vcnt; i++)
				{
					norm = !((padr[vadr[i].b]-padr[vadr[i].a])%(padr[vadr[i].c]-padr[vadr[i].a]));//Normal
					WriteLine("			*MESH_FACENORMAL "+String::IntToString(num++)+
						"   "+String::FloatToString(norm.x,vc,nc)+
						"   "+String::FloatToString(norm.z,vc,nc)+
						"   "+String::FloatToString(norm.y,vc,nc));
					//WriteLine("				*MESH_VERTEXNORMAL 0 0.0 0.0 0.0");//TEST
				
					if (vadr[i].c!=vadr[i].d){
						norm = !((padr[vadr[i].c]-padr[vadr[i].a])%(padr[vadr[i].d]-padr[vadr[i].a]));//Normal
						WriteLine("			*MESH_FACENORMAL "+String::IntToString(num++)+
							"   "+String::FloatToString(norm.x,vc,nc)+
							"   "+String::FloatToString(norm.z,vc,nc)+
							"   "+String::FloatToString(norm.y,vc,nc));
						//WriteLine("				*MESH_VERTEXNORMAL 0 0.0 0.0 0.0");//TEST
					}
				}
				WriteLine("		}");
			}


			//UVW Coordinate
			WriteLine("		*MESH_NUMTVERTEX "+String::IntToString(vcnt*3+quadcnt));//UVW //????!!!!
			WriteLine("		*MESH_TVERTLIST {");
			num = 0;
			for (i=0; i<vcnt; i++)
			{		
					if (uvwtag) uvw = uvwtag->Get(i);  //????!!!!
					WriteLine("			*MESH_TVERT    "+String::IntToString(num++)+
						"   "+String::FloatToString(uvw.a.x,vc,nc)+
						"   "+String::FloatToString(uvw.a.y,vc,nc)+
						"   "+String::FloatToString(uvw.a.z,vc,nc));
					WriteLine("			*MESH_TVERT    "+String::IntToString(num++)+
						"   "+String::FloatToString(uvw.b.x,vc,nc)+
						"   "+String::FloatToString(uvw.b.y,vc,nc)+
						"   "+String::FloatToString(uvw.b.z,vc,nc));
					WriteLine("			*MESH_TVERT    "+String::IntToString(num++)+
						"   "+String::FloatToString(uvw.c.x,vc,nc)+
						"   "+String::FloatToString(uvw.c.y,vc,nc)+
						"   "+String::FloatToString(uvw.c.z,vc,nc));

					if (vadr[i].c!=vadr[i].d){
						WriteLine("			*MESH_TVERT    "+String::IntToString(num++)+
							"   "+String::FloatToString(uvw.d.x,vc,nc)+
							"   "+String::FloatToString(uvw.d.y,vc,nc)+
							"   "+String::FloatToString(uvw.d.z,vc,nc));
					}
			}
			WriteLine("		}");

			//UVW Polygone
			WriteLine("		*MESH_NUMTVFACES "+String::IntToString(tricnt));//TriCnt
			WriteLine("		*MESH_TFACELIST {");
			num = 0;
			vn = 0;
			for (i=0; i<vcnt; i++)
			{
				WriteLine("			*MESH_TFACE    "+String::IntToString(num++)+
					"   "+String::IntToString(vn  )+
					"   "+String::IntToString(vn+1)+
					"   "+String::IntToString(vn+2));
				if (vadr[i].c!=vadr[i].d){
					WriteLine("			*MESH_TFACE    "+String::IntToString(num++)+
						"   "+String::IntToString(vn  )+
						"   "+String::IntToString(vn+2)+
						"   "+String::IntToString(vn+3));
					vn+=4;//Quads
				}else{
					vn+=3;//Triagles
				}
			}

			WriteLine("		}");


			//   "*MESH {"
			WriteLine("	}");
			WriteLine("	*PROP_MOTIONBLUR 0"); //????!!!!
			WriteLine("	*PROP_CASTSHADOW 1"); //????!!!!
			WriteLine("	*PROP_RECVSHADOW 1"); //????!!!!
			WriteLine("	*MATERIAL_REF "+String::IntToString(materialNumber)); //!!!!
			WriteLine("	*WIREFRAME_COLOR   "					
					+String::FloatToString(colorProp.color.x)+"   "
					+String::FloatToString(colorProp.color.y)+"   "
					+String::FloatToString(colorProp.color.z));
			WriteLine("}");
			
			
		}
		materialNumber++;
	}

	//RECURSE
	for (sorc=(PolygonObject*)sorc->GetDown(); sorc; sorc=(PolygonObject*)sorc->GetNext()){
		if (!WriteObject(doc,sorc,materialNumber,aseedata)) break;
	}

	return TRUE;
}
//#########################################################
Bool ASESaverData::WriteScene(BaseDocument *doc)
{
	BaseTime	mintime = doc->GetMinTime();
	BaseTime	maxtime = doc->GetMaxTime();
	Filename 	docname	= doc->GetDocumentName();

	Int32	fps	  = doc->GetFps();
	Int32	start = mintime.GetFrame(fps);
	Int32	end   = maxtime.GetFrame(fps);

	WriteLine("*SCENE {");
	WriteLine("	*SCENE_FILENAME \""+docname.GetString()+"\"");
	WriteLine("	*SCENE_FIRSTFRAME "+String::IntToString(start));
	WriteLine("	*SCENE_LASTFRAME "+String::IntToString(end));
	WriteLine("	*SCENE_FRAMESPEED "+String::IntToString(fps));
	WriteLine("	*SCENE_TICKSPERFRAME 160");
	WriteLine("	*SCENE_BACKGROUND_STATIC 0.0 0.0 0.0");
	WriteLine("	*SCENE_AMBIENT_STATIC 0.0 0.0 0.0");
	WriteLine("}");

	return TRUE;
}
Int32 ASESaverData::CountObjects(PolygonObject *sorc, Int32 cnt)
{
	for (sorc=(PolygonObject*)sorc->GetDown(); sorc; sorc=(PolygonObject*)sorc->GetNext()){
		if (sorc->GetType()==Opolygon) cnt++; //Count only Polygon Objects
		cnt = CountObjects(sorc,cnt);
	}
	return cnt;
}

//###################################################################
Bool ASESaverData::WriteMaterialList(BaseDocument *doc, PolygonObject *sorc, ASEEdata &aseedata)
{
	Int32	materialCount = CountObjects(sorc,1); //??????
	Int32	matNumber = 0;

	WriteLine("*MATERIAL_LIST {");
	WriteLine("		*MATERIAL_COUNT "+String::IntToString(materialCount));

	WriteMaterials(doc,sorc,matNumber,aseedata);

	WriteLine("}"); //MATERIAL_LIST
	return TRUE;
}

//###########################################
Bool ASESaverData::WriteMaterials(BaseDocument *doc, PolygonObject *sorc, Int32 &materialNumber, ASEEdata &aseedata)
{
	//print("Mat: "+sorc->GetName()+" ",materialNumber);

	if (sorc && sorc->GetType()==Opolygon){

		Int32		c;
		String		name;
		String		obname = sorc->GetName() + " ";

		//ID TAG
		PluginTag		*idtag = GetPlugTag(sorc,DI_IDTAG_TAG_ID);
		BaseContainer   *idtagdata = nullptr; if (idtag) idtagdata = idtag->GetDataInstance();
		Int32			 matcnt = GetMatIDCount(idtagdata,doc);//TEST
		SelectionTag	*seltag = nullptr;

		WriteLine("		*MATERIAL " + String::IntToString(materialNumber) + " {");
		WriteLine("			*MATERIAL_NAME \"" + String::IntToString(materialNumber) + " " + obname +" - Material\"");
		WriteLine("			*MATERIAL_CLASS \"Multi/Sub-Object\"");
		WriteLine("			*MATERIAL_AMBIENT 0.5 0.5 0.5");
		WriteLine("			*MATERIAL_DIFFUSE 0.5 0.5 0.5");
		WriteLine("			*MATERIAL_SPECULAR 0.9 0.9 0.9");
		WriteLine("			*MATERIAL_SHINE 0.2");
		WriteLine("			*MATERIAL_SHINESTRENGTH 0.0");
		WriteLine("			*MATERIAL_TRANSPARENCY 0.0");
		WriteLine("			*MATERIAL_WIRESIZE 1.0");
		//WriteLine("			*MATERIAL_SHADING Blinn");
		//-------------------------------------------
		WriteLine("			*NUMSUBMTLS "+String::IntToString(matcnt));

		for (c=0; c<matcnt; c++)
		{
			name = obname + "Default";
			if (idtagdata) seltag = (SelectionTag*)idtagdata->GetLink(ASEIDT_MTLID+c-1,doc,Tpolygonselection);
			if (aseedata.matObjecPrefix){
				if (seltag) name = obname + seltag->GetName();
			}else{
				if (seltag) name = seltag->GetName();
			}
			WriteLine("			*SUBMATERIAL "+String::IntToString(c)+" {");
			WriteLine("				*MATERIAL_NAME \""+name+"\"");//???
			WriteLine("				*MATERIAL_CLASS \"Standard\"");
			WriteLine("				*MAP_DIFFUSE {");
			WriteLine("					*MAP_NAME \"Map #"+String::IntToString(c+1)+"\"");
			WriteLine("					*MAP_CLASS \"Bitmap\"");
			WriteLine("					*MAP_SUBNO 1");
			WriteLine("					*MAP_AMOUNT 1.0");
			WriteLine("					*BITMAP \"C:\\default"+String::IntToString(c)+".bmp\"");
			WriteLine("					*UVW_U_OFFSET 0.0");
			WriteLine("					*UVW_V_OFFSET 0.0");
			WriteLine("					*UVW_U_TILING 1.0");
			WriteLine("					*UVW_V_TILING 1.0");
			WriteLine("				}");
			WriteLine("			}");
		}
		//-------------------------------------------
		WriteLine("		}");//MATERIAL
		materialNumber++;
	}

	//RECURSE
	for (sorc=(PolygonObject*)sorc->GetDown(); sorc; sorc=(PolygonObject*)sorc->GetNext()){
		if (!WriteMaterials(doc,sorc,materialNumber,aseedata)) break;
	}

	return TRUE;
}
//############################ Save ############################
Int32 ASESaverData::Save(PluginSceneSaver *node, const Filename &name, BaseDocument *doc, Int32 filterflags)
{
	if (!(filterflags&SCENEFILTER_OBJECTS)) return FILEERROR_NONE;
	
	Int32	materialNumber = 0;//???

	//TO DO TEST FREE OBJECT ????

	//Read Seting
	ASEEdata		aseedata;
	aseedata.ReadSetting(node->GetDataInstance());


	BaseObject		*urop = doc->GetActiveObject(); 
	if (!urop){ print("ASE Export: Select any Object"); return FILEERROR_NONE;}
	
	PolygonObject	*nuop = nullptr;
	//Polygonize
	if (urop->GetType()!=Opolygon){
		// Get polygon object
		ModelingCommandData md1; md1.op = urop; md1.doc = doc; //Create OBJECT (md1)
		if (!SendModelingCommand(MCOMMAND_CURRENTSTATETOOBJECT, md1)){ return FILEERROR_UNKNOWN_VALUE;}

		if (aseedata.joinob){
			// Join
			ModelingCommandData md2; md2.op = md1.result_ex; //Create OBJECT (md2)
			if (!SendModelingCommand(MCOMMAND_JOIN, md2)){ return FILEERROR_UNKNOWN_VALUE;}
			BaseObject::Free(md1.result_ex);//FREE OBJECT  (md1)
			nuop = ToPoly(md2.result_ex);
		}else{
			nuop = ToPoly(md1.result_ex);
		}
	}else{
		if (aseedata.joinob){
			// Join
			ModelingCommandData md2; md2.op = urop;  //Create OBJECT (md2)
			if (!SendModelingCommand(MCOMMAND_JOIN, md2)){ return FILEERROR_UNKNOWN_VALUE;}
			nuop = ToPoly(md2.result_ex);
		}else{
			nuop = ToPoly(urop->GetClone(0,nullptr));
		}
	}
	if (!nuop){ return FILEERROR_UNKNOWN_VALUE;}

	PolygonObject	*op = nullptr;
		//ReverseNormals
		if (aseedata.reversenormals){
			ModelingCommandData md3; md3.op = nuop; 
			if (!SendModelingCommand(MCOMMAND_REVERSENORMALS, md3)){ return FILEERROR_UNKNOWN_VALUE;}
			op = ToPoly(md3.op);//Read Result
		}else{
			op = nuop; //Read Result
		}
	if (!op){ return FILEERROR_UNKNOWN_VALUE;}

	file = BaseFile::Alloc(); if (!file) return FILEERROR_MEMORY; //???
	if (!file->Open(name,GE_WRITE,FILE_NODIALOG,GE_INTEL)) return file->GetError();
	
	//-------------------------------------------------------------
	Int32 year = 0,month = 0,day = 0,hour = 0,minute = 0,second = 0; 
	GeGetSysTime(&year,&month,&day,&hour,&minute,&second);
	
	WriteLine("*3DSMAX_ASCIIEXPORT	200");
	WriteLine("*COMMENT \"AsciiExport Version  0,31 - Remotion 2002 "+String::IntToString(day)+"."+String::IntToString(month)+"."+String::IntToString(year)+" \"");
	WriteScene(doc);

	WriteMaterialList(doc,op,aseedata); //?????
	WriteObject(doc,op,materialNumber,aseedata);//Triangulated Polygon Object RECURSE ???

	//ENDE -------------------------------
	PolygonObject::Free(op); //FREE OBJECT  (op,md2)
	BaseFile::Free(file);
	return FILEERROR_NONE;
}
//-----------------------------------------------------------------------------

//###################################################
Bool RegisterASEio(void)
{
	//String name=GeLoadString(IDS_STL); if (!name.Content()) return TRUE;
	//if (!RegisterSceneLoaderPlugin(ASE_LOADER_ID,"ASE Load",SCENEFILTER_OBJECTS,aseld,"ase")) return FALSE; 
	if (!RegisterSceneSaverPlugin(ASE_SAVER_ID,"ASE [Remotion]",SCENEFILTER_OBJECTS,ASESaverData::Alloc,"aseexport","ase")) return FALSE;

	return TRUE;
}
