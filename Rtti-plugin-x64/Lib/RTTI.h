/// Based on this article
/// http://www.openrce.org/articles/full_view/23
/// and information from 
/// https://www.blackhat.com/presentations/bh-dc-07/Sabanal_Yason/Paper/bh-dc-07-Sabanal_Yason-WP.pdf
/// https://www.ece.umd.edu/~barua/yoo-APSEC-2014.pdf
#pragma once

#include "..\plugin.h"
#include "RTINFO.h"
#include <string>

using namespace std;

#define MAX_CLASS_NAME 256
#define MAX_BASE_CLASSES 12

class RTTI {
public:
	RTTI(duint addr);

	// This class' name
	string name;

	// vftable
	bool GetVftable();
	duint GetAddressVftable();

	// CompleteObjectLocator
	bool GetCompleteObjectLocator();

	bool GetTypeDescriptor();

	RTTIClassHierarchyDescriptor classHierarchyDescriptor;

	// These are to iterate over the classHierarchyDescriptor Base classes
	RTTIBaseClassDescriptor GetBaseClassDescriptor(size_t n);
	TypeDescriptor GetBaseTypeDescriptor(size_t n);
	string GetBaseClassName(size_t n);

	void PrintVerboseToLog();
	void Print();
	void PrintBaseClasses();
	bool IsValid();

private:
	duint* m_fakeClass = nullptr;
	bool m_isValid = false;		// Is true if RTTI information is present
	
	duint m_this = 0;
	
	vftable_t m_vftable;
	RTTICompleteObjectLocator m_completeObjectLocator;
	TypeDescriptor m_typeDescriptor;

	duint m_classHierarchyDescriptor = 0;
	duint m_pBaseClassArray = 0;

	bool GetRTTI();
	string Demangle(char * sz_name);

	// The classHierarhcyDescriptor contains information for all the base classes of 'this'.  
	// We need to copy the information from the debugger to these
	RTTIBaseClassDescriptor m_baseClassDescriptors[MAX_BASE_CLASSES];
	
	// These refer to the position of the member inside the base class, this is used
	// for multiple, virtual inheritance, this information is parsed from the vbtable if pdisp != -1
	duint m_nBaseClassOffsets = 0;
	duint m_vbtable[MAX_BASE_CLASSES] = { 0 };
	duint m_baseClassOffsets[MAX_BASE_CLASSES] = { 0 };

	TypeDescriptor m_baseClassTypeDescriptors[MAX_BASE_CLASSES];
	string m_baseClassTypeNames[MAX_BASE_CLASSES];
	duint m_BaseClassArray[MAX_BASE_CLASSES] = { 0 };

	// Methods
	duint GetVbtable(size_t idx);
	duint GetBaseClassOffset(size_t idx);
	duint GetBaseClassAddress(size_t idx);
	
	// This is for printing so you can easily see where the base class is from _this_
	// Because these are calculated from the base of the vbtable, it's hard to know automatically where that is
	duint GetBaseClassAddressFromThis(size_t idx);
};

void DumpRttiWindow(int hWindow);