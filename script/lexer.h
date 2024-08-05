#pragma once


#include <string>
#include <vector>
#include "dllimpexp.h"


namespace script
{
	/* ***********   CGenericLex  *********************

	binop =  �+� | �-� | �*� | �/� | �//� | �^� | �%� |
			 �&� | �~� | �|� | �>>� | �<<� | �..� |
			 �<� | �<=� | �>� | �>=� | �==� | �~=� |
			 and | or

	fieldsep = �,� | �;�
	vararg = ...
	unop = �-� | not | �#� | �~�
	label = ::
	PUNCTUATION =!
	*/

	//Lexical analyzer -> Takes an input character stream and produces tokens



	class CToken
	{
	public:
		enum class TYPE {
			NUMBER = 0, IDENTIFIER, UNOP, BINOP, BRACKETS, DELIMITER,
			WHITESPACE, NEWLINE, ASSIGNMENT, STRING
		};

	public:
		DLLSCRIPT CToken(TYPE type,
			std::string TokenValue,
			size_t pos, size_t LineNumber) :
			m_TokenType(type),
			m_TokenValue(TokenValue),
			m_StartPos{ pos },
			m_LineNumber{ LineNumber } {}

		DLLSCRIPT CToken(TYPE type,
			char c,
			size_t pos,
			size_t LineNumber) : CToken(type, std::string(1, c), pos, LineNumber) {}

		CToken(const CToken& other) = default;
		~CToken() = default;

		auto value() const { return m_TokenValue; }

		size_t pos() const { return m_StartPos; }

		size_t linenum() const { return m_LineNumber; }

		std::string desc() const {
			switch (m_TokenType)
			{
			case TYPE::NUMBER: return "num";
			case TYPE::IDENTIFIER: return "name";
			case TYPE::UNOP: return "unop";
			case TYPE::BINOP: return "binop";
			case TYPE::BRACKETS: return "brkt";
			case TYPE::ASSIGNMENT: return "asgnt";
			case TYPE::NEWLINE: return "nl";
			case TYPE::STRING: return "str";
			case TYPE::WHITESPACE: return "wspc";
			default: return "";
			}
		}

		TYPE type() const { return m_TokenType; }

		void SetTokenType(TYPE tokentype) {
			m_TokenType = tokentype;
		}

	private:
		//start position
		size_t m_StartPos;

		//At which line the occurence is found
		size_t m_LineNumber;

		//Type of the Token
		TYPE m_TokenType;

		//value of Token for any kind of Token, can be +,-, primary etc...
		std::string m_TokenValue;
	};


	class CGenericLex
	{
		std::string m_Tokenize;

		//How many lines does m_Tokenize have
		size_t m_LineNumber = 0;

		void DLLSCRIPT Tokenize(bool ShowWhiteSpace = true);


	public:
		CGenericLex(const std::string& tokenize, bool ShowWhiteSpace = true)
		{
			m_Tokenize = tokenize;
			m_TokenPos = 0;

			m_LineNumber = 0;

			Tokenize(ShowWhiteSpace);
		}

		virtual DLLSCRIPT ~CGenericLex() = default;

		CToken* next() {
			return (m_TokenPos < m_TokenList.size()) ? m_TokenList[m_TokenPos++] : nullptr;
		}

		CToken* prev() {
			return (m_TokenPos > 0) ? m_TokenList[m_TokenPos--] : nullptr;
		}

		CToken* at(size_t pos) const {
			return (pos < m_TokenList.size()) ? m_TokenList[pos] : nullptr;
		}

		void insert(size_t pos, CToken* token) {
			m_TokenList.insert(m_TokenList.begin() + pos, token);
		}

		size_t size() const {
			return m_TokenList.size();
		}

		size_t linecount() const {
			return m_LineNumber + 1;
		}

		CToken* operator++(int) {
			return m_TokenList[m_TokenPos++];
		}

		CToken* operator--(int) {
			return m_TokenList[m_TokenPos--];
		}

		auto begin() {
			return m_TokenList.begin();
		}

		auto end() {
			return m_TokenList.end();
		}

		bool hasmore() const {
			return m_TokenPos < m_TokenList.size();
		}

		void reset(std::string str)
		{
			m_TokenList.clear();
			m_TokenPos = 0;

			m_Tokenize = str;

			Tokenize();
		}

		std::vector<CToken*> GetTokenList() const {
			return m_TokenList;
		}


	protected:
		size_t m_TokenPos; //token's position in the vector
		std::vector<CToken*> m_TokenList;
	};
}