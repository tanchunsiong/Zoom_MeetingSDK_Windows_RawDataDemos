#include <iostream>
#include <locale>
#include <string>
#include <curl/curl.h>
#include <json/json.h>

size_t WriteCallback(char* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(contents, totalSize);
    return totalSize;
}

wchar_t* GetSignatureFromWebService() {
    std::string url = "https://asdc.cc/meeting-csdk";
    std::string postData = ""; // Set your POST data if required

    // Initialize cURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize cURL" << std::endl;
        return nullptr;
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Set the Content-Type header to indicate JSON data
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Failed to perform cURL request: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return nullptr;
    }

    // Clean up cURL
    curl_easy_cleanup(curl);

    // Parse the JSON response
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::istringstream responseStream(response);
    if (!Json::parseFromStream(reader, responseStream, &root, nullptr)) {
        std::cerr << "Failed to parse JSON response" << std::endl;
        return nullptr;
    }

    // Retrieve the value of the "signature" key
    std::string signature = root["signature"].asString();
    std::cout << signature << std::endl;
    // Convert std::string to wchar_t*
    size_t length = signature.size() + 1;
    wchar_t* wcharSignature = new wchar_t[length];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wcharSignature, length, signature.c_str(), _TRUNCATE);
    if (convertedChars == 0) {
        std::cerr << "Failed to convert std::string to wchar_t*" << std::endl;
        delete[] wcharSignature;
        return nullptr;
    }

    return wcharSignature;
}