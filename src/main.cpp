// using namespace uft;

#include "server/Server.hpp"

#include "curlpp/cURLpp.hpp"

struct
{
	::std::string port = "6767";
} AppConfig;

void treat_args(::std::map<::std::string const, ::std::string const> const& args)
{
	const ::std::map<::std::string, ::std::string&> argToRef =
	{
		{ "--port", AppConfig.port },
		{ "-p", AppConfig.port },
	};
	auto setIfGiven = [&args, argToRef](::std::string const& arg) -> void
	{
		if(args.contains(arg) && argToRef.contains(arg))
			argToRef.at(arg) = args.at(arg);
	};
	for(auto const& arg : args)
		setIfGiven(arg.first);
}

int main(int argc, char *argv[]) {
	auto start = ::std::chrono::steady_clock::now();
	::std::map<::std::string const, ::std::string const> args;
	for(int i = 1; i < argc - 1; i+=2)
		args.emplace(::std::string(argv[i]), ::std::string(argv[i+1]));
	treat_args(args);
	int errcode = 0;
	::std::cout << "Ensuring ADB server is running..." << ::std::endl;
	::uft::Tools::Flash::EnsureADB();
	::std::cout << "Initializing curlpp to make requests on packages..." << ::std::endl;
	curlpp::initialize();
	::std::cout << "Preparing server..." << ::std::endl;
	auto server = ::uft::server::Server::GetInstance(::std::stoi(AppConfig.port));
	auto prepared = ::std::chrono::steady_clock::now();
	::std::cout << "Done in " << ::std::chrono::duration_cast<::std::chrono::milliseconds>(prepared - start) << " !" << ::std::endl;
	::std::cout << "Server now running on port " << server.GetPort() << ::std::endl;
	server.Run();
	auto exited = ::std::chrono::steady_clock::now();
	::std::cout << "Shutting down, ran for " << ::std::chrono::duration_cast<::std::chrono::minutes>(exited - prepared) << " !" << ::std::endl;
	::uft::Tools::ToolHandler::Free();
	return errcode;
}
