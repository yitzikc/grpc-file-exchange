#pragma once

#include <string>

// Get the basename of the given path
std::string extract_basename(const std::string& path);

// Raise a C++ system_error exception based on the current value of errno
void raise_from_errno(const std::string& user_message);
