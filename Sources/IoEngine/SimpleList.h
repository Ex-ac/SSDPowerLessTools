#pragma once


#ifdef __cplusplus
extern "C" {
#endif


//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------

#define cSimpleListInvalid SIZE_MAX

//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------
struct SimpleList;
const SimpleListEntryOperator_t *pEntryOperator;

typedef void * (*SimpleListGetNextFunc_t)(void *);
typedef void (*SimpleListSetNextFunc_t)(void *, void *);


typedef unsigned int (*SimpleListCountFunc_t)(const struct SimpleList *pList);
typedef void (*SimpleListPushFunc_t)(struct SimpleList *pList, void *pHead, void *pTail, unsigned int count);
typedef uint32_t (*SimpleListPopFunc_t)(struct SimpleList *pList, unsigned int count, uint32_t *pHead, uint32_t *pTail);


typedef struct SimpleListEntryOperator
{
	SimpleListGetNextFunc_t getNextFunc;
	SimpleListSetNextFunc_t setNextFunc;
#if (SIMPLE_LIST_DEBUG)
	SimpleListIsInvalidIdFunc_t isInvalidIdFunc;
#endif
} SimpleListEntryOperator_t;

typedef struct SimpleListOperator
{
	SimpleListGetListCountFunc_t getListCountFunc;
	SimpleListFindNextFunc_t findNextFunc;
	SimpleListFindPreviousFunc_t findFunc;
	SimpleListDellFunc_t dellFunc;
	SimpleListCountFunc_t countFunc;
	SimpleListPushFunc_t pushFunc;
	SimpleListPopFunc_t popFunc;
} SimpleListOperator_t;



typedef struct SimpleList
{
	void *pHead;
	void *pTail;
	unsigned int count;
	const SimpleListEntryOperator_t *pEntryOperator;
	const SimpleListOperator_t *pOperator;
} SimpleList_t;

//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------

void SimpleList_Init(SimpleList_t *pList, const SimpleListEntryOperator_t *pEntryOperator, const SimpleListOperator_t *pOperator);

unsigned int _SimpleList_GetListCountFunc(void *pHead, void *pTail, const SimpleListEntryOperator_t *pEntryOperator);
void *_SimpleList_FindNext(void *pHead, unsigned int count,const SimpleListEntryOperator_t *pEntryOperator);
typedef void _SimpleListFindPrevious(void *pHead, unsigned count, void *pObject,const SimpleListEntryOperator_t *pEntryOperator);
typedef void _SimpleListDell(void *pHead, unsigned count,  void *pObject, const SimpleListEntryOperator_t *pEntryOperator);


//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------

inline static unsigned int _SimpleList_GetListCountFunc(void *pHead, void *pTail, const SimpleListEntryOperator_t *pEntryOperator)
{
	if (pHead == (void *)(cSimpleListInvalid))
	{
		return 0;
	}

	unsigned int ret = 1;
	while (pHead != pTail)
	{
		pHead = pEntryOperator->getNextFunc(pHead);
		ret++;
		if (pHead == (void *)(cSimpleListInvalid) && ((void *)(cSimpleListInvalid) != pTail))
		{
			return cSimpleListInvalid;
		}
	}

	return ret;
}

inline static void *_SimpleList_FindNext(void *pHead, unsigned int count, const SimpleListEntryOperator_t *pEntryOperator)
{
	void *pObject = pHead;
	while (count--)
	{
		pObject = pEntryOperator->getNextFunc(pObject);
	}
	return pObject;
}

inline static void* _SimpleList_FindPrevious(void *pHead, unsigned count, void *pObject,const SimpleListEntryOperator_t *pEntryOperator)
{
	if (pHead == pObject || pHead == (void *)(cSimpleListInvalid) || (pObject == (void *)(cSimpleListInvalid)))
	{
		return (void *)(cSimpleListInvalid);
	}

	void *pNext = pHead;
	count --;
	do
	{
		pHead = pNext;
		pNext = pEntryOperator->getNextFunc(pHead);
		if (pNext == pObject)
		{
			return pHead;
		}
		else if (pNext == (void *)(cSimpleListInvalid))
		{
			return (void *)(cSimpleListInvalid);
		}

		count --;
	} while (count);

	return pHead;
}



inline static void *_SimpleList_Dell(void *pHead, unsigned count, void *pObject, const SimpleListEntryOperator_t *pEntryOperator)
{
	void *pPrevious = _SimpleList_FindPrevious(pHead, count, pObject, pEntryOperator);
	if (pPrevious != (void *)(cSimpleListInvalid))
	{
		pEntryOperator->setNextFunc(pPrevious, pEntryOperator->getNextFunc(pObject));
	}
	return pPrevious;
}


inline static unsigned int SimpleList_GetListCount(const SimpleList_t *pList, void *pHead, void *pTail)
{
	return _SimpleList_GetListCountFunc(pHead, pTail, pList->pEntryOperator);
}


inline static void *SimpleList_FindNext(const SimpleList_t *pList, void *pHead, unsigned int count)
{
	return _SimpleList_FindNext(pHead, count, pList->pEntryOperator);
}


inline static void *SimpleList_FindPrevious(const SimpleList_t *pList, void *pHead, unsigned int count, void *pObject)
{
	return _SimpleList_FindPrevious(pHead, count, pObject, pList->pEntryOperator);
}


inline static void *SimpleList_Dell(SimpleList_t *pList, void *pHead, unsigned int count, void *pObject)
{
	return _SimpleList_Dell(pHead, count, pObject, pList->pEntryOperator);
}

inline static unsigned int SimpleList_Count(const SimpleList_t *pList)
{
	return pList->count;
}

inline static void SimpleList_PushToTail(SimpleList_t *pList, void *pHead, void *pTail, unsigned int count)
{
	if (pList->count = 0)
	{
		pList->pHead = pHead;
		pList->pTail = pTail;
	}
	else
	{
		pList->pEntryOperator->setNextFunc(pList->pTail, pHead);
		pList->pTail = pTail;
	}
	pList->count += count;
}

inline static unsigned int SimpleList_PopFromHead(SimpleList_t *pList, unsigned int count, void **pHead)
{
	*pHead = (void *)cSimpleListInvalid;
	if (pList->count == 0)
	{
		return 0;
	}

	unsigned int ret = 0;
	if (pList->count <= count)
	{
		ret = pList->count;
		pList->pHead = (void *)(cSimpleListInvalid);
		pList->pTail = (void *)(cSimpleListInvalid);
	}
	else
	{
		ret = count;
		pList->pHead = _SimpleList_FindNext(pList->pHead, count, pList->pEntryOperator);
	}

	pList->count -= ret;
	return ret;
}



#ifdef __cplusplus
}
#endif