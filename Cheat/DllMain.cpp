#include "Backend/Backend.hpp"
#include "Utils/Utils.hpp"
#include "Cheat.hpp"

#include <Obfusheader.hpp>

static void PrintLogo()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);

	std::cout << " _____ _               _   _ _   _ _     " << std::endl;
	std::cout << "/  __ \\ |             | | | | | (_) |    " << std::endl;
	std::cout << "| /  \\/ | __ _ _ __   | | | | |_ _| |___ " << std::endl;
	std::cout << "| |   | |/ _` | '_ \\  | | | | __| | / __|" << std::endl;
	std::cout << "| \\__/\\ | (_| | | | | | |_| | |_| | \\__ \\" << std::endl;
	std::cout << " \\____/_|\\__,_|_| |_|  \\___/ \\__|_|_|___/" << std::endl;
	std::cout << std::endl;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);
}
// Callback function to handle the server response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), realsize);
    return realsize;
}
static void MakeCurlRequest()
{
    CURL* curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, OBF("https://voidpg.online/api/clanutil"));
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, OBF("User-Agent: ClanUtil"));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set up a callback function to capture the response
        std::string response_data;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << OBF("CURL request failed: ") << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            // Check if the response contains "NO"
            if (response_data.find("\"status\":\"NO\"") != std::string::npos)
            {
                std::cerr << OBF("Server denied access. Terminating application.") << std::endl;
                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
                exit(0); // Terminate the application
            }
            // Continue execution if status is YES
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cerr << OBF("Failed to initialize CURL") << std::endl;
    }
}



static DWORD WINAPI MainThread(LPVOID param)
{
	Features.Load(); // Load everything
    std::cout << "Calling MakeCurlRequest...\n";
    MakeCurlRequest();

	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	SetConsoleTitleA(OBF("Clan Utils Console"));
	const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	// Print Unicode characters
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	std::wcout.imbue(std::locale("en_US.UTF-8"));

	PrintLogo();
	Cheat::Init();
	printf("Press Right Control to open the menu\n");

	return 0;
}

extern "C" __declspec(dllexport)
BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case 1:
		HANDLE hMainThread = CreateThread(nullptr, 0, LPTHREAD_START_ROUTINE(MainThread), hModule, 0, nullptr);
		if (hMainThread)
			CloseHandle(hMainThread);
		break;
	}

	return TRUE;
}