#ifndef INTERNAL_ERROR_HPP
# define INTERNAL_ERROR_HPP

#include <map>
#include <string>

enum internal_error_code
{
	IEC_NONE = 0,
	IEC_BADPASS,
	IEC_BADPORTNUM,
	IEC_BADARGC
};

class InternalError
{
	private:
		static std::map<internal_error_code, std::string> error_messages;
	public:
		static const std::string& get_error_message(internal_error_code);
};

#endif /* INTERNAL_ERROR_HPP */
