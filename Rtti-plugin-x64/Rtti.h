/// http://www.openrce.org/articles/full_view/23
#pragma once
#include "plugin.h"

class RTTIClass {
public:
	RTTIClass(duint addr);
};

struct vftable_t {
	duint vmethod1;
	duint vmethod2;
	duint vmethod3;
};

//struct PMD
//{
//	int mdisp;  //member displacement
//	int pdisp;  //vbtable displacement
//	int vdisp;  //displacement inside vbtable
//};
//
//struct TypeDescriptor {
//	DWORD pVFTable;
//	DWORD spare;
//	char sz_mangled_name;
//};
//
//struct RTTICompleteObjectLocator
//{
//	DWORD signature; //always zero ?
//	DWORD offset;    //offset of this vtable in the complete class
//	DWORD cdOffset;  //constructor displacement offset
//	TypeDescriptor* pTypeDescriptor; //TypeDescriptor of the complete class
//	struct RTTIClassHierarchyDescriptor* pClassDescriptor; //describes inheritance hierarchy
//};
//
//struct RTTIBaseClassDescriptor
//{
//	TypeDescriptor* pTypeDescriptor; //type descriptor of the class
//	DWORD numContainedBases; //number of nested classes following in the Base Class Array
//	struct PMD where;        //pointer-to-member displacement info
//	DWORD attributes;        //flags, usually 0
//};
//
//struct RTTIClassHierarchyDescriptor
//{
//	DWORD signature;      //always zero?
//	DWORD attributes;     //bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
//	DWORD numBaseClasses; //number of classes in pBaseClassArray
//	struct RTTIBaseClassArray* pBaseClassArray;
//};



void DumpRtti(int hWindow);