#include <windows.h>
#include "data.h"
#include "conmsgbox.h"











#define NINJA
#ifndef NINJA
#include <iostream>
#include <string>
//using namespace std;

#define pause system("pause");
#define P(x) (x)

std::wstring err2s(LSTATUS res)
{
	wchar_t buff[300];

	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		(DWORD)res,
		0,
		buff,
		300,
		NULL);

	return std::wstring(L"[E:") + std::to_wstring(res) + L"] " + std::wstring(buff);
}

void perr()
{
	P(std::wcout << err2s(GetLastError()) << std::endl);
}

#else
#define pause
#define P(x)
#endif

















#define MAX_SUB_DIR 3 // Maximum subdirectory name length (46 656 folders total)

const wchar_t *arg = L"%1";
const wchar_t *arg_dir_back = L"%V";
const wchar_t *shell = L"*\\shell\\makepathsymlink";
const wchar_t *shell_cmd = L"*\\shell\\makepathsymlink\\command";
const wchar_t *shell_dir = L"Directory\\shell\\makepathsymlink";
const wchar_t *shell_dir_cmd = L"Directory\\shell\\makepathsymlink\\command";
const wchar_t *shell_dir_back = L"Directory\\Background\\shell\\makepathsymlink";
const wchar_t *shell_dir_back_cmd = L"Directory\\Background\\shell\\makepathsymlink\\command";
const wchar_t *path_var = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
const wchar_t *store_val = L"SlinkStorePath";
const wchar_t *ignore_val = L"IgnoreLimit1024";

wchar_t inpt[MAX_S];
wchar_t path[MAX_S];

// Gets executable path and quotes it to put in registry, ico uses , instead of space
void quoteExePath(LPCWSTR args, LPWSTR out, bool ico = false);
// Selects appropriate subfolder to save symb. links to, creates new if needed and adds PATH
// Also this functions remove broken symlinks, if found
bool linkPathDir(LPCWSTR src_dir, LPWSTR path);
bool linkPathFile(LPCWSTR src_file, LPWSTR path);
// Modifies base directory path to appropriate subdirectory path. 'dir' -> base link directory
bool nextSubDir(LPWSTR dir);

// HINSTANCE -> "handle" to "instance" aka "module".
// It's NOT a handle. And not to "instance" or "module".
// I's all 16 bit Windows legacy backwards compatability nonsense.
// Since 16-bit Windows had a common address space, the GetInstanceData function was really
// nothing more than a hmemcpy, and many programs relied on this and just used raw hmemcpy
// instead of using the documented API.
// In Win32/Win64 it's actually executable (DLL or EXE) image.
// HINSTANCE points to actual virtual adress where first byte of
// executable's code is located: cout << (const char*)hinst ---> MZ? (? = 0x90/0x00)
int WINAPI wWinMain(
	_In_		HINSTANCE hinst,	// "Handle" to "instance"
	_In_opt_	HINSTANCE phinst,	// "Handle" to "previous instance", always NULL
	_In_		PWSTR cmd,			// Command line arguments
	_In_		int show)			// Default user preference for ShowWindow()
{
	
	//wchar_t cmd[] = L"C:\\ScienceDiscoverer\\PROGRAMZ\\Inkscape\\bin";





#ifndef NINJA
	AllocConsole();
	FILE *s = freopen("CONIN$", "r", stdin);
	s = freopen("CONOUT$", "w", stdout);
	s = freopen("CONOUT$", "w", stderr);
#endif

	if(cmd[0]) // Filename passed
	{
		SIZE_T cl = wcslen(cmd);

		regSetRoot(HKEY_CLASSES_ROOT);
		LPWSTR lnk_path = NULL;
		SIZE_T pl = regGet(shell, store_val, &lnk_path)/sizeof(wchar_t) - 1; // -1 bc. \0

		if(pl > MAX_S)
		{
			return 1;
		}

		wcscpy(path, lnk_path);
		HeapFree(GetProcessHeap(), NULL, lnk_path);
		DWORD flags = 0;

		// Handle full directory symbolic linking
		if(GetFileAttributes(cmd) & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(!linkPathDir(cmd, path))
			{
				return 1;
			}
			
			SIZE_T sl = wcslen(cmd);
			static HANDLE heap = GetProcessHeap();
			LPWSTR dir_all = (LPWSTR)HeapAlloc(heap, NULL, (sl+3) * sizeof(wchar_t));
			wcscpy(dir_all, cmd);
			dir_all[sl] = L'\\';
			dir_all[sl+1] = L'*';
			dir_all[sl+2] = 0;

			SIZE_T npl = wcslen(path);
			path[npl] = L'\\';

			WIN32_FIND_DATA fd;
			HANDLE f = FindFirstFile(dir_all, &fd);
			do
			{
				LPCWSTR fn = fd.cFileName;
				if(fn[0] == L'.' && fn[1] == L'.' && fn[2] == 0 ||
					fn[0] == L'.' && fn[1] == 0)
				{
					continue;
				}

				DWORD flags = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ?
					SYMBOLIC_LINK_FLAG_DIRECTORY : 0;

				wcscpy(path+npl+1, fn);

				P(std::wcout << "pathafter: " << path << std::endl);

				// Generate new source file name
				SIZE_T fnl = wcslen(fn);
				LPWSTR full_fn = (LPWSTR)HeapAlloc(heap, NULL, (sl+fnl+2) * sizeof(wchar_t));
				wcscpy(full_fn, cmd);
				full_fn[sl] = L'\\';
				wcscpy(full_fn+sl+1, fn);

				CreateSymbolicLink(path, full_fn, flags); // Finally...

				HeapFree(heap, NULL, full_fn);
			}
			while(FindNextFile(f, &fd));
			FindClose(f);
			HeapFree(heap, NULL, dir_all);

			pause

			return 0;
		}

		// Handle individual file symbolic linking
		if(!linkPathFile(cmd, path))
		{
			return 1;
		}

		LPCWSTR lslash = cmd+cl; // Null wchar aka. Path End
		while(*(--lslash) != L'\\' && lslash >= cmd)
		{
			;
		}

		P(std::wcout << "pathbef: " << path << std::endl);

		pl = wcslen(path);
		path[pl] = L'\\';
		wcscpy(path+pl+1, lslash+1);

		P(std::wcout << "pathaft: " << path << std::endl);

		CreateSymbolicLink(path, cmd, 0);

		pause

		return 0;
	}

#ifdef NINJA
	AllocConsole();
#endif
	SetConsoleTitle(L"Shell Symbolic Link for PATH Config");
	DWORD cr = 0, cw = 0;
	HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
retry:
	WriteConsole(oh,
		L"Input full path where symlinks shall be stored.\n"
		L"Recommended path is the shortest one, for example C:\\l\n",
		103, &cw, NULL);

	CONSOLE_READCONSOLE_CONTROL rc;
	rc.nLength = sizeof(CONSOLE_READCONSOLE_CONTROL);
	rc.nInitialChars = 0;
	rc.dwCtrlWakeupMask = 0x2000; // Same as -> 0x1 << '\r'
	rc.dwControlKeyState = 0;

	ReadConsole(GetStdHandle(STD_INPUT_HANDLE), inpt, MAX_S, &cr, &rc);
	inpt[cr-1] = 0;

	if(cr >= MAX_S)
	{
		WriteConsole(oh, L"\nPath is longer than 1024 characters.\n", 38, &cw, NULL);
		goto retry;
	}

	CreateDirectory(inpt, NULL);
	DWORD err = GetLastError();
	if(err == ERROR_PATH_NOT_FOUND)
	{
		WriteConsole(oh, L"\nPath is incorrect. Can't create intermidiate dirs.\n", 52, &cw, NULL);
		goto retry;
	}
	else if(err != ERROR_ALREADY_EXISTS && err != ERROR_SUCCESS)
	{
		WriteConsole(oh, L"\nFailed to create dir.\n", 23, &cw, NULL);
		goto retry;
	}

	SetFileAttributes(inpt, FILE_ATTRIBUTE_HIDDEN);

	quoteExePath(arg, path);
	regSetRoot(HKEY_CLASSES_ROOT);
	regSet(shell, L"", L"Файл у symlink до PATH");
	regSet(shell_dir, L"", L"Папку у symlink до PATH");
	regSet(shell_dir_back, L"", L"Папку у symlink до PATH");
	regSet(shell, L"Extended");
	regSet(shell_dir, L"Extended");
	regSet(shell_dir_back, L"Extended");
	regSet(shell, store_val, inpt);
	regSet(shell_cmd, L"", path);
	regSet(shell_dir_cmd, L"", path);
	quoteExePath(arg_dir_back, path);
	regSet(shell_dir_back_cmd, L"", path);
	quoteExePath(NULL, path);
	regSet(shell, L"Icon", path);
	regSet(shell_dir, L"Icon", path);
	regSet(shell_dir_back, L"Icon", path);

	WriteConsole(oh,
		L"\nBase folder created. Context menu items added.\n"
		L"New PATH will be set after creation of the first links.\n",
		104, &cw, NULL);

	pause

	return 0;
}

void quoteExePath(LPCWSTR args, LPWSTR out, bool ico)
{
	out[0] = L'\"';
	out[MAX_S-1] = L'\0';
	GetModuleFileName(NULL, out+1, MAX_PATH);
	DWORD s = (DWORD)wcslen(out)+1;

	out[s-1] = L'\"';
	if(args != NULL)
	{
		out[s] = ico ? L',' : L' ';
		wcscpy(out+s+1, args);
	}
	else
	{
		out[s] = 0;
	}
}

bool linkPathDir(LPCWSTR src_dir, LPWSTR path)
{
	bool ret_val = true;
	
	SIZE_T sl = wcslen(src_dir);
	static HANDLE heap = GetProcessHeap();
	LPWSTR dir_all = (LPWSTR)HeapAlloc(heap, NULL, (sl+3) * sizeof(wchar_t));
	wcscpy(dir_all, src_dir);
	dir_all[sl] = L'\\';
	dir_all[sl+1] = L'*';
	dir_all[sl+2] = 0;

	SIZE_T pl = wcslen(path);

	static LPCWSTR msg0 =
		L"Filename collisions detected in a link folder |";
	static SIZE_T msg0l = wcslen(msg0);
	static LPCWSTR msg1 = L"|.\nPress one of the following buttons:\n"
		L"ABORT to cancel the operation\n"
		L"RETRY to try next folder\n"
		L"IGNORE to ignore this files and create\n"
		L"symlinks in this folder anyway:\n\n";
	static SIZE_T msg1l = wcslen(msg1);

	LPWSTR fn_colls = NULL;
	
	while(ret_val = nextSubDir(path))
	{
		SIZE_T npl = wcslen(path);
		path[npl] = L'\\';

		wchar_t sdir_n[MAX_SUB_DIR+1]; // For the MessageBox
		LPWSTR path_base = path+pl; // Slash before subdir
		SIZE_T sdnl = 0;
		while(*(++path_base) != L'\\')
		{
			sdir_n[sdnl++] = *path_base;
		}
		sdir_n[sdnl] = 0;

		WIN32_FIND_DATA fd;
		HANDLE f = FindFirstFile(dir_all, &fd);
		do
		{
			LPCWSTR fn = fd.cFileName;
			if(fn[0] == L'.' && fn[1] == L'.' && fn[2] == 0 ||
				fn[0] == L'.' && fn[1] == 0)
			{
				continue;
			}

			wcscpy(path+npl+1, fn);

			WIN32_FIND_DATA fdlnk;
			HANDLE flnk = FindFirstFile(path, &fdlnk);

			if(flnk != INVALID_HANDLE_VALUE) // File Name collision
			{
				if(fn_colls == NULL)
				{
					SIZE_T s = msg0l + sdnl + msg1l + wcslen(fn);
					fn_colls = (LPWSTR)HeapAlloc(heap, NULL, (s+2) * sizeof(wchar_t));
					wcscpy(fn_colls, msg0);
					wcscpy(fn_colls+msg0l, sdir_n);
					wcscpy(fn_colls+msg0l+sdnl, msg1);
					wcscpy(fn_colls+msg0l+sdnl+msg1l, fn);
					fn_colls[s] = '\n';
					fn_colls[s+1] = 0;
				}
				else
				{
					SIZE_T fcs = wcslen(fn_colls);
					SIZE_T s = fcs + wcslen(fn);
					LPWSTR tmp = (LPWSTR)HeapAlloc(heap, NULL, (s+2) * sizeof(wchar_t));
					wcscpy(tmp, fn_colls);
					wcscpy(tmp+fcs, fn);
					tmp[s] = '\n';
					tmp[s+1] = 0;
					HeapFree(heap, NULL, fn_colls);
					fn_colls = tmp;
				}
			}
		}
		while(FindNextFile(f, &fd));
		FindClose(f);

		if(fn_colls != NULL)
		{
			int res = consoleMessageBox(fn_colls, L"File Name Collision(s) detected");
			if(res == IDABORT)
			{
				ret_val = false;
				break;
			}
			else if(res == IDIGNORE)
			{
				path[npl] = 0; // Erase filename from path
				break;
			}
			else // IDRETRY
			{
				HeapFree(heap, NULL, fn_colls);
				fn_colls = NULL;
			}
		}
		else
		{
			path[npl] = 0;
			break;
		}

		path[pl] = 0; // Restore original base dir path
	}

	HeapFree(heap, NULL, dir_all);
	HeapFree(heap, NULL, fn_colls);
	return ret_val;
}

bool linkPathFile(LPCWSTR src_file, LPWSTR path)
{
	bool ret_val = true;
	
	SIZE_T fl = wcslen(src_file);
	LPCWSTR lslash = src_file+fl;
	while(*(--lslash) != L'\\' && lslash >= src_file)
	{
		;
	}

	SIZE_T pl = wcslen(path);

	while(ret_val = nextSubDir(path))
	{
		SIZE_T npl = wcslen(path);
		path[npl] = L'\\';
		wcscpy(path+npl+1, lslash+1);

		WIN32_FIND_DATA fd;
		HANDLE flnk = FindFirstFile(path, &fd);
		FindClose(flnk);

		if(flnk == INVALID_HANDLE_VALUE) // File Name collision not detected
		{
			path[npl] = 0;  // Erase filename from path
			break;
		}

		path[pl] = 0; // Restore original base dir path
	}

	return ret_val;
}

bool nextSubDir(LPWSTR dir)
{
	static wchar_t subd[MAX_SUB_DIR+1] = { L'/' };

	// Overflow detection is very expensive... Sad...
	SIZE_T i = 0;
	for(; i < MAX_SUB_DIR; ++i)
	{
		if(subd[i] != L'z')
		{
			break;
		}
	}

	if(i == MAX_SUB_DIR) // Overflow!
	{
		return false;
	}

	// Basically, this is General Purpose Base36 Little Endian Adder
	int carry = 1;
	for(i = 0; i < MAX_SUB_DIR; ++i)
	{
		if(carry && subd[i] == 0) // Add new digit to string
		{
			subd[i] = L'0';
		}

		if((subd[i] += carry) > L'z') // Carry over
		{
			subd[i] = L'0';
			carry = 1;
			continue;
		}
		else if(subd[i] == L':')
		{
			subd[i] = L'a';
		}

		carry = 0;
	}
	SIZE_T dl = wcslen(dir);
	LPWSTR dir_end = dir+dl;

	dir_end[0] = L'\\';
	wcscpy(dir_end+1, subd);
	dl = wcslen(dir);
	if(CreateDirectory(dir, NULL)) // New directory was created
	{
		LPWSTR sys_path = NULL;
		regSetRoot(HKEY_LOCAL_MACHINE);
		SIZE_T pl = regGet(path_var, L"Path", &sys_path)/sizeof(wchar_t) - 1; // -1 bc. \0
		static HANDLE heap = GetProcessHeap();
		LPWSTR new_sys_path = (LPWSTR)HeapAlloc(heap, NULL,
			(pl + dl + 2) * sizeof(wchar_t));
		wcscpy(new_sys_path, sys_path);

		if(pl + dl + 2 > MAX_S)
		{
			if(!regChk(shell, ignore_val) && MessageBox(NULL,
				L"PATH will be longer than 1024 symbols. Some programs might behave "
				L"incorrectly because of this.\n"
				L"Press OK if you plan to manually remove non-critical paths and "
				L"re-add them as symbolic links.\n"
				L"Press CANCEL to ignore this warning and never show it again.\n",
				L"PATH 'soft' limit reached",
				MB_OKCANCEL | MB_ICONSTOP | MB_SYSTEMMODAL) == IDCANCEL)
			{
				regSet(shell, ignore_val);
			}
		}

		if(new_sys_path[pl-1] != L';')
		{
			new_sys_path[pl++] = L';';
		}
		wcscpy(new_sys_path+pl, dir);

		P(std::wcout << "OLDPATH: " << sys_path << std::endl << std::endl
			<< "NEWPATH: " << new_sys_path << std::endl);

		regSet(path_var, L"Path", new_sys_path);
		SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, NULL, (LPARAM)L"Environment",
			SMTO_ABORTIFHUNG, 20, NULL); // Alert all other windows about registry change

		HeapFree(heap, NULL, sys_path);
		HeapFree(heap, NULL, new_sys_path);
	}
	else
	{
		dir[dl] = L'\\';
		dir[dl+1] = L'*';
		dir[dl+2] = 0;
		
		// Cull all broken symbolic links
		WIN32_FIND_DATA fd;
		HANDLE f = FindFirstFile(dir, &fd);
		do
		{
			LPCWSTR fn = fd.cFileName;
			if(fn[0] == L'.' && fn[1] == L'.' && fn[2] == 0 ||
				fn[0] == L'.' && fn[1] == 0)
			{
				continue;
			}

			wcscpy(dir+dl+1, fn);

			HANDLE h = CreateFile(
				dir,
				0,
				0x7, // Read|Write|Delete are shared
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

			if(h == INVALID_HANDLE_VALUE)
			{
				DeleteFile(dir);
			}

			CloseHandle(h);

		}
		while(FindNextFile(f, &fd));
		FindClose(f);

		dir[dl] = 0;
	}

	return true;
}