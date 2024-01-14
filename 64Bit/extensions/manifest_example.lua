--[[
	This is an example manifest file, 
	1) Change the contents in line with your extension
	2) Rename the file as manifest.lua 
	3) Place it in your own extension directory
]]


--[[
Type: string
Globally Unique Identifier(GUID)

To generate the ID:
1) Open PowerShell In Windows
2) Type/paste: New-Guid
3) Copy the ID from PowerShell and paste it here
4) Make sure it does not contain empty spaces, i.e. "  bad  " not valid
5) Do NOT change the ID later on
]]

guid = "bbfdcf4d-2835-4b9d-b29d-b18719297e7b"

--[[
Type: string
Name of the extension 
]]
extname = "Psychrometry"

--[[
Type: string
Name (or email) of the developer (company, person, etc...) 
]]
developer = "PEBytes"

--[[
Type: string
Readme file, must be either txt or html (4.0 standard)
This file is shown in extension manager
]]
readme = "readme.html"


--[[
Type: string
A short description (use readme file for detailed explanations)
]]
short_desc = "Export Menu Item in Worksheet's Selection context-menu"

--[[
Type: string
Version number (must be x.y.z ...)
]]
version = "1.0"