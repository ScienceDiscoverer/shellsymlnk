# Shell Symbolic PATH Links Setter
This tool allows to shorten your system PATH environment variable immensly and thus increase system performance and incorporate much more folders in you PATH that would othervise be possible. If path becomes longer than 1024 characters, some programs might start working incorrectly, this tool solves this problem too.

When launched without attributes, you can set a hidden folder where symboic links will be placed. The recomended path is the shortest one like `C:\l`. It also stores appropriate registry keys to integrate into Explorer. It sets following keys:
```
HKCR\shell\makepathsymlink
HKCR\Directory\shell\makepathsymlink
HKCr\Directory\Background\shell\makepathsymlink
```
You can remove them if you deside to stop using the tool. By right clicking a folder or file with `Shift` pressed you can add it to the previousely created folder in a subfolder 0, 1, 2 ... ZZZ. The folder will be then added to your system PATH variable and `WM_SETTINGCHANGE` will be broadcast. After this, you can freely acces linked files from CMD or RUN dialoge. Tool will warn about name mistmatches and offer you to try next folder if files are critical for the application, or ignore the files (usefull for things like licence.txt or readme.txt). Remember that OS chooses first file it finds, so place newer/better files in lower numbered folders. You can also use it as command line tool by passing it full path to file/folder you want to add to PATH. You will need to run it with admin rights for it to work (creating symbolic links + editing PATH).

You might want to localise context menu entry name located at lines 233-235 of `main.cpp` to you system local.
