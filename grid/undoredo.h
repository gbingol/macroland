#pragma once

#include <string>
#include <vector>

#include <wx/wx.h>
#include <wx/grid.h>

#include "dllimpexp.h"

namespace grid
{
	class Cell;
	class CWorksheetBase;

	class WorksheetUndoRedoEvent
	{
	public:
		DLLGRID WorksheetUndoRedoEvent(CWorksheetBase* worksheet, bool CanRedo);
		virtual DLLGRID ~WorksheetUndoRedoEvent();

		DLLGRID void ShowWorksheet(); //Show the worksheet where events are happening

		DLLGRID CWorksheetBase* GetEventSource() const
		{
			return m_WSBase;
		}

		DLLGRID bool CanRedo() const
		{
			return m_CanRedo;
		}

		virtual DLLGRID std::wstring GetToolTip(bool IsUndo) = 0;
		virtual DLLGRID void Undo() = 0;
		virtual DLLGRID void Redo() = 0;

	protected:
		/*
		Initially say a cell has a value of 0 and then we changed it to 10
		Undo should restore the cell to the initial value of 0
		Redo should restore the cell to the last value of 10
		*/
		CWorksheetBase* m_WSBase;

		bool m_CanRedo;

	};





	//triggered when data is changed by user by typing
	class CellDataChanged : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID CellDataChanged(CWorksheetBase* worksheet, int row, int col);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		std::wstring m_InitVal; //Before the change
		std::wstring m_LastVal; //After the change

	private:
		int m_row;
		int m_col;
	};



	//triggered with SetCellValue
	class CellValueChangedEvent : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID CellValueChangedEvent(CWorksheetBase* worksheet, int row, int col);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		std::wstring m_InitVal; //Before the change
		std::wstring m_LastVal; //After the change

	private:
		int m_row;
		int m_col;
	};



	class CellContentDeleted : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID CellContentDeleted(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		DLLGRID void SetInitialCells(const std::vector<Cell>& Cells);

		DLLGRID void SetCoords(const wxGridCellCoords& TL, const wxGridCellCoords& BR) {
			m_TL = TL;
			m_BR = BR;
		}

	private:
		std::vector<Cell> m_InitVal; //Before the change

		wxGridCellCoords m_TL;
		wxGridCellCoords m_BR;
	};



	class CellBGColorChangedEvent : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID CellBGColorChangedEvent(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		std::vector<Cell> m_InitVal; //Before the change
		std::vector<Cell> m_LastVal; //After the change

		wxGridCellCoords m_TL;
		wxGridCellCoords m_BR;
	};



	class TextColorChangedEvent : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID TextColorChangedEvent(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		std::vector<Cell> m_InitVal; //Before the change
		std::vector<Cell> m_LastVal; //After the change

		wxGridCellCoords m_TL;
		wxGridCellCoords m_BR;
	};



	class FontChangedEvent : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID FontChangedEvent(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		std::vector<Cell> m_InitVal;
		std::vector<Cell> m_LastVal;

		std::string m_FontProperty; //Info on changed property, such as "Size", "Face", "Bold" ...

		wxGridCellCoords m_TL;
		wxGridCellCoords m_BR;
	};



	class CellAlignmentChangedEvent : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID CellAlignmentChangedEvent(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		std::vector<Cell> m_InitVal;
		std::vector<Cell> m_LastVal;

		wxGridCellCoords m_TL;
		wxGridCellCoords m_BR;
	};



	class RowsDeleted : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID RowsDeleted(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		DLLGRID void SetInfo(int Start, int Length) 
		{
			m_StartPos = Start;
			m_Length = Length;
		}

		DLLGRID void SetInitialCells(const std::vector<Cell>& InitCells);

	private:
		std::vector<Cell> m_InitVal; //Before the change

		int m_StartPos;
		int m_Length;
	};



	class RowsInserted : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID RowsInserted(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		DLLGRID void SetInfo(int Start, int Length)
		{
			m_StartPos = Start;
			m_Length = Length;
		}

	private:

		int m_StartPos;
		int m_Length;
	};





	class ColumnsDeleted : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID ColumnsDeleted(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		DLLGRID void SetInfo(int Start, int Length)
		{
			m_StartPos = Start;
			m_Length = Length;
		}

		DLLGRID void SetInitialCells(const std::vector<Cell>& InitCells);

	private:

		std::vector<Cell> m_InitVal; //Before the change

		int m_StartPos;
		int m_Length;
	};





	class ColumnsInserted : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID ColumnsInserted(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		DLLGRID void SetInfo(int Start, int Length) 
		{
			m_StartPos = Start;
			m_Length = Length;
		}

	private:
		int m_StartPos;
		int m_Length;
	};



	class RowColSizeChanged : public WorksheetUndoRedoEvent
	{

	public:
		enum class ENTITY { ROW = 0, COL };

		DLLGRID RowColSizeChanged(CWorksheetBase* worksheet, ENTITY entity, int Number);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		int m_PrevSize;
		int m_FinalSize;

	private:
		int m_ColRowNumber;
		ENTITY m_Entity;
	};


	class DataPasted : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID DataPasted(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		DLLGRID void SetCoords(const wxGridCellCoords& TL, const wxGridCellCoords& BR) 
		{
			m_TL = TL;
			m_BR = BR;
		}

		//first:TL, second:BR
		DLLGRID void SetCoords(const std::pair< wxGridCellCoords, wxGridCellCoords>& Coords) 
		{
			m_TL = Coords.first;
			m_BR = Coords.second;
		}


		DLLGRID void SetPaste(int pastewhat) 
		{
			m_PasteWhat = pastewhat;
		}

	private:
		int m_PasteWhat;

		wxGridCellCoords m_TL;
		wxGridCellCoords m_BR;

	};



	class DataCut : public WorksheetUndoRedoEvent
	{
	public:
		DLLGRID DataCut(CWorksheetBase* worksheet);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		DLLGRID void SetCells(const std::vector<Cell>& Cells);

		DLLGRID void SetCoords(const wxGridCellCoords& TL, const wxGridCellCoords& BR) 
		{
			m_TL = TL;
			m_BR = BR;
		}

	private:

		std::vector<Cell>	m_Value;

		wxGridCellCoords m_TL;
		wxGridCellCoords m_BR;
	};



	class DataMovedEvent : public WorksheetUndoRedoEvent
	{
	public:
		//can also be copied during moving (Ctrl button)
		DLLGRID DataMovedEvent(CWorksheetBase* worksheet, bool Moved = true);

		DLLGRID void Undo();
		DLLGRID void Redo();

		DLLGRID std::wstring GetToolTip(bool IsUndo) override;

		DLLGRID bool operator==(const WorksheetUndoRedoEvent& other) const;

		DLLGRID void SetInitCoords(const wxGridCellCoords& TL, const wxGridCellCoords& BR) 
		{
			m_Init_TL = TL;
			m_Init_BR = BR;
		}

		DLLGRID void SetFinalCoords(const wxGridCellCoords& TL, const wxGridCellCoords& BR) 
		{
			m_Final_TL = TL;
			m_Final_BR = BR;
		}

		DLLGRID void SetCells(const std::vector<Cell>& Cells);

	private:

		std::vector<Cell>	m_CellValues; //Initial location and values

		wxGridCellCoords m_Init_TL, m_Init_BR;
		wxGridCellCoords m_Final_TL, m_Final_BR;


		//if user presses Ctrl button then it is copied
		bool m_Moved = true;
	};


}
