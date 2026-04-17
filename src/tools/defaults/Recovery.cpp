#include "Recovery.hpp"

namespace uft::Tools
{
	// ::std::map<::std::string const, Recovery> Recovery::CachedRecoveryImages{};

	Recovery::Recovery(Tool source) : Tool{source}
	{
		Type = RECOVERY;
	}
	
	Recovery Recovery::OrangeFox(::std::string const& _deviceCodeName)
	{
		Recovery OrangeFox;
		// Load 'em supported devices
		::nlohmann::json const rawDevice =
			::nlohmann::json::parse(
				HttpGet("https://api.orangefox.download/releases?codename=" + _deviceCodeName)
			)["data"];
		::nlohmann::json device;
		if(rawDevice.is_array())
			device = rawDevice.front();

		OrangeFox.TargetDevice = _deviceCodeName;
		OrangeFox.SourceType = SOURCE_TYPE::ARCHIVE;
		OrangeFox.Source = device["mirrors"].front(); // It will be the Source URL for future downloads of this precise image
		OrangeFox.Type = TOOL_TYPE::RECOVERY;
		OrangeFox.Name = "OrangeFox for " + _deviceCodeName;
		
		// CachedRecoveryImages.emplace(_deviceCodeName, OrangeFox);
		return OrangeFox;
	}

	Recovery Recovery::PitchBlack(::std::string const& _deviceCodename)
	{
		Recovery PitchBlack;
		::std::string const url = repository::SourceForge::GetURLFromProjectName(
			"pbrp"
		);
		::std::string const rss = HttpGet(
			repository::SourceForge::GetRSSForURL(
				url, "/" + _deviceCodename
			)
		);
		PitchBlack.TargetDevice = _deviceCodename;
		PitchBlack.SourceType = SOURCE_TYPE::ARCHIVE;
		PitchBlack.Type = TOOL_TYPE::RECOVERY;
		PitchBlack.Name = "PitchBlack Recovery for " + _deviceCodename;
		PitchBlack.Source = repository::SourceForge::LastReleaseUri(rss);

		return PitchBlack;
	}

	::std::optional<::std::string> Recovery::GetImageFromArchive() const
	{
		auto toolPath = Origin->GetToolPath(*this); // Ensures the tool is downloaded
		if(!toolPath || !::std::filesystem::exists(*toolPath))
			return ::std::nullopt;
		QString targetPath = QString::fromStdString(*toolPath).replace(".zip", ".img");
		if(::std::filesystem::exists(targetPath.toStdString()))
			return targetPath.toStdString();
		// At this point, we basically know the tool is downloaded, and is not extracted
		struct archive* _recovery;
		_recovery = archive_read_new();
		archive_read_support_filter_all(_recovery);
		archive_read_support_format_all(_recovery);
		struct archive_entry* _recovery_entry;
		int result;
		result = archive_read_open_filename(_recovery, toolPath->c_str(), 16384);
		if(result != ARCHIVE_OK)
			return ::std::nullopt;
		// Vibe coded from here to the end of the method, I'm sorry guys
		while (archive_read_next_header(_recovery, &_recovery_entry) == ARCHIVE_OK) {
			const char* currentFile = archive_entry_pathname(_recovery_entry);
			if (QString(currentFile).endsWith(".img"))
			{
				QString targetPath = QString::fromStdString(*toolPath).replace(".zip", ".img");
				FILE* out = fopen(targetPath.toLocal8Bit().constData(), "wb");
				if (out)
				{
					const void* buff;
					size_t size;
					la_int64_t offset;
					
					while (archive_read_data_block(_recovery, &buff, &size, &offset) == ARCHIVE_OK)
						fwrite(buff, 1, size, out);
					fclose(out);
					
					archive_read_free(_recovery);
					return targetPath.toStdString();
				}
				else
					return ::std::nullopt;
			}
		}
		return ::std::nullopt;
	}

	bool Recovery::Flash(QTextEdit* log) const
	{
		if(!ArchiveName || !TargetDevice)
			return false;
		::std::optional<::std::string> imagePath = GetImageFromArchive();
		if(!imagePath)
			return false;
		::std::string const output = Tools::Flash::FastBoot::Flash(Flash::PARTITION::RECOVERY, *imagePath, log);
		return ::uft::Platform::CheckForCommandExecution(
			output
		);
	}
}