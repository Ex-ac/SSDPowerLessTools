//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include "SimpleList.h"

//-----------------------------------------------------------------------------
//  Constants definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Private function proto-type definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Data declaration: Private or Public
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Imported data proto-type without header include
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Imported function proto-type without header include
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Private functions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Public functions
//-----------------------------------------------------------------------------

unsigned int _SimpleList_GetListCountFunc(void *pHead, void *pTail, const SimpleListEntryOperator_t *pEntryOperator);

void *_SimpleList_FindNext(void *pHead, unsigned int count,const SimpleListEntryOperator_t *pEntryOperator)
{
	void *pObject = pHead;
	while (count--)
	{
		pObject = pEntryOperator->getNextFunc(pObject);
	}
	return pObject;
}


typedef void _SimpleListFindPrevious(void *pHead, unsigned count, void *pObject,const SimpleListEntryOperator_t *pEntryOperator)
{

}


typedef void _SimpleListDell(void *pHead, unsigned count,  void *pObject, const SimpleListEntryOperator_t *pEntryOperator)
{

}