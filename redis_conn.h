// libhiredis-dev

#include <hiredis/hiredis.h>
#include <iostream>
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

    bool connect() {
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

        //println("Connected to Redis at ", host, ":", port);
        return true;
    }

    void disconnect()
    {
        context.reset();

        println("Disconnected from Redis at ", host, ":", port);
    }

    bool isConnected() const {
        return context != nullptr && context->err == 0;
    }

    std::string getValueOfKey(const std::string &key)
    {
        if (!isConnected()) {
            println("Not connected to Redis");
            return "";
        }

        redisReply *reply = (redisReply *)redisCommand(context.get(), "GET %s", key.c_str());
        std::string value = "";
        
        if (reply != NULL) {
            if (reply->type == REDIS_REPLY_STRING) {
                value = reply->str;
            } else {
                println("Redis key not found or not a string.");
            }
            freeReplyObject(reply);
        }
        
        return value;
    }

    std::string query(const std::string &command) {
        if (!isConnected()) {
            println("Not connected to Redis");
            return "";
        }

        redisReply *reply = (redisReply *)redisCommand(context.get(), command.c_str());
        std::string result = "";
        
        if (reply != NULL) {
            if (reply->type == REDIS_REPLY_STRING) {
                result = reply->str;
            } else if (reply->type == REDIS_REPLY_INTEGER) {
                result = std::to_string(reply->integer);
            } else if (reply->type == REDIS_REPLY_STATUS) {
                result = reply->str;
            }
            freeReplyObject(reply);
        }
        
        return result;
    }
};