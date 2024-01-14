#include "baseelement.h"


namespace {
	namespace fs = std::filesystem;
}




namespace lua
{
	fs::path CElement::s_RootPath = fs::path();

	fs::path CElement::GetNormalizedPath(const fs::path& Path)
	{
		if (!Path.has_root_path())
			return s_RootPath / Path;

		return Path;
	}

	wxBitmap CElement::GetBitmap(const fs::path& BitmapPath)
	{
		wxBitmap bmp;

		if (BitmapPath.empty())
			return bmp;

		if (!bmp.LoadFile(BitmapPath.wstring(), wxBITMAP_TYPE_ANY))
			throw std::runtime_error("Image at " + BitmapPath.string() + " could not be loaded.");

		if (bmp.GetHeight() > 64 || bmp.GetWidth() > 64)
			throw std::runtime_error("Image at" + BitmapPath.string() + " is larger than 64 by 64 pixels.");

		return bmp;
	}


	void CElement::SetImgPath(const fs::path& Path)
	{
		m_ImgPath = GetNormalizedPath(Path);
	}
}