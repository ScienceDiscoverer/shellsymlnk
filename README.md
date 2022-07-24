# Shell Symbolic PATH Links Setter
This tool allows to shorten your system PATH environment variable immensely and thus increase system performance and incorporate much more folders in your PATH that would otherwise be possible. If the path becomes longer than 1024 characters, some programs might start working incorrectly, this tool solves this problem too.

When launched without attributes, you can set a hidden folder where symbolic links will be placed. The recommended path is the shortest one, like `C:\l`. It also stores appropriate registry keys to integrate into Explorer. It sets the following keys:
```
HKCR\shell\makepathsymlink
HKCR\Directory\shell\makepathsymlink
HKCr\Directory\Background\shell\makepathsymlink
```
You can remove them if you decide to stop using the tool. By right-clicking a folder or file with `Shift` pressed you can add it to the previously created folder in a subfolder 0, 1, 2 ... ZZZ. The folder will be then added to your system PATH variable and `WM_SETTINGCHANGE` will be broadcast. After this, you can freely access linked files from CMD or RUN dialogue. The tool will warn about name mismatches and offer you to try the next folder if files are critical for the application, or ignore the files (useful for things like licence.txt or readme.txt). Remember that OS chooses the first file it finds, so place newer/better files in lower numbered folders. You can also use it as a command line tool by passing it full path to the file/folder you want to add to PATH. You will need to run it with admin rights for it to work (creating symbolic links + editing PATH).

Furthermore, you might want to localise context menu entry name located at lines 233-235 of `main.cpp` to your system local.
