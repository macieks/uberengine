#include "Base/Containers/ueList.h"

struct Element : public ueList<Element>::Node
{
	UE_INLINE Element(const char* name) : m_name(name) {}
	const char* m_name;
};

typedef ueList<Element> MyList;

void PrintList(const MyList& l)
{
	ueLogI("List [%d]:", l.Length());
	Element* e = l.Front();
	while (e)
	{
		ueLogI("  %s", e->m_name);
		e = e->Next();
	}
}

void UnitTest_List(ueAllocator* allocator)
{
	Element the("the");
	Element weather("weather");
	Element is("is");
	Element windy("windy");
	Element today("today");

	MyList l;
	Element* e;

	l.PushBack(&the);
	l.PushBack(&weather);
	l.PushBack(&is);
	l.PushBack(&windy);
	l.PushBack(&today);
	PrintList(l);
	UE_ASSERT(l.Length() == 5);

	e = l.Front();
	UE_ASSERT(!ueStrCmp(e->m_name, "the"));
	e = l.Back();
	UE_ASSERT(!ueStrCmp(e->m_name, "today"));

	l.PopBack();
	PrintList(l);
	e = l.Front();
	UE_ASSERT(!ueStrCmp(e->m_name, "the"));
	e = l.Back();
	UE_ASSERT(!ueStrCmp(e->m_name, "windy"));

	l.PushBack(&today);
	e = l.Back();
	UE_ASSERT(!ueStrCmp(e->m_name, "today"));
	l.PopBack();

	l.PopFront();
	PrintList(l);
	e = l.Front();
	UE_ASSERT(!ueStrCmp(e->m_name, "weather"));
	e = l.Back();
	UE_ASSERT(!ueStrCmp(e->m_name, "windy"));

	l.Remove(&is);
	PrintList(l);
	e = l.Front();
	UE_ASSERT(!ueStrCmp(e->m_name, "weather"));
	e = l.Back();
	UE_ASSERT(!ueStrCmp(e->m_name, "windy"));

	l.PopBack();
	l.PopFront();
	e = l.Front();
	UE_ASSERT(!e);
	e = l.Back();
	UE_ASSERT(!e);
}