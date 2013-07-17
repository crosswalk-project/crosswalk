if WScript.Arguments.length <> 1 then
    set obj = CreateObject("Scriptlet.TypeLib")  
    WScript.StdOut.WriteLine MID(obj.GUID, 2, len(obj.GUID) - 4)
else
    WScript.StdOut.WriteLine WScript.Arguments(0)
end if
