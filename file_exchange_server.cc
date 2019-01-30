#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <map>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "file_exchange.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

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
        std::ofstream ofs;
        FileContent contentPart;
        while (reader->Read(&contentPart)) {
            if (! ofs.is_open()) {
                try {
                    ofs = OpenFileForWriting(contentPart.id(), contentPart.name());
                }
                catch (const std::system_error& ex) {
                    // TODO: Distignuish various possible errors based on the error code, and return more specific error codes.
                    return Status(grpc::StatusCode::INTERNAL, std::string("Failed to open file: ") + ex.what());
                }
            }

            try {
                WriteData(ofs, contentPart.mutable_content());
            }
            catch (const std::system_error& ex) {
                // FIXME: Check that the name and ID suplied in the current message are same as in the initial message

                std::remove(contentPart.name().c_str());
                m_FileIdToName.erase(contentPart.id());       
                // TODO: Distignuish various possible errors based on the error code, notably disk full, and return more specific error codes.
                return Status(grpc::StatusCode::INTERNAL,  std::string("Error writing to file: ") + ex.what());
            }
        }

        return Status::OK;
    }

private:
    std::ofstream OpenFileForWriting(FileIdKey id, const std::string& name)
    {
        using std::ios_base;
        std::ofstream ofs;
        ofs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        ofs.open(name, ios_base::out | ios_base::trunc | ios_base::binary);

        // FIXME: Do something if a file with a different name but the same ID already exists
        // FIXME: Protect from concurrent access by multiple threads
        m_FileIdToName[id] = name;
        return ofs;
    }

    void WriteData(std::ofstream& ofs, std::string* data)
    {
        // TODO: Write asynchronously instead
        ofs << *data;
        return;
    }

    std::map<FileIdKey, std::string> m_FileIdToName;
};


void RunServer() {
  std::string server_address("0.0.0.0:50051");
  FileExchangeImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv)
{
    RunServer();
    return 0;
}
