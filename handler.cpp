#include "resp.h"
#include "KVStore.h"
#include "handler.h"

RedClone::Resp RedClone::executeCommand(const Resp &command, KVStore &store)
{
    const auto &args = command.getData();
    if (args.empty())
    {
        return Resp(RespType::Error, "ERR empty command");
    }

    // first element tells if it's SET, DEL, etc.
    KVStore::Command cmd = KVStore::parseCommand(args[0]);
    string retVal{};
    switch (cmd)
    {
    case KVStore::Command::Ping:
        return Resp(RedClone::RespType::SimpleString, "PONG");
    case KVStore::Command::Set:
        if (args.size() < 3)
        {
            return Resp(RespType::Error, "ERR wrong number of arguments for 'SET'");
        }
        store.set(args[1], args[2]);
        return Resp(RedClone::RespType::SimpleString, "OK");
    case KVStore::Command::Get:
        if (args.size() < 2)
        {
            return Resp(RespType::Error, "ERR wrong number of arguments for 'GET'");
        }

        if (store.get(args[1], retVal))
        {
            return Resp(RedClone::RespType::BulkString, retVal);
        };
        return Resp(RedClone::RespType::BulkString, retVal);
    case KVStore::Command::Del:
        if (args.size() < 2)
        {
            return Resp(RespType::Error, "ERR wrong number of arguments for 'DEL'");
        }
        return Resp(RespType::Integer, store.del(args[1]) ? "(integer) 1" : "(integer) 0");
    default:
        return Resp(RespType::Error, "ERR unknown command '" + args[0] + "'");
    }
    return Resp(RespType::Undef, "This should not happen");
}