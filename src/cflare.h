#ifndef  CFLARE_H
#define  CFLARE_H

#include <string>
#include <json/json.h>

class cflare {
public:
    cflare() = default;
    ~cflare() = default;

    static void run();

private:
    static Json::Value getJson(const std::string &filename);
    static std::string get_public_ip(const std::string& req_url);
    static std::string parse_ip_from_json(const std::string& json_response);
    static std::string list_dns_records(const std::string &api_token, const std::string &zone_id, const std::string &record_name, const std::string &record_type);
    static void update_dns_record(const std::string &api_token, const std::string &zone_id, const std::string &record_id, const std::string &record_name, bool proxy, const std::string &new_ip);



};
#endif //CFLARE_H