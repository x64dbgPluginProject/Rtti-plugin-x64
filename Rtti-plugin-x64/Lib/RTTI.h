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

	// Populates the private fields m_...
	bool GetVftable();
	duint GetAddressVftable();
	bool GetCompleteObjectLocator();
	bool GetTypeDescriptor();
	bool GetClassHierarchyDescriptor();	
	bool GetBaseClasses();

	// These are to iterate over the classHierarchyDescriptor Base classes
	RTTIBaseClassDescriptor GetBaseClassDescriptor(size_t n);
	//TypeDescriptor GetBaseTypeDescriptor(size_t n);
	//string GetBaseClassName(size_t n);

	string ToString();
	bool IsValid();

private:
	
	duint m_this = 0;
	bool m_isValid = false;		// Is true if RTTI information is present
		
	vftable_t m_vftable;
	RTTICompleteObjectLocator m_completeObjectLocator;
	TypeDescriptor m_typeDescriptor;
	RTTIClassHierarchyDescriptor m_classHierarchyDescriptor;
	  
	/// Base Class Descriptors for inheritance

	// The classHierarhcyDescriptor contains information for all the base classes of 'this'.  
	RTTIBaseClassDescriptor m_baseClassDescriptors[MAX_BASE_CLASSES];
	TypeDescriptor m_baseClassTypeDescriptors[MAX_BASE_CLASSES];

	// These refer to the position of the member inside the base class.
	// I haven't seen multiple vbtables in a this, but the information in the BaseClassTypeDescriptors
	// Contain potentially different offsets from the vbtable?
	// for multiple, virtual inheritance, this information is parsed from the vbtable if pdisp != -1
	duint m_vbtable[MAX_BASE_CLASSES] = { 0 };
	
	// The offsets that each base class is at within the this class
	duint m_baseClassOffsets[MAX_BASE_CLASSES] = { 0 };

	// Methods

	// These take the index of the class in the m_baseClassArray
	duint GetVbtable(size_t idx);
	duint GetBaseClassOffset(size_t idx);
	
	//duint GetBaseClassAddress(size_t idx);

	// Automatically called at construction
	bool GetRTTI();

	// Demangles a decorated name
	string Demangle(char * sz_name);
	
	// This is for printing so you can easily see where the base class is from _this_
	// Because these are calculated from the base of the vbtable, it's hard to know where the vbtable is
	duint GetBaseClassOffsetFromThis(size_t idx);
};

void DumpRttiWindow(int hWindow);