// libhiredis-dev

#include <hiredis/hiredis.h>

#include <memory>
#include <string>


class RedisConnect {
private:
    struct RedisContextDeleter {
        void operator()(redisContext* ctx) const {
            if (ctx) {
                redisFree(ctx);
            }
        }
    };
    
    std::unique_ptr<redisContext, RedisContextDeleter> context;
    std::string host;
    int port;

public:
    RedisConnect(const std::string_view host, int port);
    ~RedisConnect() = default;

    bool Connect();
    void Disconnect();
    bool isConnected() const;
    std::tuple<std::string, int> GetHost() const; // host, port

    std::string GetString(const std::string &key, bool log = false); // GET
    bool SetString(const std::string &key, const std::string &value); // SET
    bool Delete(const std::string &key); // DEL    
    std::tuple<std::string, int> Query(std::string command, std::string args); // Generic command
};