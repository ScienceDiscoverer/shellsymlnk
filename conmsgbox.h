#pragma once

/*___________________________________________________________________
|  consoleMessageBox:
|    Creates console window and displays text with buttons, just like
|    standart MessageBox, but much better and cooler!
|
|   text: String to display, 32766 lines max
|  title: Title of the console dialog
|
|  Return value:
|     ABORT pressed -> IDABORT
|     RETRY pressed -> IDRETRY
|    IGNORE pressed -> IDIGNORE
|____________________________________________________________________*/
int consoleMessageBox(LPCWSTR text, LPCWSTR title);