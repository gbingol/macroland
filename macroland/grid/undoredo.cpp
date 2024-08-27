#include "undoredo.h"

#include "ws_funcs.h"
#include "worksheetbase.h"
#include "workbookbase.h"



namespace grid
{

	
	void WSUndoRedoEvent::ShowWorksheet()
	{
		auto workbook = m_WSBase->GetWorkbook();
		if (m_WSBase != workbook->GetActiveWS())
			workbook->ShowWorksheet(m_WSBase);

	}



	/*************   Cell Data Changed Event ***************************/
	


	void CellDataChanged::Undo()
	{
		ShowWorksheet();

		m_WSBase->SetCellValue(m_row, m_col, m_InitVal);

		if (m_WSBase->IsSelection())
			m_WSBase->ClearSelection();

		m_WSBase->GoToCell(m_row, m_col);
	}

	void CellDataChanged::Redo()
	{
		ShowWorksheet();

		m_WSBase->SetCellValue(m_row, m_col, m_LastVal);
		if (m_WSBase->IsSelection())
			m_WSBase->ClearSelection();

		m_WSBase->GoToCell(m_row, m_col);
	}


	std::wstring CellDataChanged::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;
		wxString ValueToShow;//in case user enters a very long text

		if (m_LastVal.length() > 40)
			ValueToShow << "\'" << m_LastVal.substr(0, 39) << "..." << "\'";
		else
			ValueToShow = m_LastVal;

		if (IsUndo)
			ToolTip << L"Undo typing " << ValueToShow.ToStdWstring() << L" in " << ColNumtoLetters(m_col + 1).c_str() << m_row + 1;
		else
			ToolTip << L"Redo typing " << ValueToShow.ToStdWstring() << L" in " << ColNumtoLetters(m_col + 1).c_str() << m_row + 1;


		return ToolTip.str();
	}




	/*************   Cell Value Changed Event ***************************/

	void CellValueChangedEvent::Undo()
	{
		ShowWorksheet();

		m_WSBase->SetCellValue(m_row, m_col, m_InitVal);

		if (m_WSBase->IsSelection())
			m_WSBase->ClearSelection();

		m_WSBase->GoToCell(m_row, m_col);
	}

	void CellValueChangedEvent::Redo()
	{
		ShowWorksheet();

		m_WSBase->SetCellValue(m_row, m_col, m_LastVal);

		if (m_WSBase->IsSelection())
			m_WSBase->ClearSelection();

		m_WSBase->GoToCell(m_row, m_col);
	}


	std::wstring CellValueChangedEvent::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;
		wxString Val;//in case user enters a very long text

		if (m_LastVal.length() > 40)
			Val << "\'" << m_LastVal.substr(0, 39) << "..." << "\'";
		else
			Val = m_LastVal;

		ToolTip << (IsUndo ? "Undo typing " : "Redo typing") << Val << " in " << ColNumtoLetters(m_col + 1) << m_row + 1;

		return ToolTip.str();
	}





	/*************   Cell Content Deleted Event ***************************/

	void CellContentDeleted::Undo()
	{
		ShowWorksheet();

		for (auto elem : m_InitVal)
			m_WSBase->SetCellValue(elem.GetRow(), elem.GetCol(), elem.GetValue());


		m_WSBase->SelectBlock(m_TL, m_BR);
	}


	void CellContentDeleted::Redo()
	{
		ShowWorksheet();

		for (auto elem : m_InitVal)
			m_WSBase->SetCellValue(elem.GetRow(), elem.GetCol(), wxEmptyString);


		m_WSBase->SelectBlock(m_TL, m_BR);
	}


	std::wstring CellContentDeleted::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;
		ToolTip << (IsUndo ? "Undo " : "Redo ");

		if (m_TL == m_BR)
			ToolTip << L"deletion from " << ColNumtoLetters(m_TL.GetCol() + 1).c_str() << m_TL.GetRow() + 1;
		else
			ToolTip << L"deletion from " << ColNumtoLetters(m_TL.GetCol() + 1).c_str() << m_TL.GetRow() + 1 << L" to "
			<< ColNumtoLetters(m_BR.GetCol() + 1) << m_BR.GetRow() + 1;

		return ToolTip.str();
	}





	/*************   Cell BG Color Changed Event ***************************/

	void CellBGColorChangedEvent::Undo()
	{
		ShowWorksheet();

		for (const auto& c : m_InitVal)
			m_WSBase->SetCellBackgroundColour(c.GetRow(), c.GetCol(), c.GetFormat().GetBackgroundColor());

		m_WSBase->SelectBlock(m_TL, m_BR);
	}


	void CellBGColorChangedEvent::Redo()
	{
		ShowWorksheet();

		for (const auto& c : m_LastVal)
			m_WSBase->SetCellBackgroundColour(c.GetRow(), c.GetCol(), c.GetFormat().GetBackgroundColor());

		m_WSBase->SelectBlock(m_TL, m_BR);
	}


	std::wstring CellBGColorChangedEvent::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;
		ToolTip << (IsUndo ? "Undo " : "Redo ");

		if (m_TL == m_BR)
			ToolTip << "background color change in cell " << ColNumtoLetters(m_TL.GetCol() + 1) << m_TL.GetRow() + 1;
		else
			ToolTip << "background color change in cells " << ColNumtoLetters(m_TL.GetCol() + 1) << m_TL.GetRow() + 1 << " to "
			<< ColNumtoLetters(m_BR.GetCol() + 1) << m_BR.GetRow() + 1;

		return ToolTip.str();
	}






	/*************   Text Color Changed Event ***************************/

	void TextColorChangedEvent::Undo()
	{
		ShowWorksheet();

		for (const auto& cell : m_InitVal)
			m_WSBase->SetCellTextColour(cell.GetRow(), cell.GetCol(), cell.GetFormat().GetTextColor());


		m_WSBase->SelectBlock(m_TL, m_BR);
	}


	void TextColorChangedEvent::Redo()
	{
		ShowWorksheet();

		for (const auto& cell : m_LastVal)
			m_WSBase->SetCellTextColour(cell.GetRow(), cell.GetCol(), cell.GetFormat().GetTextColor());


		m_WSBase->SelectBlock(m_TL, m_BR);
	}


	std::wstring TextColorChangedEvent::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;
		ToolTip << (IsUndo ? "Undo " : "Redo ");

		if (m_TL == m_BR)
			ToolTip << "text color change in cell " << ColNumtoLetters(m_TL.GetCol() + 1) << m_TL.GetRow() + 1;
		else
			ToolTip << "text color change in cells " << ColNumtoLetters(m_TL.GetCol() + 1) << m_TL.GetRow() + 1 << " to "
			<< ColNumtoLetters(m_BR.GetCol() + 1) << m_BR.GetRow() + 1;

		return ToolTip.str();
	}







	/*************   Font Changed Event ***************************/

	void FontChangedEvent::Undo()
	{
		ShowWorksheet();

		for (auto cell : m_InitVal)
			m_WSBase->SetCellFont(cell.GetRow(), cell.GetCol(), cell.GetFormat().GetFont());


		m_WSBase->SelectBlock(m_TL, m_BR);
	}


	void FontChangedEvent::Redo()
	{
		ShowWorksheet();

		for (auto cell : m_LastVal)
			m_WSBase->SetCellFont(cell.GetRow(), cell.GetCol(), cell.GetFormat().GetFont());

		m_WSBase->SelectBlock(m_TL, m_BR);
	}


	std::wstring FontChangedEvent::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;
		ToolTip << (IsUndo ? "Undo text " : "Redo text ") << m_FontProperty << " ";

		if (m_TL == m_BR)
			ToolTip << "in cell " << ColNumtoLetters(m_TL.GetCol() + 1) << m_TL.GetRow() + 1;
		else
			ToolTip << "in cells " << ColNumtoLetters(m_TL.GetCol() + 1) << m_TL.GetRow() + 1 << " to "
			<< ColNumtoLetters(m_BR.GetCol() + 1) << m_BR.GetRow() + 1;

		return ToolTip.str();
	}




	/*************   Cell Alignment Changed Event ***************************/

	
	void CellAlignmentChangedEvent::Undo()
	{
		ShowWorksheet();

		for (const auto& cell : m_InitVal)
			m_WSBase->SetCellAlignment(cell.GetRow(), cell.GetCol(), cell.GetFormat().GetHAlign(), cell.GetFormat().GetVAlign());

		m_WSBase->SelectBlock(m_TL, m_BR);
	}

	void CellAlignmentChangedEvent::Redo()
	{
		ShowWorksheet();

		for (const auto& cell : m_LastVal)
			m_WSBase->SetCellAlignment(cell.GetRow(), cell.GetCol(), cell.GetFormat().GetHAlign(), cell.GetFormat().GetVAlign());

		m_WSBase->SelectBlock(m_TL, m_BR);
	}

	std::wstring CellAlignmentChangedEvent::GetToolTip(bool IsUndo)
	{
		std::wstringstream Tip;
		Tip << (IsUndo ? "Undo " : "Redo ") + std::wstring(L"cell alignment");

		if (m_TL == m_BR)
			Tip << "in cell " << ColNumtoLetters(m_TL.GetCol() + 1) << m_TL.GetRow() + 1;
		else
			Tip << "in cells " << ColNumtoLetters(m_TL.GetCol() + 1) << m_TL.GetRow() + 1 << " to "
			<< ColNumtoLetters(m_BR.GetCol() + 1) << m_BR.GetRow() + 1;

		return Tip.str();
	}




	/*************   Rows Deleted Event ***************************/

	void RowsDeleted::Undo()
	{
		ShowWorksheet();

		m_WSBase->InsertRows(m_StartPos, m_Length);

		for (const auto& elem : m_InitVal) {
			m_WSBase->SetCellValue(elem.GetRow(), elem.GetCol(), elem.GetValue());
			m_WSBase->ApplyCellFormat(elem.GetRow(), elem.GetCol(), elem);
		}
	}

	void RowsDeleted::Redo()
	{
		ShowWorksheet();

		m_WSBase->DeleteRows(m_StartPos, m_Length);
	}

	std::wstring RowsDeleted::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;

		if (IsUndo)
			ToolTip << "Undo deletion of " << m_Length << " rows.";
		else
			ToolTip << "Redo deletion of " << m_Length << " rows.";

		return ToolTip.str();
	}





	/*************   Rows Inserted Event ***************************/

	void RowsInserted::Undo()
	{
		ShowWorksheet();

		m_WSBase->DeleteRows(m_StartPos, m_Length);
	}

	void RowsInserted::Redo()
	{
		ShowWorksheet();

		m_WSBase->InsertRows(m_StartPos, m_Length);
	}

	std::wstring RowsInserted::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;

		if (IsUndo)
			ToolTip << "Undo insertion of " << m_Length << " rows.";

		else
			ToolTip << "Redo insertion of " << m_Length << " rows.";


		return ToolTip.str();
	}




	/*************   Columns Deleted Event ***************************/

	void ColumnsDeleted::Undo()
	{
		ShowWorksheet();
		m_WSBase->InsertCols(m_StartPos, m_Length);

		for (const auto& elem : m_InitVal) {
			m_WSBase->SetCellValue(elem.GetRow(), elem.GetCol(), elem.GetValue());
			m_WSBase->ApplyCellFormat(elem.GetRow(), elem.GetCol(), elem);
		}

	}

	void ColumnsDeleted::Redo()
	{
		ShowWorksheet();
		m_WSBase->DeleteCols(m_StartPos, m_Length);
	}

	std::wstring ColumnsDeleted::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;

		ToolTip << (IsUndo ? "Undo" : "Redo") << " deletion of " << m_Length << " cols.";

		return ToolTip.str();
	}



	/*************   Columns Inserted Event ***************************/

	void ColumnsInserted::Undo()
	{
		ShowWorksheet();
		m_WSBase->DeleteCols(m_StartPos, m_Length);
	}


	void ColumnsInserted::Redo()
	{
		ShowWorksheet();
		m_WSBase->InsertCols(m_StartPos, m_Length);
	}


	std::wstring ColumnsInserted::GetToolTip(bool IsUndo)
	{
		std::wstring s = (IsUndo ? L"Undo" : L"Redo ");
		return  s + L"insertion of " + std::to_wstring(m_Length) + L" cols.";
	}






	/*************   Row / Col Size Changed Event ***************************/

	void RowColSizeChanged::Undo()
	{
		if (m_Entity == ENTITY::ROW)
			m_WSBase->SetRowSize(m_Pos, m_PrevSize);
		else
			m_WSBase->SetColSize(m_Pos, m_PrevSize);
	}

	void RowColSizeChanged::Redo()
	{
		if (m_Entity == ENTITY::ROW)
			m_WSBase->SetRowSize(m_Pos, m_FinalSize);
		else
			m_WSBase->SetColSize(m_Pos, m_FinalSize);
	}

	std::wstring RowColSizeChanged::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;
		ToolTip << (IsUndo ? L"Undo " : L"Redo ");

		if (m_Entity == ENTITY::ROW)
			ToolTip << "height change of row " << m_Pos + 1;
		else
			ToolTip << "width change of column " << ColNumtoLetters(m_Pos + 1);

		return ToolTip.str();
	}





	/*************   Data Pasted Event ***************************/

	void DataPasted::Undo()
	{
		ShowWorksheet();

		if (m_PasteWhat == (int)CWorksheetBase::PASTE::ALL) {
			m_WSBase->ClearBlockFormat(m_TL, m_BR);
			m_WSBase->ClearBlockContent(m_TL, m_BR);
		}
		else if (m_PasteWhat == (int)CWorksheetBase::PASTE::VALUES)
			m_WSBase->ClearBlockContent(m_TL, m_BR);

		else if (m_PasteWhat == (int)CWorksheetBase::PASTE::FORMAT)
			m_WSBase->ClearBlockFormat(m_TL, m_BR);
	}


	std::wstring DataPasted::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;

		if (m_PasteWhat == (int)CWorksheetBase::PASTE::ALL)
			ToolTip << "Undo paste ";

		else if (m_PasteWhat == (int)CWorksheetBase::PASTE::VALUES)
			ToolTip << "Undo paste values ";

		else if (m_PasteWhat == (int)CWorksheetBase::PASTE::FORMAT)
			ToolTip << "Undo paste format ";

		ToolTip << "in cells " << ColNumtoLetters(m_TL.GetCol() + 1) << m_TL.GetRow() + 1 << " to "
			<< ColNumtoLetters(m_BR.GetCol() + 1) << m_BR.GetRow() + 1;

		return ToolTip.str();
	}





	/*************   Data Cut Event ***************************/

	void DataCut::Undo()
	{
		ShowWorksheet();

		for (const auto& elem : m_Value)
		{
			m_WSBase->SetCellValue(elem.GetRow(), elem.GetCol(), elem.GetValue());
			m_WSBase->ApplyCellFormat(elem.GetRow(), elem.GetCol(), elem);
		}
	}


	void DataCut::Redo()
	{
		ShowWorksheet();

		for (const auto& elem : m_Value)
		{
			m_WSBase->SetCellValue(elem.GetRow(), elem.GetCol(), wxEmptyString);
			m_WSBase->SetCellFormattoDefault(elem.GetRow(), elem.GetCol());
		}
	}


	std::wstring DataCut::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;
		ToolTip << (IsUndo ? L"Undo " : L"Redo ");

		ToolTip << "cut from cells " << ColNumtoLetters(m_TL.GetCol() + 1) << m_TL.GetRow() + 1 << ":"
			<< ColNumtoLetters(m_BR.GetCol() + 1) << m_BR.GetRow() + 1;


		return ToolTip.str();
	}



	/*************   Data Moved Event ***************************/
	
	void DataMovedEvent::Undo()
	{
		m_WSBase->ClearBlockContent(m_Final_TL, m_Final_BR);
		m_WSBase->ClearBlockFormat(m_Final_TL, m_Final_BR);


		ShowWorksheet();

		for (const auto& cell : m_CellValues) {
			m_WSBase->SetCellValue(cell.GetRow(), cell.GetCol(), cell.GetValue());
			m_WSBase->ApplyCellFormat(cell.GetRow(), cell.GetCol(), cell);
		}
	}


	void DataMovedEvent::Redo()
	{
		if (m_Moved) {
			m_WSBase->ClearBlockContent(m_Init_TL, m_Init_BR);
			m_WSBase->ClearBlockFormat(m_Init_TL, m_Init_BR);
		}


		ShowWorksheet();

		auto InitialCoords = Cell::Get_TLBR(m_CellValues);

		int diffRow = InitialCoords.first.GetRow() - m_Final_TL.GetRow();
		int diffCol = InitialCoords.first.GetCol() - m_Final_TL.GetCol();


		for (const auto& elem : m_CellValues) {
			//final=initial-diff
			int row = elem.GetRow() - diffRow;
			int col = elem.GetCol() - diffCol;

			m_WSBase->SetCellValue(row, col, elem.GetValue());
			m_WSBase->ApplyCellFormat(row, col, elem);
		}


	}

	std::wstring DataMovedEvent::GetToolTip(bool IsUndo)
	{
		std::wstringstream ToolTip;
		ToolTip << (IsUndo ? L"Undo " : L"Redo ");
		ToolTip << (m_Moved ? L"move " : L"copy ");

		assert(m_Init_BR.GetCol() >= 0);
		assert(m_Init_TL.GetCol() >= 0);

		ToolTip << "cells from " << ColNumtoLetters((size_t)m_Init_TL.GetCol() + 1) << m_Init_TL.GetRow() + 1 << ":"
			<< ColNumtoLetters((size_t)m_Init_BR.GetCol() + 1) << m_Init_BR.GetRow() + 1;


		return ToolTip.str();
	}


}