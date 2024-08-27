#include "json.h"

#include <assert.h>
#include <fstream>
#include <sstream>



namespace JSON
{
	//static method
	bool JSON::Write(const Value &value, const std::filesystem::path &path)
	{
		if(!(value.is_array() || value.is_object()))
			return false;

		std::ofstream file(path, std::ios::out);
		if (!file.is_open())
			return false;

		file << value;
		file.close();

		return true;
	}

	
	JSON::JSON(std::filesystem::path path)
	{
		if (!std::filesystem::exists(path))
			throw std::exception("File does not exist");

		std::ifstream file(path, std::ios::in);
		if (!file.is_open())  
			throw std::exception("Failed to open file");
	
		for (std::string Line; std::getline(file, Line);)
			m_Content.append(Line);

		file.close();
	}


	Value JSON::Parse(Error& err)
	{
		try {
			Lexer lex(m_Content);
			auto t = lex.cur();

			if(t.type() == t.BRACKET) 
			{
				lex.move(1);
				if(std::get<std::string>(t.value())=="{")
					return ParseObject(lex);
				
				if(std::get<std::string>(t.value())=="[")
					return ParseArray(lex);
			}
		}
		catch(std::exception& e) {
			err.failed = true;
			err.msg = e.what();
		}

		return Value();
	}


	Value JSON::FromObject(Lexer& lex)
	{
		auto t = lex.cur();
		lex.move(1);
		if(t.type() == t.BOOL)
			return std::get<bool>(t.value());

		if(t.type() == t.STR)
			return std::get<std::string>(t.value());
		
		if(t.type() == t.INT)
			return std::get<int>(t.value());

		if(t.type() == t.FLOAT)
			return std::get<double>(t.value());
		
		if(t.type() == t.BRACKET && std::get<std::string>(t.value())=="[")
			return ParseArray(lex);

		if(t.type() == t.BRACKET && std::get<std::string>(t.value())=="{")
			return ParseObject(lex);

		return {};
	}



	Value JSON::ParseObject(Lexer& lex)
	{
		std::unordered_map<std::string, Value> map;

		while(true)
		{
			auto Token = lex.cur();
			if(Token.type() == Token.BRACKET && std::get<std::string>(Token.value())=="}")
			{
				lex.move(1);
				break;
			}

			if(Token.type() == Token.DELIM && std::get<std::string>(Token.value())==",")
			{
				lex.move(1);
				continue;
			}

			if(Token.type()!= Token.STR)
				throw std::exception("Invalid key in object");

			auto key = std::get<std::string>(Token.value());
			lex.move(1);
			auto Column = lex.cur();
			if(Column.type() != Column.DELIM || std::get<std::string>(Column.value())!=":")
				throw std::exception("Object assignments require : sign");

			lex.move(1);
			auto valTok = lex.cur();
			map[key] = FromObject(lex);
		}

		return map;
	}


	Value JSON::ParseArray(Lexer& lex)
	{
		std::vector<Value> Arr;
		
		while(true)
		{
			auto Token = lex.cur();
			if(Token.type() == Token.BRACKET && std::get<std::string>(Token.value())=="]")
			{
				lex.move(1);
				break;
			}

			if(Token.type() == Token.DELIM && std::get<std::string>(Token.value())==",")
			{
				lex.move(1);
				continue;
			}
			
			Arr.push_back(FromObject(lex));
		}

		return Arr;
	}



	/*****************************  OBJECTS ******************************************* */

	Array::Array(const std::vector<Value> &d)
	{
		m_Data = d;
	}

	Array &Array::operator=(const std::vector<Value> &rhs)
	{
		m_Data = rhs;
		return *this;
	}

	Array &Array::operator=(std::vector<Value> &&rhs) noexcept
	{
		m_Data = rhs;
		return *this;
	}

    void Array::push_back(const Value &v)
    {
		m_Data.push_back(v);
    }

    std::ostream &operator<<(std::ostream &os, const Array &arr)
	{
		const auto &data = arr.m_Data;
		os << "[";

		if(data.size() == 0)
			os << "]";
		else
		{
			for (size_t i = 0; i < data.size() - 1; ++i) {
				auto e = data[i];
				os << e << ",";
			}
			os << *data.rbegin();
			os << "]";
		}
		
		return os;
	}


	/**************************************************** */

	Object::Object(const std::unordered_map<std::string, Value> &d)
	{
		m_Data = d;
	}

	Object &Object::operator=(const std::unordered_map<std::string, Value> &rhs)
	{
		m_Data = rhs;
		return *this;
	}

	Object &Object::operator=(std::unordered_map<std::string, Value> &&rhs) noexcept
	{
		m_Data = rhs;
		return *this;
	}

	std::ostream &operator<<(std::ostream &os, const Object &obj)
	{
		const auto &data = obj.m_Data;
		os << "{";
		if(data.size() == 0)
			os << "}";
		
		else
		{
			auto Last = (--obj.m_Data.end());
			
			for (auto iter = std::begin(obj.m_Data); iter != std::end(obj.m_Data); ++iter)
			{
				os << "\"" << (*iter).first << "\""; //key
				os << ":";
				os << (*iter).second;

				if(iter != Last)
					os << ",";
			}
			os << "}";
		}

		return os;
	}


	/************************************** */

	std::ostream &operator<<(std::ostream &os, const Value &v)
	{
		if(v.is_array())
			os << v.as_Array();
		else if(v.is_object())
			os << v.as_Object();
		else if(v.is_bool())
			os << v.as_bool();
		else if(v.is_int())
			os << v.as_int();
		else if(v.is_double())
			os << v.as_double();
		else if(v.is_string())
		{
			auto s = v.as_string();
			s = EscapeQuotes(s);
			os << "\"" << s << "\"";
		}
		else if(v.is_null())
			os << "null";

		return os;
	}



	/******************************** LEXER  *************************************** */

		void Lexer::Tokenize(std::string_view _Text)
	{
		std::string tokenStr;
		size_t len = _Text.length();

		if (_Text.empty()) 
			throw std::exception("Empty string cannot be tokenized.");

		for (size_t index = 0; index < len; ++index)
		{
			char c = _Text[index];

			if (c == 0 || c == '\0')
				break;

			else if (c == ' ' || c == '\t')
				continue;

			else if (isdigit(c))
			{
				char* pEnd;
				auto SubStr = _Text.substr(index);

				auto Num = std::strtod(SubStr.data(), &pEnd);
				auto Length = std::distance(const_cast<char*>(SubStr.data()), pEnd);

				index += Length - 1;

				if(int(Num) == Num)
					m_Tokens.emplace_back(CToken::INT, (int)Num);
				else
					m_Tokens.emplace_back(CToken::FLOAT, Num);
			}

			//decimal equivalent in ASCII is 39
			else if (c == '\"' || c == '\'')
			{
				char Search = c == '\"' ? '\"' : '\'';

				bool Escape = false;
				while (true)
				{
					c = _Text[++index];
					if(c == '\\')
					{
						Escape = true;
						continue;
					}

					if(c == Search && !Escape)
						break;
					
					if(Escape) {
						tokenStr += '\\';
						Escape = false;
					}
					tokenStr += c;

					if (index == _Text.length())
						throw std::exception("Parse error while searching for \" or \'");
				}

				m_Tokens.emplace_back(CToken::STR, tokenStr);
			}

			else if (c == '[' || c == ']' || c == '{' || c == '}')
				m_Tokens.emplace_back(CToken::BRACKET, std::string(1, c));

			else if (c == ',' || c == ':' )
				m_Tokens.emplace_back(CToken::DELIM, std::string(1, c));

			else if (isalpha(c) || c == '_') 
			{
				tokenStr += c;

				c = _Text[++index];
				while (std::isalnum(c) || c == '_') {
					tokenStr += c;
					c = _Text[++index];
				}
				index--;

				if(tokenStr == "true" || tokenStr == "false")
					m_Tokens.emplace_back(CToken::BOOL, tokenStr == "true" ? true : false);

				else if(tokenStr == "null")
					m_Tokens.emplace_back(CToken::NONE);

				else
					m_Tokens.emplace_back(CToken::ID, tokenStr);
			}

			tokenStr.clear();
		}
	}

}