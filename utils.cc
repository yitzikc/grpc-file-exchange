#include <sstream>
#include <stdexcept>
#include <cassert>

#include <limits.h>
#include <libgen.h>
#include <string.h>

#include "utils.h"

std::string extract_basename(const std::string& path)
{
    char result[PATH_MAX] = "";
    basename_r(path.c_str(), result);
    // TODO: Check for errors

    return std::string(result);
}

void raise_from_system_error_code(const std::string& user_message, int err)
{
    std::ostringstream sts;
    if (user_message.size() > 0) {
        sts << user_message << ' ';
    }

    assert(0 != err);
    throw std::system_error(std::error_code(err, std::system_category()), sts.str().c_str());
}

void raise_from_errno(const std::string& user_message)
{
    raise_from_system_error_code(user_message, errno);
}
