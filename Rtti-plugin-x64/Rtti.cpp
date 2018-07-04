#include "Rtti.h"
#include "plugin.h"
#include "Lib\macros.h"

//template <typename T>
//bool FillInStruct(T* t, duint addr)
//{
//	// A struct like this
//	// struct { void* A, void* B }; 
//	// length would be 2 (A and B)
//	duint length = sizeof(T) / sizeof(void*);
//
//	for (size_t i = 0; i < length; i++)
//	{
//		// Start at offset 0 and increment by the sizeof a pointer
//		duint read = ADDPTR(addr, sizeof(void*) * i, duint);
//		
//		// Read the value and store it
//		duint val;
//
//		if (!DbgMemRead(read, &val, sizeof(void*)))
//			dputs("Couldn't read memory!");
//
//		// Essentially what I want is this   *t[i] = val;
//		// but it gives an "illegal indirection error"
//		// which makes sense since the typename I'm passing in is *vftable_t
//		*t[i] = val;
//	}
//
//	return true;
//}

RTTIClass::RTTIClass(duint addr)
{
	vftable_t vftable;

	FillInStruct<vftable_t>(&vftable, addr);

	// works
	//(*(duint*)&vftable) = 2;

	duint val = 0x33;
	(*(duint*)&vftable) = val;


	dprintf("vmethod1: %p\n", vftable.vmethod1);
	dprintf("vmethod2: %p\n", vftable.vmethod2);
	dprintf("vmethod3: %p\n", vftable.vmethod3);
	// Get the vftable
	//if (!DbgMemRead(addr, &vftable, sizeof(size_t)))
	//	dputs("No RTTI Information found");

	//dprintf("vftable: %p\n", vftable);
}

void DumpRtti(int hWindow)
{
	if (!DbgIsDebugging())
	{
		dputs("You need to be debugging to use this command");
		return;
	}
	SELECTIONDATA sel;
	GuiSelectionGet(hWindow, &sel);
	sel.start = sel.start - (sel.start % (sizeof duint));

	dprintf("Dumping RTTI data for %p\n", sel.start);

	RTTIClass rttiClass(sel.start);
}