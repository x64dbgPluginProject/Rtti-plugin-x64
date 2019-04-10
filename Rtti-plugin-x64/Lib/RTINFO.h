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

struct PMD
{
	// A vbtable (virtual base class table) is generated for multiple virtual inheritance.
	// Because it is sometimes necessary to upclass (casting to base classes), the exact location of
	// the base class needs to be determined.

	int mdisp;  //member displacement, vftable offset (if PMD.pdisp is -1)
	int pdisp;  //vbtable displacement, vbtable offset (-1: vftable is at displacement PMD.mdisp inside the class)
	int vdisp;  //displacement inside vbtable, displacement of the base class vftable pointer inside the vbtable

	void Print();
};

inline void PMD::Print() 
{
	dprintf("        mdisp: %d  pdisp: %d  vdisp: %d\n", mdisp, pdisp, vdisp);
}

struct TypeDescriptor {
	duint pVFTable;					// Always points to the type_info descriptor
	duint spare;
	char skip;						// Skip the period before the mangled name
	char sz_decorated_name[256];

	void Print();
};

inline void TypeDescriptor::Print()
{
	dprintf("    pVFTable: %p\n", pVFTable);
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

// DWORD always
struct RTTIClassHierarchyDescriptor
{
	duint signature;								//always zero?
	duint attributes;								//bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
	duint numBaseClasses;							//number of classes in pBaseClassArray
	struct RTTIBaseClassArray* pBaseClassArray;		// Index 0 of this array is always 'this' class first

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
	DWORD signature;								//always zero ?  (x64 always 1?)
	DWORD offset;									//offset of this vtable in the complete class
	DWORD cdOffset;									//constructor displacement offset
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
