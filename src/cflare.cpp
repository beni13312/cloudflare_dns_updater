#include "cflare.h"
#include <iostream>
#include <string>
#include <curl/curl.h>
#include "Logger.h"
#include <chrono>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <cerrno>

Json::Value cflare::getJson(const std::string &filename){
    Json::Value jsondata;
    std::ifstream file(filename, std::ifstream::binary);
    if (!file.is_open()) {
        logger().message("Failed to open file: " + filename, ERR0R);
        throw std::runtime_error("Failed to open file: " + filename);
    }
    file >> jsondata;
    file.close();
    return jsondata;


}
// global init curl
struct CurlGlobalGuard {
    CurlGlobalGuard()  { curl_global_init(CURL_GLOBAL_DEFAULT); }
    ~CurlGlobalGuard() { curl_global_cleanup(); }
};
static void ensure_curl_global() {
    static CurlGlobalGuard guard; // init once, cleanup at process exit
}

// Callback function to handle the data received from the server
std::size_t WriteCallback(void* contents, std::size_t size, std::size_t nmemb, void* userp) {
    auto* str = static_cast<std::string*>(userp);
    try {
        str->append(static_cast<char*>(contents), size * nmemb);
    } catch (const std::exception& e) {
        std::cerr << "WriteCallback exception: " << e.what() << std::endl;
    }
    return size * nmemb;
}

// Function to get the public IP address
std::string cflare::get_public_ip(const std::string& req_url) {
    ensure_curl_global();

    std::string ip_address;

    if(CURL *curl = curl_easy_init()) {
        curl_easy_setopt(curl, CURLOPT_URL, req_url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ip_address);

        if(const CURLcode res = curl_easy_perform(curl); res != CURLE_OK) {
            logger().message("curl_easy_perm failed: " + std::string(curl_easy_strerror(res)), ERR0R);
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }

    return ip_address;
}

// Function to parse the JSON response to extract IP address
std::string cflare::parse_ip_from_json(const std::string& json_response) {
    Json::Value root;
    std::istringstream s(json_response);
    std::string errs;

    if (const Json::CharReaderBuilder reader_builder; Json::parseFromStream(reader_builder, s, &root, &errs)) {
        return root["ip"].asString();
    }
        std::cerr << "Failed to parse JSON: " << errs << std::endl;
        return "";

}

// List DNS records
std::string cflare::list_dns_records(const std::string &api_token, const std::string &zone_id, const std::string &record_name, const std::string &record_type) {
    ensure_curl_global();

    std::string response;
    std::string record_id;

    if(CURL* curl = curl_easy_init()){
        CURLcode res;
        std::string url = "https://api.cloudflare.com/client/v4/zones/" + zone_id + "/dns_records";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		// Headers
        curl_slist *headers = nullptr;
        const std::string auth_bearer = "Authorization: Bearer " + api_token;
        headers = curl_slist_append(headers, auth_bearer.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK){
            logger().message("curl_easy_perform() failed: " + static_cast<std::string>(curl_easy_strerror(res)), ERR0R);
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }else{
            std::istringstream root(response);
            Json::Value data;
            std::string errs;

            if(Json::CharReaderBuilder reader; Json::parseFromStream(reader, root, &data, &errs)){
                for(const auto &record: data["result"]){
                    if(record["name"].asString() == record_name && record["type"].asString() == record_type){
                        std::cout << "Record ID: " << record["id"].asString() << std::endl;
                        record_id = record["id"].asString();
                        std::cout << "Record Name: " << record["name"].asString() << std::endl;
                        std::cout << "Record Type: " << record["type"].asString() << std::endl;
                    }
                }
            }
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}
return record_id;
}

// Update dns recrods with cloudflare API 
void cflare::update_dns_record(const std::string &api_token, const std::string &zone_id, const std::string &record_id, const std::string &record_name, const bool proxy, const std::string &new_ip) {
    ensure_curl_global();

    std::string response;

    if(CURL *curl = curl_easy_init()) {
        const std::string url = "https://api.cloudflare.com/client/v4/zones/" + zone_id + "/dns_records/" + record_id;

        // Create the JSON payload for updating the DNS record
        Json::Value json_payload;
        json_payload["type"] = "A";
        json_payload["name"] = record_name;
        json_payload["content"] = new_ip;
        json_payload["ttl"] = 120;
        json_payload["proxied"] = proxy;

        const Json::StreamWriterBuilder writer;
        const std::string payload = Json::writeString(writer, json_payload);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		
		// Headers
        curl_slist *headers = nullptr;
        const std::string auth_bearer = "Authorization: Bearer " + api_token;
        headers = curl_slist_append(headers, auth_bearer.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        if(const CURLcode res = curl_easy_perform(curl); res != CURLE_OK) {
            logger().message("curl_easy_perform() failed: " + static_cast<std::string>(curl_easy_strerror(res)), ERR0R);
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            logger().message("DNS record updated successfully. Response: "+response, INFO);
            std::cout << "DNS record updated successfully. Response: " << response << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

void cflare::run() {
    Json::Value jsondata = getJson("cflare.json");
    const std::string api_token = jsondata["api_token"].asString();
    const std::string zone_id = jsondata["zone_id"].asString();
    const std::string record_name = jsondata["record_name"].asString();
    const std::string record_type = jsondata["record_type"].asString();
    const bool proxy = jsondata["proxy"].asBool();

    const std::string ip_req_url = "https://api.ipify.org?format=json"; // Ipfy API
    static std::string static_ip; // Store IP address

    while(true){
        // Get public IP address
        std::string json_response = get_public_ip(ip_req_url);
        if (!json_response.empty()) {

            // Parse IP address from JSON response
            std::string ip_address = parse_ip_from_json(json_response);

            if (!ip_address.empty()) {
                if(ip_address != static_ip) { // If the fetched ip address does not equal to the static current ip
                    static_ip = ip_address; // Set the new ip address in the var
                    std::string record_id = list_dns_records(api_token, zone_id, record_name, record_type);
                    update_dns_record(api_token, zone_id, record_id, record_name, proxy, static_ip); // Update cliudflare DNS record with the new ip address

                    logger().message("Public IP address changed: " + ip_address, INFO);
                    std::cout << "Public IP Address changed: " << ip_address << std::endl;
                }
            } else {
                logger().message("Failed to extract IP address from JSON.", ERR0R);
                std::cerr << "Failed to extract IP address from JSON." << std::endl;
            }
        } else {
            logger().message("Failed to get public IP address.", ERR0R);
            std::cerr << "Failed to get public IP address." << std::endl;
        }
        // Sleep, default is 5 min
        int minutes = std::stoi(jsondata["check_ip_min"].asString());
        if (minutes <= 0) minutes = 5;
        std::this_thread::sleep_for(std::chrono::minutes(minutes));
    }
}
