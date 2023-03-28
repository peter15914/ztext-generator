#pragma once


//------------------------------------------------------------------------------------------
namespace zCurlUtl
{
	struct FtpFile
	{
		const char *filename;
		FILE *stream;
	};

	size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream);
	extern const std::string USER_AGENT;
	//extern const std::string WORK_FOLDER;
	//extern const std::string COOKIE_FILE;
	extern const int TIME_OUT;
};




//------------------------------------------------------------------------------------------
class zPageDownloader
{
public:
	zPageDownloader();
	virtual ~zPageDownloader();

	bool get_UrlToFile(std::string sUrl, std::string sFileName);
};

