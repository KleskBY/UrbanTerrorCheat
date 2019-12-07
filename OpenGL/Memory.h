/*
	AUTHOR : TheFlood
	URL : http://unknowncheats.me/
	DESCRIPTION : A Class containing functions that make the
					manipulation of memory within a proccess easier.
	CREATED : November 13th 2013
	MODIFIED : November 13th 2013
*/

#include <windows.h>
#include "Structs.h"


//Get the value of an address as a POINTER
static DWORD* getPointer(DWORD Address)
{
	DWORD* Assignment = (DWORD*)Address;
	return Assignment;
}
//Check if a pointer exist
static BOOL checkPointer(DWORD* Address)
{
	if (!IsBadReadPtr(Address, sizeof(DWORD))) {
		try {
			DWORD value = *Address;
			return TRUE;
		}
		catch (EXCEPTION_POINTERS) {}
	}
	return FALSE;
}

	//Set the value of an address to an ADDRESS
	static void setValue(DWORD Address, DWORD value) 
	{
		DWORD* pointer = getPointer(Address);
		*pointer = value;
	}
	//Set the value of an address to an INTEGER
	static void setValue(DWORD Address, int value) 
	{
		DWORD* pointer = getPointer(Address);
		*pointer = value;
	}
	//Set the value of an address to a FLOAT
	static void setValue(DWORD Address, float value) 
	{
		float* pointer = (float*)Address;
		*pointer = value;
	}
	//Set the value of an address to a CHAR* (BETA)
	static void setValue(DWORD Address, char* value) 
	{
		DWORD* pointer = getPointer(Address);
		pointer = (DWORD*)value;
	}
	//Set the value of an address to a FLOAT
	static void setValue(DWORD Address, Vector3 value)
	{
		Vector3* pointer = (Vector3*)Address;
		*pointer = value;
	}

	//Get the value of an address as an INTEGER
	static int getValueInt(DWORD Address) 
	{
		int* pointer = (int*)Address;
		return *pointer;
	}
	//Get the value of an address as an FLOAT
	static float getValueFloat(DWORD Address) 
	{
		float* pointer = (float*)Address;
		return *pointer;
	}
	static Vector3 getValueVector(DWORD Address)
	{
		Vector3* pointer = (Vector3*)Address;
		return *pointer;
	}
	//Get the value of an address as a CHAR* (BETA)
	static char* getValueChar(DWORD Address) 
	{
		char* pointer = (char*)Address;
		return pointer;
	}
	//Get the value of an address as an ADDRESS
	static DWORD getValue(DWORD Address)
	{
		if (Address != 0) {
			DWORD_PTR* pointer = getPointer(Address);
			if (checkPointer(pointer)) {
				Address = *pointer;
				if ((Address != 0) && (Address != 0x0)) {
					return Address;
				}
			}
		}
		return 0;
	}
	
