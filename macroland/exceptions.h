#pragma once

#include <exception>
#include <string>


namespace exceptions
{
	class PyPkgMissingException : public std::exception
	{
	public:
		explicit PyPkgMissingException(const char* message)
			: msg_(message) {}

		explicit PyPkgMissingException(const std::string& message)
			: msg_(message) {}

		virtual ~PyPkgMissingException() noexcept {}

		virtual const char* what() const noexcept {
		return msg_.c_str();
		}

	protected:
		std::string msg_;
	};
}