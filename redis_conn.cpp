// libhiredis-dev

#include <hiredis/hiredis.h>

#include <memory>
#include <string>
#include <thread>

#include "print.h"
#include "logger.h"
#include "redis_conn.h"

extern Logger gLogger; // declare external logger instance

RedisConnect::RedisConnect(const std::string_view host, int port)
    : host(host), port(port), context(nullptr)
{
    // must connect explicitly later
}

bool RedisConnect::Connect() 
{
    bool ok = tryConnect();

    if( ! ok ) {
        retryConnect(5, 2); // 5 tries, 2 sec interval
    }

    return true;
}

bool RedisConnect::tryConnect() 
{
    context.reset(redisConnect(host.c_str(), port));

    bool ok = true;

    if (!context || context->err) 
    {
        if (context) 
        {
            gLogger.log("Connection error: ", context->errstr);
            context.reset();
        } 
        else  {
            gLogger.log("Connection error: can't allocate redis context");
        }

        ok = false;
    }

    if( ok ) {
        gLogger.log("Connected to Redis at ", host, ":", port);
    }

    return ok;
}


bool RedisConnect::retryConnect(int maxTries, int intervalSeconds)
{
    int attempts = 0;    
    while (attempts < maxTries)
    {
        attempts++;
        gLogger.log("Auto-connect attempt ", attempts, " of ", maxTries);
        
        if (tryConnect())
        {
            return true;
        }
        
        if (attempts < maxTries)
        {
            gLogger.log("Retrying in ", intervalSeconds, " seconds...");
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        }
    }
    
    gLogger.log("Auto-connect failed after ", maxTries, " attempts");
    return false;
}

void RedisConnect::Disconnect()
{
    context.reset();

    gLogger.log("Disconnected from Redis at ", host, ":", port);
}

bool RedisConnect::isConnected() const {
    return context != nullptr && context->err == 0;
}

std::tuple<std::string, int> RedisConnect::GetHost() const {
    return {host, port};
}


std::string RedisConnect::GetString(const std::string &key, bool log)
{
    std::string value = "";

    if (!isConnected()) 
    {
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

bool RedisConnect::SetString(const std::string &key, const std::string &value)
{
    if (!isConnected()) 
    {
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

bool RedisConnect::Delete(const std::string &key)
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
std::tuple<std::string, int> RedisConnect::Query(std::string command, std::string args) 
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
