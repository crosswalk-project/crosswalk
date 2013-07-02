set obj = CreateObject("Scriptlet.TypeLib")  
WScript.StdOut.WriteLine MID(obj.GUID, 2, len(obj.GUID) - 4)
