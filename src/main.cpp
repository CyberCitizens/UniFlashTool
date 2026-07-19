// using namespace uft;

#include "server/Server.hpp"

#include "curlpp/cURLpp.hpp"
int main(int argc, char *argv[]) {
	int errcode = 0;
	::uft::Tools::Flash::EnsureADB();
	::uft::Tools::Flash::SetAdbPort(6520);
	curlpp::initialize();
	auto server = ::uft::server::Server::GetInstance();
	server.Run();
	::uft::Tools::ToolHandler::Free();
	return errcode;
}
