# cloudflare-dns-updater

A lightweight **Dynamic DNS updater** written in C++ for Cloudflare.  
It fetches your current public IP address and automatically updates a DNS record via the Cloudflare API.  

✅ Works reliably on **Linux**  
⚠️ Windows support is **experimental**

---

## ✨ Features

- Fetches public IP from [ipify.org](https://www.ipify.org)  
- Parses JSON responses with [jsoncpp](https://github.com/open-source-parsers/jsoncpp)  
- Updates Cloudflare DNS `A` records automatically when your IP changes  
- Logging to `log.txt` with size-based rotation (max ~60MB)  
- Configurable check interval (default 5 minutes)

---

## 🛠️ Build Instructions

### Linux (tested on Ubuntu 22.04)

```bash
# Install dependencies
sudo apt install g++ cmake libcurl4-openssl-dev libjsoncpp-dev

# Clone and build
git clone https://github.com/beni13312/cloudflare_dns_updater.git
cd cflare-ddns
mkdir build && cd build
cmake ..
make -j4
./cflare_ddns
