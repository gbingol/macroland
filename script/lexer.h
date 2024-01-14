#pragma once


#include <string>
#include <vector>
#include "dllimpexp.h"


namespace script
{
	/* ***********   CGenericLex  *********************

	binop =  ‘+’ | ‘-’ | ‘*’ | ‘/’ | ‘//’ | ‘^’ | ‘%’ |
			 ‘&’ | ‘~’ | ‘|’ | ‘>>’ | ‘<<’ | ‘..’ |
			 ‘<’ | ‘<=’ | ‘>’ | ‘>=’ | ‘==’ | ‘~=’ |
			 and | or

	fieldsep = ‘,’ | ‘;’
	vararg = ...
	unop = ‘-’ | not | ‘#’ | ‘~’
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

		DLLSCRIPT CToken(const CToken& other) = default;
		DLLSCRIPT ~CToken() = default;

		DLLSCRIPT auto value() const { return m_TokenValue; }

		DLLSCRIPT size_t pos() const { return m_StartPos; }

		DLLSCRIPT size_t linenum() const { return m_LineNumber; }

		DLLSCRIPT std::string desc() const {
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

		DLLSCRIPT TYPE type() const { return m_TokenType; }

		DLLSCRIPT void SetTokenType(TYPE tokentype) {
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
		DLLSCRIPT CGenericLex(const std::string& tokenize, bool ShowWhiteSpace = true)
		{
			m_Tokenize = tokenize;
			m_TokenPos = 0;

			m_LineNumber = 0;

			Tokenize(ShowWhiteSpace);
		}

		virtual DLLSCRIPT ~CGenericLex() = default;

		DLLSCRIPT CToken* next() {
			return (m_TokenPos < m_TokenList.size()) ? m_TokenList[m_TokenPos++] : nullptr;
		}

		DLLSCRIPT CToken* prev() {
			return (m_TokenPos > 0) ? m_TokenList[m_TokenPos--] : nullptr;
		}

		DLLSCRIPT CToken* at(size_t pos) const {
			return (pos < m_TokenList.size()) ? m_TokenList[pos] : nullptr;
		}

		DLLSCRIPT void insert(size_t pos, CToken* token) {
			m_TokenList.insert(m_TokenList.begin() + pos, token);
		}

		DLLSCRIPT size_t size() const {
			return m_TokenList.size();
		}

		DLLSCRIPT size_t linecount() const {
			return m_LineNumber + 1;
		}

		DLLSCRIPT CToken* operator++(int) {
			return m_TokenList[m_TokenPos++];
		}

		DLLSCRIPT CToken* operator--(int) {
			return m_TokenList[m_TokenPos--];
		}

		DLLSCRIPT auto begin() {
			return m_TokenList.begin();
		}

		DLLSCRIPT auto end() {
			return m_TokenList.end();
		}

		DLLSCRIPT bool hasmore() const {
			return m_TokenPos < m_TokenList.size();
		}

		DLLSCRIPT void reset(std::string str)
		{
			m_TokenList.clear();
			m_TokenPos = 0;

			m_Tokenize = str;

			Tokenize();
		}

		DLLSCRIPT std::vector<CToken*> GetTokenList() const {
			return m_TokenList;
		}


	protected:
		size_t m_TokenPos; //token's position in the vector

		std::vector<CToken*> m_TokenList;
	};
}