// SPDX-License-Identifier: GPL-2.0-only

#include "Lokal.hpp"

#include <curl/curl.h>
#include <stdexcept>
#include <cstdio>

namespace LokalSo {

using json = nlohmann::json;

constexpr static const char lokal_so_banner[] = 
"    __       _         _             \n"
"   / /  ___ | | ____ _| |  ___  ___  \n"
"  / /  / _ \\| |/ / _  | | / __|/ _ \\ \n"
" / /__| (_) |   < (_| | |_\\__ \\ (_) |\n"
" \\____/\\___/|_|\\_\\__,_|_(_)___/\\___/ \n";

Tunnel::Tunnel(Lokal *lokal):
	lokal_(lokal)
{
}

Tunnel::~Tunnel(void) = default;

std::string Tunnel::create(void)
{
	std::string req_body;
	std::string res_body;
	json j;

	if (address_mdns_.length() == 0 && address_public_.length() == 0)
		throw std::runtime_error("Either LAN or Public address must be set");

	req_body = this->jsonSerialize();
	res_body = lokal_->post("/api/tunnel/start", req_body);

	if (res_body.length() == 0)
		throw std::runtime_error("Failed to create tunnel");

	try {
		j = json::parse(res_body);
	} catch (json::parse_error &e) {
		throw std::runtime_error("Failed to parse response: " + res_body);
	}

	if (startup_banner_)
		__showStartupBanner();

	return res_body;
}

void Tunnel::__showStartupBanner(void)
{
	std::string val;

	printf("%s\n\n", lokal_so_banner);
	printf("Minimum Lokal Client\t%s\n", LOKAL_SERVER_MIN_VERSION);

	if (this->address_public_.length())
		printf("Public Address\t\thttps://%s\n", this->address_public_.c_str());

	val = getLANAddress();
	if (val.length())
		printf("LAN Address\t\thttp://%s\n", val.c_str());

	printf("\n");
}

LOKAL_CONST_DEF_CPP(Lokal::TunnelTypeHTTP);

Lokal::Lokal(std::string base_url):
	base_url_(base_url)
{
	this->curl_pImpl = std::make_unique<CurlImpl>();
}

Lokal::~Lokal(void)
{
}


void curlDeleter(CURL* ptr) {
	curl_easy_cleanup(ptr);
}
void curlListDeleter(curl_slist* ptr) {
	curl_slist_free_all(ptr);
}

using CURL_ptr       = std::unique_ptr<CURL, decltype(&curlDeleter)>;
using curl_slist_ptr = std::unique_ptr<curl_slist, decltype(&curlListDeleter)>;


struct curl_data {
	std::string res_body;
	std::string res_hdr;
	bool version_ok;
};

static size_t res_body_callback(void *contents, size_t size, size_t nmemb,
				void *userp)
{
	struct curl_data *mem = (struct curl_data *)userp;
	size_t full_size = size * nmemb;

	mem->res_body.append((char *)contents, full_size);
	return full_size;
}

static size_t res_hdr_callback(void *contents, size_t size, size_t nmemb,
			       void *userp)
{
	struct curl_data *mem = (struct curl_data *)userp;
	size_t full_size = size * nmemb;

	mem->res_hdr.append((char *)contents, full_size);
	return full_size;
}

class Lokal::CurlImpl {
private:
	CURL_ptr curl_;
public:
	CurlImpl(): curl_(curl_easy_init(), curlDeleter) {}
	
	std::string __curl(std::string const& url, std::string const& method,
					std::string const& req_body = "") {

	struct curl_data data;
	long http_code = 0;
	CURLcode res;
	CURL *ch = curl_.get();

	curl_slist_ptr headers {curl_slist_append(nullptr, "Content-Type: application/json"), curlListDeleter};
	
	curl_easy_setopt(ch, CURLOPT_URL, url.c_str());
	curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, method.c_str());
	curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, &res_body_callback);
	curl_easy_setopt(ch, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(ch, CURLOPT_HEADERFUNCTION, &res_hdr_callback);
	curl_easy_setopt(ch, CURLOPT_HEADERDATA, &data);
	curl_easy_setopt(ch, CURLOPT_USERAGENT, "Lokal Go - github.com/lokal-so/lokal-go");
	curl_easy_setopt(ch, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers.get());
	if (req_body != "") {
		curl_easy_setopt(ch, CURLOPT_POST, 1L);
		curl_easy_setopt(ch, CURLOPT_POSTFIELDSIZE, req_body.size());
		curl_easy_setopt(ch, CURLOPT_POSTFIELDS, req_body.c_str());
	}

	res = curl_easy_perform(ch);
	if (res != CURLE_OK) {
		throw std::runtime_error(curl_easy_strerror(res));
	}

	curl_easy_getinfo(ch, CURLINFO_RESPONSE_CODE, &http_code);
	if (http_code != 200) {
		std::string msg;

		try {
			json j = json::parse(data.res_body);
			msg = j["message"];
		} catch (json::parse_error &e) {
			msg = "";
		}

		throw std::runtime_error("HTTP error: " + std::to_string(http_code) + ": " + msg);
	}

	return data.res_body;
	}
};

std::string Lokal::post(std::string path, std::string data)
{
	std::string url {this->base_url_ + path};
	return this->curl_pImpl->__curl(url, "POST", data);
}


} /* namespace LokalSo */
