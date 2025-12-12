// libhiredis-dev

#include <hiredis/hiredis.h>

#include <memory>
#include <string>

#include "print.h"


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
    RedisConnect(const std::string_view host, int port)
        : host(host), port(port), context(nullptr)
    {
        // must connect explicitly later
    }

    ~RedisConnect() = default;

    bool Connect() {
        context.reset(redisConnect(host.c_str(), port));
        if (!context || context->err) {
            if (context) {
                println("Connection error: ", context->errstr);
                context.reset();
            } else {
                println("Connection error: can't allocate redis context");
            }
            return false;
        }

        println("Connected to Redis at ", host, ":", port);
        return true;
    }

    void Disconnect()
    {
        context.reset();

        println("Disconnected from Redis at ", host, ":", port);
    }

    bool isConnected() const {
        return context != nullptr && context->err == 0;
    }

    std::tuple<std::string, int> GetHost() const {
        return {host, port};
    }


    std::string GetString(const std::string &key)
    {
        std::string value = "";

        if (!isConnected()) {
            println("Not connected to Redis");
            return value;
        }

        redisReply *reply = (redisReply *)redisCommand(context.get(), "GET %s", key.c_str());        
        if (reply != NULL) 
        {
            if (reply->type == REDIS_REPLY_STRING) 
            {
                value = reply->str;
            } 
            else 
            {
                println("Redis key:", key, ",not found or not a string");
            }

            freeReplyObject(reply);
        }
        else 
        {
            println("Failed to execute GET command for key:", key);
        }
        return value;
    }

    // SET, DEL, etc ..
    std::tuple<std::string, int> Query(std::string command, std::string args) 
    {
        std::string result = "";
        int type = 0;  

        if (!isConnected()) {
            println("Not connected to Redis");
            return {result, type};
        }

        redisReply *reply = (redisReply *)redisCommand(context.get(), command.c_str(), args.c_str());
        
        if (reply != NULL) 
        {
            type = reply->type;

            if (type == REDIS_REPLY_STRING) {
                result = reply->str;
            } 
            else if (type == REDIS_REPLY_INTEGER) {
                result = std::to_string(reply->integer);
            } 
            else if (type == REDIS_REPLY_STATUS) {
                result = reply->str;
            }

            freeReplyObject(reply);
        }
        
        return {result, type};
    }
};