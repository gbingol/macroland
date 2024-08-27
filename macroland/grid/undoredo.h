#pragma once

#include <string>
#include <vector>

#include <wx/wx.h>

#include "ws_cell.h"





namespace grid
{
	class CWorksheetBase;

	class WSUndoRedoEvent
	{
	public:
		WSUndoRedoEvent(CWorksheetBase* worksheet, bool CanRedo)
		{
			m_CanRedo = CanRedo;
			m_WSBase = worksheet;
		}
		virtual ~WSUndoRedoEvent() = default;

		void ShowWorksheet(); //Show the worksheet where events are happening

		auto GetEventSource() const {
			return m_WSBase;
		}

		bool CanRedo() const {
			return m_CanRedo;
		}

		virtual std::wstring GetToolTip(bool IsUndo) = 0;
		virtual void Undo() = 0;
		virtual void Redo() = 0;

	protected:
		CWorksheetBase* m_WSBase;
		bool m_CanRedo;
	};




	//triggered when data is changed by user by typing
	class CellDataChanged : public WSUndoRedoEvent
	{
	public:
		CellDataChanged(CWorksheetBase* worksheet, int row, int col): 
		WSUndoRedoEvent(worksheet, true)
		{
			m_row = row;
			m_col = col;
		}
		

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		std::wstring m_InitVal; //Before the change
		std::wstring m_LastVal; //After the change

	private:
		int m_row, m_col;
	};



	//triggered with SetCellValue
	class CellValueChangedEvent : public WSUndoRedoEvent
	{
	public:
		CellValueChangedEvent(CWorksheetBase* worksheet, int row, int col):
		WSUndoRedoEvent(worksheet, true)
		{
			m_row = row;
			m_col = col;
		}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		std::wstring m_InitVal; //Before the change
		std::wstring m_LastVal; //After the change

	private:
		int m_row, m_col;
	};



	class CellContentDeleted : public WSUndoRedoEvent
	{
	public:
		CellContentDeleted(CWorksheetBase* worksheet):
		WSUndoRedoEvent(worksheet, true) {}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		void SetInitialCells(const std::vector<Cell>& Cells){
			m_InitVal = Cells;
		}

		void SetCoords(const wxGridCellCoords& TL, const wxGridCellCoords& BR) {
			m_TL = TL;
			m_BR = BR;
		}

	private:
		std::vector<Cell> m_InitVal; //Before the change
		wxGridCellCoords m_TL, m_BR;
	};



	struct CellBGColorChangedEvent : WSUndoRedoEvent
	{
		CellBGColorChangedEvent(CWorksheetBase* worksheet):
		WSUndoRedoEvent(worksheet, true) {}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		std::vector<Cell> m_InitVal; //Before the change
		std::vector<Cell> m_LastVal; //After the change

		wxGridCellCoords m_TL, m_BR;
	};



	struct TextColorChangedEvent : WSUndoRedoEvent
	{
		TextColorChangedEvent(CWorksheetBase* worksheet):
		WSUndoRedoEvent(worksheet, true) {}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		std::vector<Cell> m_InitVal; //Before the change
		std::vector<Cell> m_LastVal; //After the change

		wxGridCellCoords m_TL, m_BR;
	};



	struct FontChangedEvent : WSUndoRedoEvent
	{
		FontChangedEvent(CWorksheetBase* worksheet):
		WSUndoRedoEvent(worksheet, true) {}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		std::vector<Cell> m_InitVal, m_LastVal;
		std::string m_FontProperty; //Info on changed property, such as "Size", "Face", "Bold" ...
		wxGridCellCoords m_TL, m_BR;
	};



	struct CellAlignmentChangedEvent : WSUndoRedoEvent
	{
		CellAlignmentChangedEvent(CWorksheetBase* worksheet):
		WSUndoRedoEvent(worksheet, true) {}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		std::vector<Cell> m_InitVal, m_LastVal;
		wxGridCellCoords m_TL, m_BR;
	};



	class RowsDeleted : public WSUndoRedoEvent
	{
	public:
		RowsDeleted(CWorksheetBase* worksheet): 
		WSUndoRedoEvent(worksheet, true) {}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		void SetInfo(int Start, int Length) 
		{
			m_StartPos = Start;
			m_Length = Length;
		}

		void SetInitialCells(const std::vector<Cell>& InitCells){
			m_InitVal = InitCells;
		}

	private:
		std::vector<Cell> m_InitVal; //Before the change
		int m_StartPos, m_Length;
	};



	class RowsInserted : public WSUndoRedoEvent
	{
	public:
		RowsInserted(CWorksheetBase* worksheet):
		WSUndoRedoEvent(worksheet, true) {}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		void SetInfo(int Start, int Length)
		{
			m_StartPos = Start;
			m_Length = Length;
		}

	private:
		int m_StartPos, m_Length;
	};





	class ColumnsDeleted : public WSUndoRedoEvent
	{
	public:
		ColumnsDeleted(CWorksheetBase* worksheet) :
		WSUndoRedoEvent(worksheet, true) {}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		void SetInfo(int Start, int Length)
		{
			m_StartPos = Start;
			m_Length = Length;
		}

		void SetInitialCells(const std::vector<Cell>& InitCells){
			m_InitVal = InitCells;
		}

	private:

		std::vector<Cell> m_InitVal; //Before the change
		int m_StartPos, m_Length;
	};





	class ColumnsInserted : public WSUndoRedoEvent
	{
	public:
		ColumnsInserted(CWorksheetBase* worksheet):
		WSUndoRedoEvent(worksheet, true) {}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		void SetInfo(int Start, int Length) 
		{
			m_StartPos = Start;
			m_Length = Length;
		}

	private:
		int m_StartPos, m_Length;
	};



	class RowColSizeChanged : public WSUndoRedoEvent
	{
	public:
		enum class ENTITY { ROW = 0, COL };

		RowColSizeChanged(CWorksheetBase* worksheet, ENTITY entity, int Number):
		WSUndoRedoEvent(worksheet, true)
		{
			m_Pos = Number;
			m_Entity = entity;
		}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		int m_PrevSize;
		int m_FinalSize;

	private:
		int m_Pos;
		ENTITY m_Entity;
	};


	class DataPasted : public WSUndoRedoEvent
	{
	public:
		DataPasted(CWorksheetBase* worksheet):
		WSUndoRedoEvent(worksheet, false) {} //Cannot redo

		void Undo() override; 
		void Redo() override{}

		std::wstring GetToolTip(bool IsUndo) override;

		void SetCoords(const wxGridCellCoords& TL, const wxGridCellCoords& BR) 
		{
			m_TL = TL;
			m_BR = BR;
		}

		//first:TL, second:BR
		void SetCoords(const std::pair< wxGridCellCoords, wxGridCellCoords>& Coords) 
		{
			m_TL = Coords.first;
			m_BR = Coords.second;
		}

		void SetPaste(int pastewhat) {
			m_PasteWhat = pastewhat;
		}

	private:
		int m_PasteWhat;
		wxGridCellCoords m_TL, m_BR;
	};



	class DataCut : public WSUndoRedoEvent
	{
	public:
		DataCut(CWorksheetBase* worksheet):
		WSUndoRedoEvent(worksheet, true) {}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		void SetCells(const std::vector<Cell>& Cells){
			m_Value = Cells;
		}

		void SetCoords(const wxGridCellCoords& TL, const wxGridCellCoords& BR) 
		{
			m_TL = TL;
			m_BR = BR;
		}

	private:

		std::vector<Cell>	m_Value;
		wxGridCellCoords m_TL, m_BR;
	};



	class DataMovedEvent : public WSUndoRedoEvent
	{
	public:
		//can also be copied during moving (Ctrl button)
		DataMovedEvent(CWorksheetBase* worksheet, bool Moved = true): WSUndoRedoEvent(worksheet, true) 
		{
			m_Moved = Moved;
		}

		void Undo() override;
		void Redo() override;

		std::wstring GetToolTip(bool IsUndo) override;

		void SetInitCoords(const wxGridCellCoords& TL, const wxGridCellCoords& BR) 
		{
			m_Init_TL = TL;
			m_Init_BR = BR;
		}

		void SetFinalCoords(const wxGridCellCoords& TL, const wxGridCellCoords& BR) 
		{
			m_Final_TL = TL;
			m_Final_BR = BR;
		}

		void SetCells(const std::vector<Cell>& Cells){
			m_CellValues = Cells;
		}

	private:

		std::vector<Cell>	m_CellValues; //Initial location and values

		wxGridCellCoords m_Init_TL, m_Init_BR;
		wxGridCellCoords m_Final_TL, m_Final_BR;


		//if user presses Ctrl button then it is copied
		bool m_Moved = true;
	};


}
