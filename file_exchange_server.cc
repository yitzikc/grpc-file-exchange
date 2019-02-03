#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <map>
#include <cstdint>
#include <stdexcept>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include "sequential_file_writer.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

using fileexchange::FileId;
using fileexchange::FileContent;
using fileexchange::FileExchange;

class FileExchangeImpl final : public FileExchange::Service {
private:
    typedef google::protobuf::int32 FileIdKey;

public:
    FileExchangeImpl() = default;

    Status PutFile(
      ServerContext* context, ServerReader<FileContent>* reader,
      FileId* summary) override
    {
        FileContent contentPart;
        SequentialFileWriter writer;
        while (reader->Read(&contentPart)) {
            try {
                // FIXME: Do something reasonable if a file with a different name but the same ID already exists
                writer.OpenIfNecessary(contentPart.name());
                auto* const data = contentPart.mutable_content();
                writer.Write(*data);

                summary->set_id(contentPart.id());
                // FIXME: Protect from concurrent access by multiple threads
                m_FileIdToName[contentPart.id()] = contentPart.name();
            }
            catch (const std::system_error& ex) {
                const auto status_code = writer.NoSpaceLeft() ? StatusCode::RESOURCE_EXHAUSTED : StatusCode::INTERNAL;
                return Status(status_code, ex.what());
            }
        }

        return Status::OK;
    }

    Status GetFileContent(
        ServerContext* context,
        const FileId* request,
        ServerWriter<FileContent>* writer) override
    {
        return Status::OK;
    }

private:
    std::map<FileIdKey, std::string> m_FileIdToName;
};


void RunServer() {
    // TODO: Allow the port to be customised
  std::string server_address("0.0.0.0:50051");
  FileExchangeImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << ". Press Ctrl-C to end." << std::endl;
  server->Wait();
}

int main(int argc, char** argv)
{
    RunServer();
    return 0;
}
