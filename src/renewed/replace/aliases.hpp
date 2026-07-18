#include <string>
#include <string_view>
#include <functional>
#include "strings_on_steroids.hpp"

namespace uft
{
	typedef ::std::deque<::std::string> arglist;
	typedef ::std::function<void(::std::string_view)> on_write_function;
}