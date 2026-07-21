// using namespace uft;

#include "server/Server.hpp"

#include "curlpp/cURLpp.hpp"
int main(int argc, char *argv[]) {
	auto start = ::std::chrono::steady_clock::now();
	int errcode = 0;
	::std::cout << "Ensuring ADB server is running..." << ::std::endl;
	::uft::Tools::Flash::EnsureADB();
	::std::cout << "Initializing curlpp to make requests on packages..." << ::std::endl;
	curlpp::initialize();
	::std::cout << "Preparing server..." << ::std::endl;
	auto server = ::uft::server::Server::GetInstance();
	auto prepared = ::std::chrono::steady_clock::now();
	::std::cout << "Done in " << ::std::chrono::duration_cast<::std::chrono::milliseconds>(prepared - start) << " !" << ::std::endl;
	server.Run();
	auto exited = ::std::chrono::steady_clock::now();
	::std::cout << "Shutting down, ran for " << ::std::chrono::duration_cast<::std::chrono::minutes>(exited - prepared) << " !" << ::std::endl;
	::uft::Tools::ToolHandler::Free();
	return errcode;
}
