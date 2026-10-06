#ifndef HANDLER_H
#define HANDLER_H

#include "resp.h"
#include "KVStore.h"

namespace RedClone
{
    Resp executeCommand(const Resp &command, KVStore &store);
}

#endif