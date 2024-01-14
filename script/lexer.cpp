#include "lexer.h"
#include <stdexcept>


namespace script
{
	void CGenericLex::Tokenize(bool ShowWhiteSpace)
	{
		std::string tokenStr;
		size_t len = m_Tokenize.length();

		//can't parse an empty string
		if (m_Tokenize.empty())
			return;

		for (size_t index = 0; index < len; ++index)
		{
			char c = m_Tokenize[index];

			if (!isascii(c))
				continue;

			else if (c == 0 || c == '\0')
				break;

			else if ((c == ' ' || c == '\t') && ShowWhiteSpace)
			{
				size_t startpos = index;
				char wspc = c;

				while ((c = m_Tokenize[++index]) == wspc)
					tokenStr += c;

				index--;
				m_TokenList.push_back(new CToken(CToken::TYPE::WHITESPACE, tokenStr, startpos, m_LineNumber));
			}

			/*can be \n\r or \n or \r\n */
			else if (c == '\n' || c == '\r')
			{
				size_t Pos = index++;

				if (c == '\r' || c == '\n') {}
				else
					index--;

				m_TokenList.push_back(new CToken(CToken::TYPE::NEWLINE, c, Pos, m_LineNumber));

				m_LineNumber++;
			}

			else if (isdigit(c))
			{
				size_t Pos = index;
				char* pEnd;
				auto SubStr = m_Tokenize.substr(index);

				auto Num = std::strtod(SubStr.c_str(), &pEnd);
				auto Length = std::distance(const_cast<char*>(SubStr.c_str()), pEnd);

				index += Length - 1;

				m_TokenList.push_back(new CToken(CToken::TYPE::NUMBER, m_Tokenize.substr(Pos, Length), Pos, m_LineNumber));
			}

			//decimal equivalent in ASCII is 39
			else if (c == '\"' || c == '\'')
			{
				size_t startPos = index;

				char Character = c == '\"' ? '\"' : '\'';

				tokenStr += c;

				while ((c = m_Tokenize[++index]) != Character && index < m_Tokenize.length())
					tokenStr = tokenStr + c;

				if (index == m_Tokenize.length())
					throw std::exception("Parse error while searching for \" or \'");

				tokenStr += c;

				m_TokenList.push_back(new CToken(CToken::TYPE::STRING, tokenStr, startPos, m_LineNumber));
			}

			else if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}')
				m_TokenList.push_back(new CToken(CToken::TYPE::BRACKETS, c, index, m_LineNumber));


			else if (c == '-' || c == '+' || c == '/' || c == '*' || c == '%' || c == '^' || c == '&' || c == '|' || c == '>' || c == '<')
				m_TokenList.push_back(new CToken(CToken::TYPE::BINOP, c, index, m_LineNumber));


			else if (c == '.' || c == ';' || c == ',' || c == '#' || c == '!' || c == ':' || c == '~')
				m_TokenList.push_back(new CToken(CToken::TYPE::DELIMITER, c, index, m_LineNumber));

			else if (c == '=')
				m_TokenList.push_back(new CToken(CToken::TYPE::ASSIGNMENT, c, index, m_LineNumber));


			else if (isalpha(c) || c == '_')
			{
				size_t startPos = index;

				tokenStr += c;

				c = m_Tokenize[++index];

				while (isascii(c) && ((bool)std::isalnum(c) || c == '_'))
				{
					tokenStr += c;
					c = m_Tokenize[++index];
				}

				index--;

				m_TokenList.push_back(new CToken(CToken::TYPE::IDENTIFIER, tokenStr, startPos, m_LineNumber));
			}

			tokenStr.clear();
		}
	}

}