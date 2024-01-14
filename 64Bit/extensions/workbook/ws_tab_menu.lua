local btndlgImport = std.Button.new{
	title = "Import", 
	py = ACTIVEDIR.."/frmImport.py", 
	img = ACTIVEDIR.."/icons/import.png"}



std.append() --menu separator
std.append(btndlgImport)
