#include <sstream>
#include <stdexcept>

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

void raise_from_errno(const std::string& user_message)
{
    std::ostringstream sts;
    if (user_message.size() > 0) {
        sts << user_message << ' ';
    }

    const int err = errno;
    sts << strerror(err);
    throw std::system_error(std::error_code(err, std::system_category()), sts.str().c_str());
}
