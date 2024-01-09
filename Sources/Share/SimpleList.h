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

#include "Debug.h"

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
// const SimpleListEntryOperator_t *pEntryOperator;

typedef void * (*SimpleListGetNextFunc_t)(void *);
typedef void (*SimpleListSetNextFunc_t)(void *, void *);


// typedef size_t (*SimpleListCountFunc_t)(const struct SimpleList *pList);
// typedef void (*SimpleListPushFunc_t)(struct SimpleList *pList, void *pHead, void *pTail, unsigned int count);
// typedef size_t (*SimpleListPopFunc_t)(struct SimpleList *pList, unsigned int count, void *pHead, void *pTail);


typedef struct SimpleListEntryOperator
{
	SimpleListGetNextFunc_t getNextFunc;
	SimpleListSetNextFunc_t setNextFunc;
#if (SIMPLE_LIST_DEBUG)
	SimpleListIsInvalidIdFunc_t isInvalidIdFunc;
#endif
} SimpleListEntryOperator_t;

typedef struct SimpleList
{
	void *pHead;
	void *pTail;
	unsigned int count;
	const SimpleListEntryOperator_t *pEntryOperator;
} SimpleList_t;

//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------

void SimpleList_Init(SimpleList_t *pList, const SimpleListEntryOperator_t *pEntryOperator);

//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------

inline static size_t _SimpleList_GetListCountFunc(void *pHead, void *pTail, const SimpleListEntryOperator_t *pEntryOperator)
{
	if (pHead == (void *)(cSimpleListInvalid))
	{
		return 0;
	}

	size_t ret = 1;
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

inline static void* _SimpleList_FindPrevious(void *pHead, void *pTail,  unsigned int count, void *pObject, const SimpleListEntryOperator_t *pEntryOperator)
{
	if (pHead == pObject || pHead == (void *)(cSimpleListInvalid) || (pObject == (void *)(cSimpleListInvalid)))
	{
		return (void *)(cSimpleListInvalid);
	}

	void *pNext = pHead;
	count --;
	while (count)
	{
		pHead = pNext;
		pNext = pEntryOperator->getNextFunc(pHead);
		if (pNext == pObject)
		{
			return pHead;
		}
		else if ((pNext == (void *)(cSimpleListInvalid)) || (pNext == pTail))
		{
			return (void *)(cSimpleListInvalid);
		}

		count --;
	};

	return pHead;
}



inline static void *_SimpleList_Dell(void *pHead, void *pTail, unsigned count, void *pObject, const SimpleListEntryOperator_t *pEntryOperator)
{
	void *pPrevious = _SimpleList_FindPrevious(pHead, pTail, count, pObject, pEntryOperator);
	if (pPrevious != (void *)(cSimpleListInvalid))
	{
		pEntryOperator->setNextFunc(pPrevious, pEntryOperator->getNextFunc(pObject));
	}
	return pPrevious;
}


inline static size_t SimpleList_GetListCount(const SimpleList_t *pList, void *pHead, void *pTail)
{
	return _SimpleList_GetListCountFunc(pHead, pTail, pList->pEntryOperator);
}


inline static void *SimpleList_FindNext(const SimpleList_t *pList, unsigned int count)
{
	return _SimpleList_FindNext(pList->pHead, count, pList->pEntryOperator);
}


inline static void *SimpleList_FindPrevious(const SimpleList_t *pList, void *pObject)
{
	return _SimpleList_FindPrevious(pList->pHead, pList->pTail, pList->count, pObject, pList->pEntryOperator);
}


inline static void *SimpleList_Dell(SimpleList_t *pList, void *pObject)
{
	if (pObject == (void *)(cSimpleListInvalid) || pList->count == 0)
	{
		return (void *)(cSimpleListInvalid);
	}
	ASSERT_DEBUG(pList->pHead != (void *)(cSimpleListInvalid) && pList->pTail != (void *)(cSimpleListInvalid));

	if (pList->pHead == pObject)
	{
		pList->pHead = pList->pEntryOperator->getNextFunc(pObject);
		pList->count --;
		return pObject;
	}
	else
	{
		void *pPrevious = _SimpleList_Dell(pList->pHead, pList->pTail, pList->count, pObject, pList->pEntryOperator);
		if (pPrevious != (void *)(cSimpleListInvalid))
		{
			if (pList->pHead == pObject)
			{
				pList->pHead = pList->pEntryOperator->getNextFunc(pObject);
			}
			else if (pList->pTail == pObject)
			{
				pList->pTail = pPrevious;
			}
			pList->count --;
		}
		return pPrevious;
	}
}

inline static unsigned int SimpleList_Count(const SimpleList_t *pList)
{
	return pList->count;
}

inline static void SimpleList_PushToTail(SimpleList_t *pList, void *pHead, void *pTail, unsigned int count)
{
	// DebugPrint("pList->count = %u, count = %u\n", pList->count, count);
	if (pList->count == 0)
	{
		pList->pHead = pHead;
		pList->pTail = pTail;
	}
	else
	{
		pList->pEntryOperator->setNextFunc(pList->pTail, pHead);
		pList->pTail = pTail;
	}
	pList->pEntryOperator->setNextFunc(pTail, (void *)(cSimpleListInvalid));
	pList->count += count;
}

inline static unsigned int SimpleList_PopFromHead(SimpleList_t *pList, unsigned int count, void **pHead)
{
	void *pTail = (void *)cSimpleListInvalid;
	*pHead = (void *)cSimpleListInvalid;
	if (pList->count == 0)
	{
		return 0;
	}

	unsigned int ret = 0;
	*pHead = pList->pHead;

	if (pList->count <= count)
	{
		ret = pList->count;
		pTail = pList->pTail;
		pList->pHead = (void *)(cSimpleListInvalid);
		pList->pTail = (void *)(cSimpleListInvalid);
	}
	else
	{
		ret = count;
		pTail = SimpleList_FindNext(pList, count - 1);
		pList->pHead = _SimpleList_FindNext(pTail, 1, pList->pEntryOperator);
	}

	pList->pEntryOperator->setNextFunc(pTail, (void *)(cSimpleListInvalid));

	pList->count -= ret;
	return ret;
}



#ifdef __cplusplus
}
#endif