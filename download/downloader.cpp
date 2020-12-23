#include "downloader.h"

namespace LSW {
	namespace v5 {
		namespace Interface {
			Downloader::~Downloader()
			{
				if (thr.joinable()) thr.join();
			}
			bool Downloader::get(const std::string& url)
			{
#ifdef UNICODE
				HINTERNET connect = InternetOpen(L"LSW Downloader V5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
#else
				HINTERNET connect = InternetOpen("LSW Downloader V5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
#endif
				if (!connect) return false;
				HINTERNET OpenAddress = InternetOpenUrlA(connect, url.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION, 0);
				if (!OpenAddress) return false;


				char DataReceived[download::max_block_size];
				DWORD NumberOfBytesRead = 0;
				TotalBytesRead = 0;

				buf.clear();

				buf.resize(download::max_block_size); // set maximum size

				while (InternetReadFile(OpenAddress, DataReceived, download::max_block_size, &NumberOfBytesRead) && NumberOfBytesRead) {
					buf.resize(TotalBytesRead + NumberOfBytesRead);
					memcpy_s(buf.data() + TotalBytesRead, NumberOfBytesRead, DataReceived, NumberOfBytesRead);
					TotalBytesRead += NumberOfBytesRead;
				}

				InternetCloseHandle(OpenAddress);
				InternetCloseHandle(connect);
				return true;
			}
			bool Downloader::get_async(const std::string& url)
			{
				if (!threadend) return false;
				else if (thr.joinable()) thr.join();
				threadend = false;
				thr = std::thread([&, url] { get(url); threadend = true; });
				return true;
			}
			bool Downloader::ended() const
			{
				return threadend;
			}
			size_t Downloader::bytes_read() const
			{
				return TotalBytesRead;
			}
			const std::string& Downloader::read() const
			{
				return buf;
			}
		}
	}
}
