Set oShell=CreateObject("Wscript.Shell") 
Dim a
a="cmd /c taskkill /im wri_keylogger.exe"
oShell.Run a,0,false