"""
It is recommended NOT to change or to edit this file

This file is run when the system starts, so anything wrong here will 
either prevent system from starting or will break things
"""



"""
When designing wxPython apps do not use app.MainLoop in your application
since there can only be one main loop in wxPython in a single process. 

Attempting to use app.MainLoop elsewhere will crash the whole system 
unless a subprocess is used.
"""



import wx
app=wx.App(useBestVisual = True)
if app.IsMainLoopRunning() == False:
	app.MainLoop() #there can only be one main loop in wxPython