from wx import Icon, Image
from pathlib import Path as _Path

def makeicon(path:_Path)->Icon:
	"""
	path: image's full path 
	"""

	if(not path.is_absolute()):
		raise ValueError(path + " is relative path, full path expected.")
		
	if(not path.exists()):
		raise ValueError("Invalid path: " + path)

	icon = Icon()
	image = Image()
	image.LoadFile(str(path))
	bmp=image.ConvertToBitmap()
	icon.CopyFromBitmap(bmp)

	return icon


