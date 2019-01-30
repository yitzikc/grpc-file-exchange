#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <string>
#include <cstdint>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
// #include "helper.h"
#include "file_exchange.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using fileexchange::FileId;
using fileexchange::FileContent;
using fileexchange::FileExchange;

FileId MakeFileId(std::int32_t id)
{
    FileId fid;
    fid.set_id(id);
    return fid;
}

FileContent MakeFileContent(std::int32_t id, const std::string& name, const void* data, size_t data_len)
{
    FileContent fc;
    fc.set_id(id);
    fc.set_name(name);
    fc.set_content(data, data_len);
    return fc;
}

class FileExchangeClient {
public:
    FileExchangeClient(std::shared_ptr<Channel> channel)
        : m_stub(FileExchange::NewStub(channel))
    {

    }

    void PutFile(std::int32_t id, const std::string& path)
    {
        FileId returnedId;
        ClientContext context;

        std::unique_ptr<ClientWriter<FileContent>> writer(m_stub->PutFile(&context, &returnedId));

        std::vector<std::uint8_t> dummy_data { 'A', 'B', 'C', '\n' };
        // TODO: Make this a loop
        FileContent fc = MakeFileContent(id, path, dummy_data.data(), dummy_data.size());
        writer->Write(fc);

        writer->WritesDone();
        Status status = writer->Finish();
        if (!status.ok()) {
            std::cerr << "RouteChat rpc failed: " << status.error_message() << std::endl;
        }
        else {
            std::cout << "Finished sending file with id " << returnedId.id() << std::endl;
        }

    }

    void GetFileContent(std::int32_t id)
    {

    }
private:
    std::unique_ptr<fileexchange::FileExchange::Stub> m_stub;
};

int main(int argc, char** argv)
{
    FileExchangeClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    // FIXME: Actually parse CLI

    client.PutFile(1, "dummy.txt"); 
    return 0;
}
