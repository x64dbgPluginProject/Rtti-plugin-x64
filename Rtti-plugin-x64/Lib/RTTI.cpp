#include "Rtti.h"
#include "..\plugin.h"
#include "macros.h"
#include "MemHelpers.h"
#include "RTINFO.h"
#include <string>
#include <DbgHelp.h>

#pragma comment(lib, "DbgHelp.lib")

using namespace std;

duint GetBaseAddress(duint addr)
{
	return DbgFunctions()->ModBaseFromAddr(addr);
}

RTTI::RTTI(duint addr)
{
	m_this = addr;

	m_isValid = GetRTTI();
}

bool RTTI::GetRTTI()
{
	duint addr = m_this;
	duint moduleBase;

	// Read the value at this to m_vftable
	if (!DbgMemRead(addr, &m_vftable, sizeof(duint)))
		return false;

	// Read offset at the vftable -4 is a pointer to a complete object locator
	addr = (duint)SUBPTR(m_vftable, sizeof(duint));

	// Read the entire vftable
	if (!DbgMemRead(addr, &vftable, sizeof(vftable_t)))
		return false;

	// Read the RTTICompleteObjectLocator
	m_completeObjectLocator = (duint)vftable.pCompleteObjectLocator;
	if (!DbgMemRead(m_completeObjectLocator, &completeObjectLocator, sizeof(RTTICompleteObjectLocator)))
		return false;

	moduleBase = GetBaseAddress(m_completeObjectLocator);
	dprintf("moduleBase: %p\n\n", moduleBase);

	dprintf("m_completeObjectLocator: %p\n", m_completeObjectLocator);
	dprintf("signature: %p\n", completeObjectLocator.signature);
	dprintf("offset: %p\n", completeObjectLocator.offset);
	dprintf("cdOffset: %p\n", completeObjectLocator.cdOffset);

#ifdef _WIN64

	// In x64 the CompleteObjectLocator is different, this is encapsulated in the RTTICompleteObjectLocator .x64 field
	// The TypeDescriptor and ClassDescriptors are offsets from the base of the module, so to get their final address 
	// We add module base + (DWORD)offset_typeDescriptor to it.

	// Offset from the base of the module to the typeDescriptor
	duint offset_typeDescriptor = completeObjectLocator.x64_typeDescriptor.offset;
	duint offset_classDescriptor = completeObjectLocator.x64_classHierarchyDescriptor.offset;

	m_typeDescriptor = (duint)ADDPTR(moduleBase, offset_typeDescriptor);
	m_classHierarchyDescriptor = (duint)ADDPTR(moduleBase, offset_classDescriptor);

#else

	m_typeDescriptor = (duint)completeObjectLocator.pTypeDescriptor;
	m_classHierarchyDescriptor = (duint)completeObjectLocator.pClassHierarchyDescriptor;

#endif

	dprintf("m_typeDescriptor: %p\n", m_typeDescriptor);
	dprintf("m_classHierarchyDescriptor: %p\n", m_classHierarchyDescriptor);

	// Read the TypeDescriptor
	if (!DbgMemRead(m_typeDescriptor, &typeDescriptor, sizeof(TypeDescriptor)))
		return false;

	// Demangle the name and copy it 
	name = Demangle(typeDescriptor.sz_decorated_name);

	// Read the RTTIClassHierarchyDescriptor
	if (!DbgMemRead(m_classHierarchyDescriptor, &classHierarchyDescriptor, sizeof(RTTIClassHierarchyDescriptor)))
		return false;

	dprintf("numBaseClasses: %p\n", classHierarchyDescriptor.numBaseClasses);

#ifdef _WIN64
	duint offset_pBaseClassArray = classHierarchyDescriptor.x64.offset_baseClassArray;
	m_pBaseClassArray = (duint)ADDPTR(moduleBase, offset_pBaseClassArray);
#else
	m_pBaseClassArray = (duint)classHierarchyDescriptor.pBaseClassArray;
#endif
	
	dprintf("m_pBaseClassArray: %p\n", m_pBaseClassArray);

	// Populate the BaseClassArray
	// For each of the numBaseClasses populate the BaseClassDescriptors
	for (size_t i = 0; i < classHierarchyDescriptor.numBaseClasses; i++)
	{
		addr = m_pBaseClassArray + (i * sizeof(DWORD));

		dprintf("addr: %p\n", addr);

		duint pBaseClassDescriptor;
		if (!DbgMemRead(addr, &pBaseClassDescriptor, sizeof(void*)))
			return false;

#ifdef _WIN64
		pBaseClassDescriptor = (duint)ADDPTR(moduleBase, pBaseClassDescriptor);
#endif

		dprintf("pBaseClassDescriptor: %p\n", pBaseClassDescriptor);

		// Save the pointer to each of the base class descriptors
		m_BaseClassArray[i] = pBaseClassDescriptor;

		dprintf("m_BaseClassArray[%d]: %p\n", i, m_BaseClassArray[i]);

		// Read it into the struct
		if (!DbgMemRead(pBaseClassDescriptor, &m_baseClassDescriptors[i], sizeof(RTTIBaseClassDescriptor)))
			return false;

		// Populate the TypeDescriptors for the Base Classes as well
		if (!DbgMemRead((duint)m_baseClassDescriptors[i].pTypeDescriptor, &m_baseClassTypeDescriptors[i], sizeof(TypeDescriptor)))
			return false;

		m_baseClassTypeNames[i] = Demangle(m_baseClassTypeDescriptors[i].sz_decorated_name);

		// Assign the vbtable entry
		m_vbtable[i] = 0;
		m_baseClassOffsets[i] = (duint)ADDPTR(m_this, m_baseClassDescriptors[i].where.mdisp);

		if (m_baseClassDescriptors[i].where.pdisp != -1)
		{
			// The docs aren't very clear here.
			// The pdisp field is the offset of the vbtable from this
			// Inside the vbtable we read at vdisp to get the final offset from the vbtable of the class
			m_vbtable[i] = (duint)ADDPTR(m_this, m_baseClassDescriptors[i].where.pdisp);

			duint pdisp = m_baseClassDescriptors[i].where.pdisp;
			duint vdisp = m_baseClassDescriptors[i].where.vdisp;			
			
			duint pMemberOffsets = 0;
			duint memberOffsets[MAX_BASE_CLASSES] = { 0 };

			// Read the value at the vdisp to find where the class is off the vbtable of this class
			//duint vbtable = 0;
			if (!DbgMemRead(m_vbtable[i], &pMemberOffsets, sizeof(pMemberOffsets)))
			{
				dprintf("Problem reading the vbtable.\n");
				continue;
			}

			if (!DbgMemRead(pMemberOffsets, &memberOffsets, sizeof(memberOffsets)))
			{
				dprintf("Problem reading the member offsets.\n");
				continue;
			}

			m_baseClassOffsets[i] = memberOffsets[i];
			m_nBaseClassOffsets++;
		}
	}

	return true;
}

string RTTI::Demangle(char* sz_name)
{
	char tmp[MAX_CLASS_NAME] = { 0 };
	if (UnDecorateSymbolName(sz_name, tmp, MAX_CLASS_NAME, UNDNAME_NAME_ONLY) == 0)
		return false;

	// Remove 'AV' from the name
	char* n = tmp + 2;

	return string(n);
}

RTTIBaseClassDescriptor RTTI::GetBaseClassDescriptor(size_t n)
{
	if (!m_isValid)
		return RTTIBaseClassDescriptor();

	if (n >= classHierarchyDescriptor.numBaseClasses)
	{
		dprintf("can't get index %d because there are only %d base classes!\n", n, classHierarchyDescriptor.numBaseClasses);
		return RTTIBaseClassDescriptor();
	}

	return m_baseClassDescriptors[n];
}

TypeDescriptor RTTI::GetBaseTypeDescriptor(size_t n)
{
	if (n >= MAX_BASE_CLASSES)
		return TypeDescriptor();

	return m_baseClassTypeDescriptors[n];
}

string RTTI::GetBaseClassName(size_t n)
{
	if (n >= MAX_BASE_CLASSES)
		return string();

	return m_baseClassTypeNames[n];
}

duint RTTI::GetVFTable()
{
	return m_vftable;
}

void RTTI::Print()
{
	if (!m_isValid)
		return;

	string result = name.c_str();

	if (classHierarchyDescriptor.numBaseClasses > 1)
	{
		result.append("  :  ");

		// Base Class formatting
		// Appends each base class in this format
		// 'ClassA (+12), ClassB (+1C)'
		for (size_t i = 1; i < classHierarchyDescriptor.numBaseClasses; i++)
		{
			auto baseClass = GetBaseClassDescriptor(i);
			auto baseClassType = m_baseClassTypeDescriptors[i];
			auto baseClassName = m_baseClassTypeNames[i];
			auto baseClassOffset = m_baseClassOffsets[i];

			result.append(baseClassName.c_str() + string(" "));

			// Print offsets
			result.append("(+");

			char hexStr[32] = { 0 };
			sprintf_s(hexStr, sizeof(hexStr), "%zX", GetBaseClassAddressFromThis(i));
			
			result.append(hexStr);
			result.append(")");

			bool isLastClass = i == classHierarchyDescriptor.numBaseClasses - 1;
			if (!isLastClass)
				result.append(",");

			result.append(" ");
		}
	}

	dprintf("%s\n", result.c_str());
}

void RTTI::PrintVerboseToLog()
{
	if (!m_isValid)
		return;

	dprintf("=====================================================================================\n");
	dprintf("RTTI Class Information:\n");
	dprintf("this: %p\n", m_this);
	dprintf("name: %s\n", name.c_str());
	dprintf("\n");
	dprintf("vftable: %p\n", m_vftable);
	vftable.Print();
	dprintf("\n");
	dprintf("CompleteObjectLocator: %p\n", m_completeObjectLocator);
	completeObjectLocator.Print();
	dprintf("\n");
	dprintf("TypeDescriptor: %p\n", m_typeDescriptor);
	typeDescriptor.Print();
	dprintf("\n");
	dprintf("ClassHierarchyDescriptor: %p\n", m_classHierarchyDescriptor);
	classHierarchyDescriptor.Print();	
	dprintf("\n");
	PrintBaseClasses();
	dprintf("=====================================================================================\n");
}

void RTTI::PrintBaseClasses()
{
	for (size_t i = 0; i < classHierarchyDescriptor.numBaseClasses; i++)
	{
		auto baseClass = GetBaseClassDescriptor(i);
		auto baseClassType = m_baseClassTypeDescriptors[i];
		auto baseClassName = m_baseClassTypeNames[i];
		auto baseClassOffset = m_baseClassOffsets[i];

		dprintf("    BaseClassDescriptor[%d]: %p - %s\n", i, m_BaseClassArray[i], baseClassName.c_str());
		baseClass.Print();
		dprintf("    ClassOffset: + 0x%X (%p)\n", GetBaseClassAddressFromThis(i), GetBaseClassAddress(i));
		dprintf("\n");
	}
}

duint RTTI::GetVbtable(size_t idx)
{
	if (idx > (sizeof(m_vbtable) / MAX_BASE_CLASSES))
		return 0;

	return m_vbtable[idx];
}

duint RTTI::GetBaseClassOffset(size_t idx)
{
	if (idx > (sizeof(m_baseClassOffsets) / MAX_BASE_CLASSES))
		return 0;

	return m_baseClassOffsets[idx];
}

duint RTTI::GetBaseClassAddress(size_t idx)
{
	duint offset = GetBaseClassOffset(idx);
	duint vbtable = GetVbtable(idx);
	
	duint addr = (duint)ADDPTR(vbtable, offset);
	return addr;
}

duint RTTI::GetBaseClassAddressFromThis(size_t idx)
{
	duint vbtable = GetVbtable(idx);
	duint offsetToThis = (duint)SUBPTR(vbtable, m_this);

	return (duint)ADDPTR(offsetToThis, GetBaseClassOffset(idx));
}

bool RTTI::IsValid()
{
	return m_isValid;
}
