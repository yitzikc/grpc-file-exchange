#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <utility>
#include <cassert>
#include <sysexits.h>

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#include "utils.h"
#include "sequential_file_writer.h"
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

    bool PutFile(std::int32_t id, const std::string& filename)
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
            return false;
        }
        else {
            std::cout << "Finished sending file with id " << returnedId.id() << std::endl;
        }

        return true;
    }

    bool GetFileContent(std::int32_t id)
    {
        FileId requestedId;
        FileContent contentPart;
        ClientContext context;
        SequentialFileWriter writer;
        std::string filename;

        requestedId.set_id(id);
        std::unique_ptr<ClientReader<FileContent> > reader(m_stub->GetFileContent(&context, requestedId));
        try {
            while (reader->Read(&contentPart)) {
                assert(contentPart.id() == id);
                filename = contentPart.name();
                writer.OpenIfNecessary(contentPart.name());
                auto* const data = contentPart.mutable_content();
                writer.Write(*data);
            };
            const auto status = reader->Finish();
            if (! status.ok()) {
                std::cerr << "Failed to get the file ";
                if (! filename.empty()) {
                    std::cerr << filename << ' ';
                }
                std::cerr << "with id " << id << ": " << status.error_message() << std::endl;
                return false;
            }
            std::cout << "Finished receiving the file "  << filename << " id: " << id << std::endl;
        }
        catch (const std::system_error& ex) {
            std::cerr << "Failed to receive " << filename << ": " << ex.what();
            return false;
        }

        return true;
    }
private:
    std::unique_ptr<fileexchange::FileExchange::Stub> m_stub;
};

void usage [[ noreturn ]] (const char* prog_name)
{
    std::cerr << "USAGE: " << prog_name << " [put|get] num_id [filename]" << std::endl;
    std::exit(EX_USAGE);
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        usage(argv[0]);
    }

    const std::string verb = argv[1];
    std::int32_t id = -1;
    try {
        id = std::atoi(argv[2]);
    }
    catch (std::invalid_argument) {
        std::cerr << "Invalid Id " << argv[2] << std::endl;
        usage(argv[0]);
    }
    bool succeeded = false;
    FileExchangeClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    if ("put" == verb) {
        if (4 != argc) {
            usage(argv[0]);
        }
        const std::string filename = argv[3];
        succeeded = client.PutFile(id, filename);
    }
    else if ("get" == verb) {
        if (3 != argc) {
            usage(argv[0]);
        }
        succeeded = client.GetFileContent(id);
    }
    else {
        std::cerr << "Unknown verb " << verb << std::endl;
        usage(argv[0]);
    }

    return succeeded ? EX_OK : EX_IOERR;
}
