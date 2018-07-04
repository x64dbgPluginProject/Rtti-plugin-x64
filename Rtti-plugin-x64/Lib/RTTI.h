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

	vftable_t vftable;
	RTTICompleteObjectLocator completeObjectLocator;
	TypeDescriptor typeDescriptor;
	RTTIClassHierarchyDescriptor classHierarchyDescriptor;

	// These are to iterate over the classHierarchyDescriptor Base classes
	RTTIBaseClassDescriptor GetBaseClassDescriptor(size_t n);
	TypeDescriptor GetBaseTypeDescriptor(size_t n);
	string GetBaseClassName(size_t n);
	
	duint m_vftable = 0;

	void PrintVerbose();
	void PrintBaseClasses();
	bool IsValid();

private:
	bool m_isValid = false;		// Is true if RTTI information is present

	duint m_this = 0;
	duint m_completeObjectLocator = 0;
	duint m_typeDescriptor = 0;
	duint m_classHierarchyDescriptor = 0;
	duint m_pBaseClassArray = 0;

	bool GetRTTI(duint addr);
	string Demangle(char * sz_name);

	// The classHierarhcyDescriptor contains information for all the base classes of 'this'.  
	// We need to copy the information from the debugger to these
	RTTIBaseClassDescriptor m_baseClassDescriptors[MAX_BASE_CLASSES];
	TypeDescriptor m_baseClassTypeDescriptors[MAX_BASE_CLASSES];
	string m_baseClassTypeNames[MAX_BASE_CLASSES];
	duint m_BaseClassArray[MAX_BASE_CLASSES] = { 0 };
};

void DumpRttiWindow(int hWindow);