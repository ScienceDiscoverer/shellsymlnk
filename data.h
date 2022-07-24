#pragma once

#define DEF_ROOT HKEY_CURRENT_USER
#define MAX_S 1024

/*___________________________________________________________________
|  regChk:
|    Checks if specified value exists in the app's registry key
|
|  app_key: Registry key to set in relation to root key
|     name: Name of the value to test
|
|  Return value:
|       Value exist -> true
|    No value exist -> false
|____________________________________________________________________*/
bool regChk(LPCWSTR app_key, LPCWSTR name);


/*___________________________________________________________________
|  regGet:
|    Reads DWORD value from the Registry, first check with regChk!
|
|  app_key: Registry key to set in relation to root key
|     name: Name of the value to retrieve
|  
|  Return value:
|       Value exist -> Data stored by the value
|    No value exist -> 0
|____________________________________________________________________*/
DWORD regGet(LPCWSTR app_key, LPCWSTR name);


/*___________________________________________________________________
|  regGet(sz):
|    Reads SZ string value from registry.
|
|  app_key: Registry key to set in relation to root key
|     name: Name of the value to retrieve
|      out: Ptr to buffer ptr to which memory will be allocated
|           Must be freed after use via HeapFree()
|
|  Return value:
|       Number of bytes read
|____________________________________________________________________*/
DWORD regGet(LPCWSTR app_key, LPCWSTR name, LPWSTR *out);


/*___________________________________________________________________
|  regSet:
|    Sets empty value in the Registry
|
|  app_key: Registry key to set in relation to root key
|     name: Name of the value to set
|____________________________________________________________________*/
void regSet(LPCWSTR app_key, LPCWSTR name);


/*___________________________________________________________________
|  regSet(dw):
|    Sets DWORD value in the Registry
|
|  app_key: Registry key to set in relation to root key
|     name: Name of the value to set
|     data: Data to associate with the value
|____________________________________________________________________*/
void regSet(LPCWSTR app_key, LPCWSTR name, DWORD data);

/*___________________________________________________________________
|  regSet(sz):
|    Sets SZ (null-terminated string) value in the Registry
|
|  app_key: Registry key to set in relation to root key
|     name: Name of the value to set
|     data: Null terminated string to associate with the value
|____________________________________________________________________*/
void regSet(LPCWSTR app_key, LPCWSTR name, LPCWSTR data);


/*___________________________________________________________________
|  regDel:
|    Removes value from the Registry
|
|  name: Name of the value to be removed
|____________________________________________________________________*/
void regDel(LPCWSTR app_key, LPCWSTR name);


/*___________________________________________________________________
|  regSetRoot:
|    Changes root registry key for application.
|
|  app_root: New root registry key
|____________________________________________________________________*/
void regSetRoot(HKEY root);