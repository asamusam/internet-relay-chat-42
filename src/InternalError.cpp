#include "InternalError.hpp"

std::pair<internal_error_code, std::string> em_data[] = {
	std::make_pair(IEC_BADPASS,    "Invalid password entered: Must be between 8 and 32 alphanumeric characters"),
	std::make_pair(IEC_BADPORTNUM, "Invalid portnumber entered: Must be a number between 1024 and 65535"),
	std::make_pair(IEC_BADARGC,    "Must have two arguments: listening port and server password")
};

std::map<internal_error_code, std::string> InternalError::error_messages(em_data, em_data + sizeof em_data / sizeof em_data[0]);

const std::string& InternalError::get_error_message(internal_error_code iec)
{
	return (error_messages[iec]);
}
