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


    std::string GetString(const std::string &key, bool log = false)
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
                if(log) println("Redis key:", key, ",not found or not a string");
            }

            freeReplyObject(reply);
        }
        else 
        {
            println("Failed to execute GET command for key:", key);
        }
        return value;
    }

    bool SetString(const std::string &key, const std::string &value)
    {
        if (!isConnected()) {
            println("Not connected to Redis");
            return false;
        }

        redisReply *reply = (redisReply *)redisCommand(context.get(), "SET %s %s", key.c_str(), value.c_str());
        bool success = false;
        
        if (reply != NULL) 
        {
            if (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK") {
                success = true;
            }
            freeReplyObject(reply);
        }
        else 
        {
            println("Failed to execute SET command for key:", key);
        }
        
        return success;
    }

    bool Delete(const std::string &key)
    {
        if (!isConnected()) {
            println("Not connected to Redis");
            return false;
        }

        redisReply *reply = (redisReply *)redisCommand(context.get(), "DEL %s", key.c_str());
        bool success = false;
        
        if (reply != NULL) 
        {
            if (reply->type == REDIS_REPLY_INTEGER) {
                success = reply->integer > 0;  // Number of keys deleted
            }
            freeReplyObject(reply);
        }
        else 
        {
            println("Failed to execute DEL command for key:", key);
        }
        
        return success;
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