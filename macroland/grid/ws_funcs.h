#pragma once

#include <string>
#include <tuple>
#include <optional>

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/xml/xml.h>



namespace grid
{
	class CWorksheetBase;
	class Cell;

	std::string ColNumtoLetters(size_t num);

	int LetterstoColumnNumber(const std::string& letters);

	wxFont StringtoFont(const wxString& str);
	std::string FonttoString(const wxFont& font);


	//tabs and newlines
	wxString GenerateTabString(
		const CWorksheetBase* ws,
		const wxGridCellCoords& TopLeft,
		const wxGridCellCoords& BottomRight);

	std::pair<wxGridCellCoords, wxGridCellCoords>
		AddSelToClipbrd(const CWorksheetBase* ws);

	

	/****************************************************** */


	std::optional<wxXmlDocument> CreateXMLDoc(const wxString& XMLString);

	wxString GetXMLData();

	bool SupportsXML();

	std::vector<Cell> XMLDocToCells(
		const grid::CWorksheetBase* ws,
		const wxXmlDocument& xmlDoc);


	std::tuple<long, long, bool>
		XMLNodeToRowsCols(const wxXmlNode* xmlNode);

	bool ParseXMLDoc(
		grid::CWorksheetBase* ws,
		const wxXmlDocument& xmlDoc);

	//UTF8 string
	std::string GenerateXMLString(
		const grid::CWorksheetBase* ws,
		const wxGridCellCoords& TopLeft,
		const wxGridCellCoords& BottomRight);

	//UTF8 string
	std::string GenerateXMLString(grid::CWorksheetBase* ws);


	//used by worksheet and others (probably they should not directly use it!)
	struct XMLDataFormat : public wxDataFormat
	{
		XMLDataFormat() : wxDataFormat("XMLDataFormat") {}
	};


	class XMLDataObject : public wxTextDataObject
	{
	public:
		XMLDataObject(const std::string& xmlUTF8 = "") :
			wxTextDataObject(xmlUTF8)
		{
			SetFormat(XMLDataFormat());
		}
	};
}