#include "Rtti.h"
#include "..\plugin.h"
#include "macros.h"
#include "MemHelpers.h"
#include "RTINFO.h"
#include <string>
#include <DbgHelp.h>

#pragma comment(lib, "DbgHelp.lib")

using namespace std;

RTTI::RTTI(duint addr)
{
	m_this = addr;

	m_isValid = GetRTTI(m_this);

	if (!m_isValid)
		dprintf("Couldn't parse RTTI data at %p\n", addr);
}

bool RTTI::GetRTTI(duint addr)
{
	if (!DbgMemRead(m_this, &m_vftable, sizeof(duint)))
		return false;

	// Read offset at the vftable -4 is a pointer to a complete object locator
	addr = (duint)SUBPTR(m_vftable, sizeof(duint));

	if (!DbgMemRead(addr, &vftable, sizeof(vftable_t)))
		return false;

	// Read the RTTICompleteObjectLocator
	m_completeObjectLocator = (duint)vftable.pCompleteObjectLocator;
	if (!DbgMemRead(m_completeObjectLocator, &completeObjectLocator, sizeof(RTTICompleteObjectLocator)))
		return false;

	// Read the TypeDescriptor
	m_typeDescriptor = (duint)completeObjectLocator.pTypeDescriptor;
	if (!DbgMemRead(m_typeDescriptor, &typeDescriptor, sizeof(TypeDescriptor)))
		return false;

	// Demangle the name and copy it 
	name = Demangle(typeDescriptor.sz_decorated_name);

	// Read the RTTIClassHierarchyDescriptor
	m_classHierarchyDescriptor = (duint)completeObjectLocator.pClassDescriptor;
	if (!DbgMemRead(m_classHierarchyDescriptor, &classHierarchyDescriptor, sizeof(RTTIClassHierarchyDescriptor)))
		return false;

	// Populate the BaseClassArray
	m_pBaseClassArray = (duint)classHierarchyDescriptor.pBaseClassArray;
	
	// For each of the numBaseClasses populate the BaseClassDescriptors
	for (size_t i = 0; i < classHierarchyDescriptor.numBaseClasses; i++)
	{
		addr = m_pBaseClassArray + (i * sizeof(duint));

		duint pBaseClassDescriptor;
		if (!DbgMemRead(addr, &pBaseClassDescriptor, sizeof(void*)))
			return false;

		// Save the pointer to each of the base class descriptors
		m_BaseClassArray[i] = pBaseClassDescriptor;

		// Read it into the struct
		if (!DbgMemRead(pBaseClassDescriptor, &m_baseClassDescriptors[i], sizeof(RTTIBaseClassDescriptor)))
			return false;

		// Populate the TypeDescriptors for the Base Classes as well
		if (!DbgMemRead((duint)m_baseClassDescriptors[i].pTypeDescriptor, &m_baseClassTypeDescriptors[i], sizeof(TypeDescriptor)))
			return false;

		m_baseClassTypeNames[i] = Demangle(m_baseClassTypeDescriptors[i].sz_decorated_name);
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

	if (n >= classHierarchyDescriptor.numBaseClasses - 1)
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

void RTTI::PrintVerbose()
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

}

void RTTI::PrintBaseClasses()
{
	for (size_t i = 0; i < classHierarchyDescriptor.numBaseClasses; i++)
	{
		auto baseClass = m_baseClassDescriptors[i];
		auto baseClassType = m_baseClassTypeDescriptors[i];
		auto baseClassName = m_baseClassTypeNames[i];

		dprintf("    BaseClassDescriptor[%d]: %p - %s\n", i, m_BaseClassArray[i], baseClassName.c_str());
		baseClass.Print();
		dprintf("\n");
	}
}
