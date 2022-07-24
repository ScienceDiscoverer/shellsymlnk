#include <windows.h>
#include <winnls.h>
#include <shobjidl.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include "data.h"

HKEY app_root = DEF_ROOT;




/*___________________________________________________________________
|  regKeyCrOp:
|    Attempts to create new registry key, if already exists, opens it
|
|  sub_k: Path to the new key from application root key
|
|  Return value:
|    Handle to created or opened key
|____________________________________________________________________*/
HKEY regKeyCrOp(LPCWSTR sub_k);


/*___________________________________________________________________
|  regKeyOp:
|    Opens already exising key
|
|  sub_k: Path to the key from application root key
|
|  Return value:
|            Key exists -> Handle to the key
|    Key does not exist -> NULL
|____________________________________________________________________*/
HKEY regKeyOp(LPCWSTR sub_k);


/*___________________________________________________________________
|  regKeyOp:
|    Destroys registry key and all it's sub-keys/values
|
|  sub_k: Path to the key from application root key
|____________________________________________________________________*/
void regKeyDel(LPCWSTR sub_k);




bool regChk(LPCWSTR app_key, LPCWSTR name)
{
	HKEY key = regKeyOp(app_key);

	// Get Registry Value ============================================================================
	LSTATUS res = RegGetValue(
		key,							//    [I]  Handle to opened key (KEY_QUERY_VALUE  acc. req.)
		NULL,							//  [I|O]  Name of the registry subkey to be retrieved
		name,							//  [I|O]  Name of the value (if NULL or "" get def. value)
		RRF_RT_ANY,						//  [I|O]  Flags comb. of data types to set filter (+ opt.)
		NULL,							//  [O|O]  Ptr to variable that recieves value type
		NULL,							//  [O|O]  Ptr to var that receive data of the value || NULL
		NULL);							// [IO|O]  Ptr sz. B -> N. of B cop. (NULL only pvData = NULL)
	// ===============================================================================================

	RegCloseKey(key);
	return !res;
}

DWORD regGet(LPCWSTR app_key, LPCWSTR name)
{
	HKEY key = regKeyOp(app_key);
	DWORD dat = 0, sz = sizeof(dat);

	// Get Registry Value ============================================================================
	RegGetValue(
		key,							//    [I]  Handle to opened key (KEY_QUERY_VALUE  acc. req.)
		NULL,							//  [I|O]  Name of the registry subkey to be retrieved
		name,							//  [I|O]  Name of the value (if NULL or "" get def. value)
		RRF_RT_REG_DWORD,				//  [I|O]  Flags comb. of data types to set filter (+ opt.)
		NULL,							//  [O|O]  Ptr to variable that recieves value type
		&dat,							//  [O|O]  Ptr to var that receive data of the value || NULL
		&sz);							// [IO|O]  Ptr sz. B -> N. of B cop. (NULL only pvData = NULL)
	// ===============================================================================================

	RegCloseKey(key);
	return dat;
}

DWORD regGet(LPCWSTR app_key, LPCWSTR name, LPWSTR *out)
{
	HKEY key = regKeyOp(app_key);
	DWORD sz = 0;

	// Get Registry Value Data size ==================================================================
	RegGetValue(
		key,							//    [I]  Handle to opened key (KEY_QUERY_VALUE  acc. req.)
		NULL,							//  [I|O]  Name of the registry subkey to be retrieved
		name,							//  [I|O]  Name of the value (if NULL or "" get def. value)
		RRF_RT_REG_SZ,					//  [I|O]  Flags comb. of data types to set filter (+ opt.)
		NULL,							//  [O|O]  Ptr to variable that recieves value type
		NULL,							//  [O|O]  Ptr to var that receive data of the value || NULL
		&sz);							// [IO|O]  Ptr sz. B -> N. of B cop. (NULL only pvData = NULL)
	// ===============================================================================================

	*out = (LPWSTR)HeapAlloc(GetProcessHeap(), NULL, sz + sizeof(wchar_t));

	// Get Registry Value ============================================================================
	RegGetValue(
		key,							//    [I]  Handle to opened key (KEY_QUERY_VALUE  acc. req.)
		NULL,							//  [I|O]  Name of the registry subkey to be retrieved
		name,							//  [I|O]  Name of the value (if NULL or "" get def. value)
		RRF_RT_REG_SZ,					//  [I|O]  Flags comb. of data types to set filter (+ opt.)
		NULL,							//  [O|O]  Ptr to variable that recieves value type
		*out,							//  [O|O]  Ptr to var that receive data of the value || NULL
		&sz);							// [IO|O]  Ptr sz. B -> N. of B cop. (NULL only pvData = NULL)
	// ===============================================================================================

	RegCloseKey(key);
	return sz;
}

void regSet(LPCWSTR app_key, LPCWSTR name)
{
	HKEY key = regKeyCrOp(app_key);

	// Set Registry Value ============================================================================
	RegSetValueEx(
		key,							//   [I]  Handle to opened key (KEY_SET_VALUE acc. req.)
		name,							// [I|O]  Value name (if not exist, will be created)
		0,								//        Reserved, must be 0
		REG_SZ,							//   [I]  Type of value beeing set
		NULL,							//   [I]  Ptr to the data that will be stored
		NULL);							//   [I]  Sizeof data in bytes (for strings must include \0)
	// ===============================================================================================

	RegCloseKey(key);
}

void regSet(LPCWSTR app_key, LPCWSTR name, DWORD data)
{
	HKEY key = regKeyCrOp(app_key);

	// Set Registry Value ============================================================================
	RegSetValueEx(
		key,							//   [I]  Handle to opened key (KEY_SET_VALUE acc. req.)
		name,							// [I|O]  Value name (if not exist, will be created)
		0,								//        Reserved, must be 0
		REG_DWORD,						//   [I]  Type of value beeing set
		(const BYTE*)&data,				//   [I]  Ptr to the data that will be stored
		sizeof(data));					//   [I]  Sizeof data in bytes (for strings must include \0)
	// ===============================================================================================

	RegCloseKey(key);
}

void regSet(LPCWSTR app_key, LPCWSTR name, LPCWSTR data)
{
	HKEY key = regKeyCrOp(app_key);
	DWORD s = DWORD(wcslen(data)+1) * sizeof(wchar_t);

	// Set Registry Value ============================================================================
	RegSetValueEx(
		key,							//   [I]  Handle to opened key (KEY_SET_VALUE acc. req.)
		name,							// [I|O]  Value name (if not exist, will be created)
		0,								//        Reserved, must be 0
		REG_SZ,							//   [I]  Type of value beeing set
		(const BYTE*)data,				//   [I]  Ptr to the data that will be stored
		s);								//   [I]  Sizeof data in bytes (for strings must include \0)
	// ===============================================================================================

	RegCloseKey(key);
}

void regDel(LPCWSTR app_key, LPCWSTR name)
{
	HKEY key = regKeyOp(app_key);

	// Delete Registry Value =========================================================================
	RegDeleteValue(
		key,							//   [I]  Handle to opened key (KEY_SET_VALUE acc. req.)
		name);							// [I|O]  Value name to be removed
	// ===============================================================================================

	RegCloseKey(key);
}

void regSetRoot(HKEY root)
{
	app_root = root;
}

HKEY regKeyCrOp(LPCWSTR sub_k)
{
	REGSAM sam = KEY_SET_VALUE | KEY_QUERY_VALUE;
	HKEY key = NULL;

	// Create Registry Key ===========================================================================
	RegCreateKeyEx(
		app_root,						//   [I]  Handle to open reg key, or root key
		sub_k,							//   [I]  Subkey name relative to 1st param (32 lvl deep)
		0,								//        Reserved, must be 0
		NULL,							// [I|O]  Class type of this key (OOP shenenigans?)
		REG_OPTION_NON_VOLATILE,		//   [I]  Key options (0 -> std non-volatile key)
		sam,							//   [I]  Security and Access Mask
		NULL,							// [I|O]  Ptr to SECURITY_ATTRIBUTES (handle inheritability)
		&key,							//   [O]  Ptr to HKEY variable that will hold returned handle
		NULL);							// [O|O]  Dis-pos -> CREATED_NEW_KEY | OPENED_EXISTING_KEY
	// ===============================================================================================

	return key;
}

HKEY regKeyOp(LPCWSTR sub_k)
{
	REGSAM sam = KEY_SET_VALUE | KEY_QUERY_VALUE;
	HKEY key = NULL;

	// Open Registry Key =============================================================================
	RegOpenKeyEx(
		app_root,						//   [I]  Handle to open reg key, or root key
		sub_k, 							// [I|O]  Name of the registry subkey to be opened
		0, 								//   [I]  Options -> 0 || REG_OPTION_OPEN_LINK
		sam,							//   [I]  Security and Access Mask
		&key);							//   [O]  Ptr to variable that receives opened key handle
	// ===============================================================================================

	return key;
}

void regKeyDel(LPCWSTR sub_k)
{
	// Delete Registry Key ===========================================================================
	RegDeleteKeyEx(
		HKEY_CLASSES_ROOT,				// [I]  Handle to open reg key, or root key
		sub_k,							// [I]  Subkey name to be deleted, relative to 1st param
		KEY_WOW64_64KEY, 				// [I]  Key platform -> KEY_WOW64_32KEY | KEY_WOW64_64KEY
		0);								//      Reserved, must be 0
	// ===============================================================================================
}