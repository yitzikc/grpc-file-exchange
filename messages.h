#pragma once

#include <cstdint>
#include <string>

#include "file_exchange.grpc.pb.h"

fileexchange::FileId MakeFileId(std::int32_t id);
fileexchange::FileContent MakeFileContent(std::int32_t id, std::string name, const void* data, size_t data_len);
