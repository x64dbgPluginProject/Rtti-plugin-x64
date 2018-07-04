#pragma once
#include "macros.h"
#include "..\plugin.h"
#include <string>

using namespace std;

struct RTTICompleteObjectLocator;

struct vftable_t 
{
	RTTICompleteObjectLocator* pCompleteObjectLocator;		// offset -4
	//void* vmethod1;										// offset +0

	void Print();
};

inline void vftable_t::Print()
{
	dprintf("    pCompleteObjectLocator: %p\n", pCompleteObjectLocator);
}

/*
	//char* pThis; struct PMD pmd;
	pThis+=pmd.mdisp;
	if (pmd.pdisp!=-1)
	{
		char *vbtable = pThis+pmd.pdisp;
		pThis += *(int*)(vbtable+pmd.vdisp);
	}
*/

struct PMD
{
	int mdisp;  //member displacement
	int pdisp;  //vbtable displacement
	int vdisp;  //displacement inside vbtable

	void Print();
	//duint GetOffset(char* pPmd);
};

inline void PMD::Print() 
{
	dprintf("        mdisp: %d  pdisp: %d  vdisp: %d\n", mdisp, pdisp, vdisp);
}

struct TypeDescriptor {
	duint pVFTable;
	duint spare;
	char skip;						// Skip the period before the mangled name
	char sz_decorated_name[256];

	void Print(int spaces = 2);
};

inline void TypeDescriptor::Print(int spaces)
{
	//dprintf("    pVFTable: %p\n", pVFTable);
	//dprintf("    spare: %p\n", spare);
	dprintf("    sz_decorated_name: %s\n", sz_decorated_name);
}

struct RTTIBaseClassDescriptor
{
	TypeDescriptor* pTypeDescriptor; //type descriptor of the class
	duint numContainedBases; //number of nested classes following in the Base Class Array
	struct PMD where;        //pointer-to-member displacement info
	duint attributes;        //flags, usually 0

	void Print();
};

inline void RTTIBaseClassDescriptor::Print()
{
	dprintf("      pTypeDescriptor: %p\n", pTypeDescriptor);
	dprintf("      numContainedBases: %d\n", numContainedBases);
	dprintf("      where:\n");
	where.Print();
	dprintf("      attributes: %X\n", attributes);
}

struct RTTIClassHierarchyDescriptor
{
	duint signature;								//always zero?
	duint attributes;								//bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
	duint numBaseClasses;							//number of classes in pBaseClassArray
	struct RTTIBaseClassArray* pBaseClassArray;

	void Print();
};

inline void RTTIClassHierarchyDescriptor::Print()
{
	dprintf("    signature: %X\n", signature);
	dprintf("    attributes: %X\n", attributes);

	string inheritanceMessage;
	if (CHECK_BIT(attributes, 0))
		inheritanceMessage += "Multiple Inheritance";
	if (CHECK_BIT(attributes, 1))
		inheritanceMessage += " | Virtual Inheritance";
	if (!inheritanceMessage.empty())
		dprintf("        %s\n", inheritanceMessage.c_str());

	dprintf("    numBaseClasses: %d\n", numBaseClasses);
	dprintf("    pBaseClassArray: %p\n", pBaseClassArray);
}

struct RTTICompleteObjectLocator
{
	duint signature;								//always zero ?
	duint offset;									//offset of this vtable in the complete class
	duint cdOffset;									//constructor displacement offset
	TypeDescriptor* pTypeDescriptor;				//TypeDescriptor of the complete class
	RTTIClassHierarchyDescriptor* pClassDescriptor; //describes inheritance hierarchy

	void Print();
};

inline void RTTICompleteObjectLocator::Print()
{
	dprintf("    signature: %X\n", signature);
	dprintf("    offset: %X\n", offset);
	dprintf("    cdOffset: %X\n", cdOffset);
	dprintf("    pTypeDescriptor: %p\n", pTypeDescriptor);
	dprintf("    pClassDescriptor: %p\n", pClassDescriptor);
}
