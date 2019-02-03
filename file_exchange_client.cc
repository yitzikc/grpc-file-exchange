#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <utility>
#include <sysexits.h>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "utils.h"
#include "file_reader_into_stream.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using fileexchange::FileId;
using fileexchange::FileContent;
using fileexchange::FileExchange;


class FileExchangeClient {
public:
    FileExchangeClient(std::shared_ptr<Channel> channel)
        : m_stub(FileExchange::NewStub(channel))
    {

    }

    void PutFile(std::int32_t id, const std::string& filename)
    {
        FileId returnedId;
        ClientContext context;

        std::unique_ptr<ClientWriter<FileContent>> writer(m_stub->PutFile(&context, &returnedId));
        try {
            FileReaderIntoStream< ClientWriter<FileContent> > reader(filename, id, *writer);

            // TODO: Make the chunk size configurable
            const size_t chunk_size = 1UL << 20;    // Hardcoded to 1MB, which seems to be recommended from experience.
            reader.Read(chunk_size);
        }
        catch (const std::exception& ex) {
            std::cerr << "Failed to send the file " << filename << ": " << ex.what() << std::endl;
            // FIXME: Indicate to the server that something went wrong and that the trasfer should be aborted.
        }

        writer->WritesDone();
        Status status = writer->Finish();
        if (!status.ok()) {
            std::cerr << "File Exchange rpc failed: " << status.error_message() << std::endl;
        }
        else {
            std::cout << "Finished sending file with id " << returnedId.id() << std::endl;
        }

        return;
    }

    void GetFileContent(std::int32_t id)
    {

    }
private:
    std::unique_ptr<fileexchange::FileExchange::Stub> m_stub;
};

int main(int argc, char** argv)
{
    if (4 != argc) {
        std::cerr << "USAGE: " << argv[0] << "[put|get] num_id [filename]" << std::endl;
        return EX_USAGE;
    }

    FileExchangeClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    
    const std::int32_t id = std::atoi(argv[2]);
    const std::string filename = argv[3];
    client.PutFile(id, filename); 
    return 0;
}
