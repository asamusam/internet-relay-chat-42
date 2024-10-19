#include "InternalError.hpp"

std::map<internal_error_code, std::string> InternalError::error_messages;

InternalError::InternalError(void)
{
	error_messages[IEC_BADPASS] = "Invalid password entered: Must be between 8 and 32 alphanumeric characters.";
	error_messages[IEC_BADPORTNUM] = "Invalid portnumber entered: Must be a number between 1024 and 65535";
}

const std::string& InternalError::get_error_message(internal_error_code iec)
{
	return (error_messages[iec]);
}
