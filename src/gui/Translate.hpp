#ifndef TRANSLATE_HPP
#define TRANSLATE_HPP

#include "../../libs/json.hpp"
#include <QString>
#include <fstream>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <deque>

namespace uft
{
	static ::std::string lang = "fr";
	static ::std::string loadedLang = "fr";

	static ::std::optional<::nlohmann::json> jsonLangData;

	static ::std::shared_mutex langMutex;

	// Sets the current language as _lang.
	// Can be any string as long as the corresponding JSON file exists in "data/langs/" from
	// the current directory (in runtime) path.
	inline void SetLang(::std::string const& _lang)
	{
		::std::lock_guard<::std::shared_mutex> ulm(langMutex);
		lang = _lang;
	}

	// stands for translate
	template<typename T>
	T const& t(::std::string const& string) // const reference to prevent copy, hence halving the memory usage for translation only "borrowing" the value.
	{
		{
			// Basically, this block says 'if the JSON has already been loaded in memory, don't
			// load and parse it again, just return the value if it has it'.
			::std::shared_lock<::std::shared_mutex> _slm(langMutex);
			if(jsonLangData && loadedLang == lang)
				if(jsonLangData->contains(string))
					return (*jsonLangData)[string].get_ref<T const&>();
				else
					return string;
		}
		::std::lock_guard<::std::shared_mutex> _lm(langMutex);
		// Updates currently loaded language as reference for future translations
		loadedLang = lang;
		::std::ifstream langFile("data/langs/" + lang + ".json");
		if(!langFile)
			return string;
		// if an error occurs while trying to lock the json data for writing, return fallback value
		jsonLangData = ::nlohmann::json();
		langFile >> *jsonLangData;
		if(jsonLangData->contains(string))
			return (*jsonLangData)[string].get_ref<T const&>();
		// fallback in case nothing's found in the language file
		return string;
	}

	template ::std::string const& t<::std::string const&>(::std::string const& string);
	template<> inline char const * const& t<char const* const&>(::std::string const& string)
	{
		return t<::std::string>(string).c_str();
	}

	inline ::std::string st(::std::string const& string)
	{
		return t<::std::string>(string);
	}

	// Translates string into a QString copy of a single string.
	inline QString qt(::std::string const& string)
	{
		return QString::fromStdString(t<::std::string>(string));
	}
}

#endif
