#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <variant>
#include <unordered_map>


namespace 
{
	std::string& EscapeQuotes(std::string& s)
	{
		// Replace all occurrences of '"' with "\\\""
		std::string target = "\"", replcmnt = "\\\""; 

		size_t pos = 0;
		while ((pos = s.find(target, pos)) != std::string::npos) 
		{
			s.replace(pos, target.length(), replcmnt);
			pos += replcmnt.length(); 
		}
		return s;
	}
}




namespace JSON
{
	struct Error
	{
		bool failed{false};
		std::string msg;
	};


	class Lexer;
	class Value;

	class JSON
	{
	public:
		JSON(const std::string& s):m_Content{s} {}
		JSON(std::filesystem::path path);

		//1st: id (converted to uppercase), 2nd: value
		Value Parse(Error& err);

		static bool Write(
				const Value &value, 
				const std::filesystem::path& path);

	private:
		Value ParseArray(Lexer& lex);
		Value ParseObject(Lexer &lex);
		Value FromObject(Lexer &lex);

	private:
		std::string m_Content;
	};



	/************************* Objects  ********************************** */

	template<typename T>
	struct BasicType 
	{
		BasicType(T i=T{}):m_Data{i}{}
		~BasicType() = default;
		
		BasicType(const BasicType& other)
		{
			m_Data = other.m_Data;
		}

		BasicType(BasicType&& other) noexcept
		{
			m_Data = other.m_Data;
		}

		BasicType &operator=(const BasicType &rhs) = default;
		BasicType &operator=(BasicType&& rhs) noexcept = default;

		BasicType& operator=(const T& rhs)
		{
			m_Data = rhs;
			return *this;
		}

		BasicType& operator=(T&& rhs) noexcept
		{
			m_Data = rhs;
			return *this;
		}

		auto data() const{
			return m_Data;
		}

	private:
		T m_Data;
	};

	using Int = BasicType<int>;
	using Double = BasicType<double>;
	using String = BasicType<std::string>;
	using Bool = BasicType<bool>;

	class Value;

	struct Array
	{
		Array() = default;
		Array(const std::vector<Value> &d);
		~Array() = default;

		Array(const Array &other) = default;
		Array(Array&& other) noexcept = default;

		Array &operator=(const Array &rhs) = default;
		Array &operator=(Array &&rhs) noexcept = default;

		Array &operator=(const std::vector<Value> &rhs);
		Array &operator=(std::vector<Value> &&rhs) noexcept;

		friend std::ostream& operator<<(std::ostream& os, const Array& arr);

		void push_back(const Value& v);

		auto& data() const{
			return m_Data;
		}

	private:
		std::vector<Value> m_Data;
	};


	struct Object
	{
		Object(const std::unordered_map<std::string, Value> &d);

		Object(const Object& other)
		{
			m_Data = other.m_Data;
		}

		Object(Object&& other) noexcept
		{
			m_Data = other.m_Data;
		}

		Object &operator=(const Object &rhs) = default;
		Object &operator=(Object &&rhs) noexcept = default;

		Object &operator=(const std::unordered_map<std::string, Value> &rhs);
		Object &operator=(std::unordered_map<std::string, Value> &&rhs) noexcept;

		friend std::ostream& operator<<(std::ostream& os, const Object& obj);

		~Object() = default;

		auto data() const{
			return m_Data;
		}

	protected:
		std::unordered_map<std::string, Value> m_Data;
	};


	class Value
	{
	public:
		Value() = default;
		Value(int i):m_Value{Int(i)}{}
		Value(double d):m_Value{Double(d)}{}
		Value(const std::string& s)
		{
			auto s2 = s;
			s2.erase(std::remove_if(s2.begin(), s2.end(),[](char c) { return c == '\\'; }), s2.end());
			m_Value = String(s2);
		}
		Value(const char* s)
		{
			std::string s2 = s;
			s2.erase(std::remove_if(s2.begin(), s2.end(),[](char c) { return c == '\\'; }), s2.end());
			m_Value = String(s2);
		}
		Value(bool b):m_Value{Bool(b)}{}
		Value(const std::vector<Value>& v):m_Value{Array(v)}{}
		Value(const std::unordered_map<std::string, Value>& m):m_Value{Object(m)}{}

		Value(Int i):m_Value{i}{}
		Value(Double d):m_Value{d}{}
		Value(String s):m_Value{s}{}
		Value(Bool b):m_Value{b}{}
		Value(Array arr):m_Value{arr}{}
		Value(Object o):m_Value{o}{}
		

		Value(const Value& other)
		{
			std::visit([&](auto&& arg)
			{
				 using T = std::decay_t<decltype(arg)>;
				 if constexpr (std::is_same_v<T, Int>)
				 	m_Value = std::get<Int>(other.m_Value);
				 else if constexpr (std::is_same_v<T, Double>)
				 	m_Value = std::get<Double>(other.m_Value);
				 else if constexpr (std::is_same_v<T, String>)
				 	m_Value = std::get<String>(other.m_Value);
				else if constexpr (std::is_same_v<T, Bool>)
				 	m_Value = std::get<Bool>(other.m_Value);
				else if constexpr (std::is_same_v<T, Array>)
				 	m_Value = std::get<Array>(other.m_Value);
				else if constexpr (std::is_same_v<T, Object>)
				 	m_Value = std::get<Object>(other.m_Value);
				}, other.m_Value);
		}

		friend std::ostream& operator<<(std::ostream& os, const Value& v);

		bool is_null() const { return std::holds_alternative<std::monostate>(m_Value); }
		bool is_string() const { return std::holds_alternative<String>(m_Value); }
		bool is_double() const {  return std::holds_alternative<Double>(m_Value); }
		bool is_int() const {  return std::holds_alternative<Int>(m_Value); }
		bool is_bool() const {  return std::holds_alternative<Bool>(m_Value); }
		bool is_array() const {  return std::holds_alternative<Array>(m_Value); }
		bool is_object() const {  return std::holds_alternative<Object>(m_Value); }

		template<typename T>
		auto as() const 
		{ 
			return std::get<T>(m_Value).data(); 
		}

		auto as_string() const { return as<String>(); }
		auto as_double() const { return as<Double>(); }
		auto as_int() const { return as<Int>(); }
		auto as_bool() const { return as<Bool>(); }
		auto as_array() const { return as<Array>(); }
		auto as_object() const { return as<Object>(); }

		template<typename T>
		auto As() const 
		{ 
			return std::get<T>(m_Value); 
		}

		auto as_String() const {return As<String>();}
		auto as_Double() const { return As<Double>(); }
		auto as_Int() const { return As<Int>(); }
		auto as_Bool() const { return As<Bool>(); }
		auto as_Array() const { return As<Array>(); }
		auto as_Object() const { return As<Object>(); }

	private:
		//std::monostate for JSON's null
		using ValueType = std::variant<std::monostate, Int, Double, String, Bool, Array, Object>;
		ValueType m_Value;
	};

	


	/************************* Lexer **************************** */

	class CToken
	{
	public:
		enum TYPE{ INT = 0, FLOAT, ID, NONE, BOOL, BRACKET, DELIM, STR };
		using DTYPE = std::variant<int, double, bool, std::string>;
	public:
		CToken(TYPE type, DTYPE TokenValue = {}) :	m_Type(type),m_Value(TokenValue){}

		auto value() const { return m_Value; }
		TYPE type() const { return m_Type; }

	private:
		TYPE m_Type;

		//value of Token for any kind of Token, can be +,-, primary etc...
		DTYPE m_Value;
	};


	class Lexer
	{
	public:
		Lexer(std::string_view JSONText)
		{
			m_TokenPos = 0;
			Tokenize(JSONText);
		}

		virtual ~Lexer() = default;

		size_t size() const {
			return m_Tokens.size();
		}

		auto cur() const{
			return m_Tokens[m_TokenPos];
		}

		//post-increment
		auto operator++(int) {
			return m_Tokens[m_TokenPos++];
		}

		//post-decrement
		auto operator--(int) {
			return m_Tokens[m_TokenPos--];
		}

		//move lexical position forward or backwards with given step size
		void move(int step=1)
		{
			m_TokenPos += step;
			if(m_TokenPos<0 || m_TokenPos>size())
				throw "Invalid step size.";
		}

		auto begin() {
			return m_Tokens.begin();
		}

		auto end() {
			return m_Tokens.end();
		}


	private:
		void Tokenize(std::string_view JSONText);

	private:
		//tracking token's position in the vector
		size_t m_TokenPos;

		//list of tokens
		std::vector<CToken> m_Tokens;
	};
}