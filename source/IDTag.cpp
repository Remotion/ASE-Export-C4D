/////////////////////////////////////////////////////////////
// Tag Plugin  IDTag
/////////////////////////////////////////////////////////////
// Date: 08.02.2003
/////////////////////////////////////////////////////////////
// (c) 2002 REMOTION, all rights reserved 
/////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"
#include "TIDTag.h"

#define DI_IDTAG_TAG_ID 1011248

//#################################################################
class IDTagTagData : public TagData
{
	INSTANCEOF(IDTagTagData,TagData)
	public:
		virtual Bool Init(GeListNode *node) override;
		//virtual Bool Draw(PluginTag *tag, BaseObject *op, BaseDraw *bd, BaseDrawHelp *bh);
		virtual EXECUTIONRESULT Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags) override;
		//virtual Bool AddToExecution(PluginTag *tag, PriorityList *list);
		virtual Bool GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags) override;
		//
		static NodeData *Alloc(void) { return NewObj(IDTagTagData); }
};
//#################################################################
Bool IDTagTagData::Init(GeListNode *node)
{
	BaseTag		  *tag  = (BaseTag*)node; if (!tag) return FALSE;
	BaseContainer *data = tag->GetDataInstance();

	data->SetBool(ASEIDT_COLLISION,FALSE);

	return TRUE;
}
//#################################################################
Bool IDTagTagData::GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags)
{
	if (!description->LoadDescription(node->GetType())) return FALSE;

	BaseDocument  *doc  = node->GetDocument();
	BaseTag		  *tag  = (BaseTag*)node;if (!tag) return FALSE;
	BaseContainer *data = tag->GetDataInstance();
	/*
	BaseList2D		*link0= data->GetLink(ASEIDT_MTLID+0,doc);
	//TO DO
		BaseContainer bc2 = GetCustomDataTypeDefault(DTYPE_BASELISTLINK);
		bc2.SetString(DESC_NAME,"PolySel2");
		bc2.SetString(DESC_SHORT_NAME,"PolySel2");
		bc2.SetInt32(DESC_ANIMATE,DESC_ANIMATE_OFF);
		bc2.SetBool(DESC_REMOVEABLE,FALSE);
		if (!description->SetParameter(DescLevel(ASEIDT_MTLID+1,DTYPE_BASELISTLINK,0),bc2,DescLevel(ID_TAGPROPERTIES))) return FALSE;
	*/
	BaseContainer abc; abc.SetString(Tpolygonselection,"Tpolygonselection");
	for (Int32 c=0; c<63; c++)
	{
		if (!data->GetLink(ASEIDT_MTLID+c,doc)) break;

		BaseContainer bc2 = GetCustomDataTypeDefault(DTYPE_BASELISTLINK);
		bc2.SetString(DESC_NAME,"ID "+String::IntToString(c+2));
		bc2.SetString(DESC_SHORT_NAME,"ID "+String::IntToString(c+2));
		bc2.SetContainer(DESC_ACCEPT,abc);
		bc2.SetInt32(DESC_ANIMATE,DESC_ANIMATE_OFF);
		bc2.SetBool(DESC_REMOVEABLE,FALSE);
		if (!description->SetParameter(DescLevel(ASEIDT_MTLID+c+1,DTYPE_BASELISTLINK,0),bc2,DescLevel(ID_TAGPROPERTIES))) return FALSE;
	}

	flags |= DESCFLAGS_DESC_LOADED;
	return SUPER::GetDDescription(node,description,flags);//Necessary
}

//#################################################################
EXECUTIONRESULT IDTagTagData::Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags)
{
	return EXECUTIONRESULT_OK;
}

//#################################################################
Bool RegisterIDTagTag(void)
{
	String name=GeLoadString(IDS_IDTAG); if (!name.Content()) return TRUE;
	return RegisterTagPlugin(DI_IDTAG_TAG_ID,name,TAG_VISIBLE,IDTagTagData::Alloc,"TIDTag",AutoBitmap("IDTag.tif"),0);
}
