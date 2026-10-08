#include "resp.h"

RedClone::Resp::Resp(RespType type, string val) : type(type)
{
    // std::move(val) casts it to an xvalue (rvalue), which lets
    // data.push_back() "move" its heap buffer to data instead of copying it.
    if (!val.empty())
        data.push_back(std::move(val));
}

// this will always be an array
RedClone::Resp::Resp(vector<string> items) : type(RespType::Array), data(std::move(items)) {}

RedClone::Resp RedClone::Resp::fromCli(const string &cliInput)
{
    return Resp(tokenize(cliInput));
}

string RedClone::Resp::serialize() const
{
    string res{};
    if (this->type == RespType::BulkString)
    {
        if (this->data.empty())
        {
            res = "$-1\r\n"; // Null Bulk String
        }
        else
        {
            res = "$" + to_string(this->data[0].size()) + "\r\n" + this->data[0] + "\r\n";
        }
    }
    else if (this->type == RespType::Array)
    {
        res.append("*");
        res.append(to_string(this->data.size()));
        res.append("\r\n");
        for (const string &e : this->data)
        {
            res.append("$");
            res.append(to_string(e.size()));
            res.append("\r\n");
            res.append(e);
            res.append("\r\n");
        }
    }
    else if (this->type == RespType::SimpleString)
    {
        res = "+" + (this->data.empty() ? "" : this->data[0]) + "\r\n";
    }
    else if (this->type == RespType::Error)
    {
        res = "-" + (this->data.empty() ? "" : this->data[0]) + "\r\n";
    }
    else if (this->type == RespType::Integer)
    {
        res = ":" + (this->data.empty() ? "0" : this->data[0]) + "\r\n";
    }

    // --- DEBUG PRINT ---
    cout << "[DEBUG Serialize]: ";
    for (char c : res)
    {
        if (c == '\r')
            cout << "\\r";
        else if (c == '\n')
            cout << "\\n";
        else
            cout << c;
    }
    cout << endl;
    
    return res;
}

vector<string> RedClone::Resp::tokenize(const string &msg)
{
    vector<std::string> res;
    string current;
    bool inQuotes = false;
    char quoteChar = '\0';
    for (size_t i = 0; i < msg.size(); ++i)
    {
        char c = msg[i];
        // start of quoted statement
        if (c == '"' && !inQuotes)
        {
            // flip switch
            inQuotes = !inQuotes;
            quoteChar = c; // Remember if open quote was " or '
        }
        // condition for end of quoted statement
        else if (c == quoteChar && inQuotes)
        {
            // reset
            inQuotes = false;
            quoteChar = '\0';
        }
        // check if space and outside of quotes
        else if (std::isspace(static_cast<unsigned char>(c)) && !inQuotes)
        {
            // add token to vector
            if (!current.empty())
            {
                res.push_back(current);
                // empty for next token
                current.clear();
            }
        }
        else
        {
            // add characters to token
            current += c;
        }
    }
    // add last token
    if (!current.empty())
    {
        res.push_back(current);
    }

    // --- DEBUG PRINT ---
    cout << "[DEBUG Tokenize]: ";
    for (size_t i = 0; i < res.size(); ++i)
    {
        cout << "[" << i << "]:\"" << res[i] << "\"" << (i + 1 < res.size() ? ", " : "");
    }
    cout << endl;

    return res;
}

bool RedClone::Resp::parser(string &buffer, Resp &outResp)
{
    // early return in case of empty string
    if (buffer.empty())
    {
        return false;
    }

    char typeByte = buffer[0];
    size_t crlf = findCRLF(buffer, 0);
    if (crlf == std::string::npos)
    {
        return false; // Incomplete line
    }

    // Simple Strings "+" ("+OK\r\n"), Errors (-), Integers (:)
    if (typeByte == '+' || typeByte == '-' || typeByte == ':'){

        // skips the type char, second arg of substr is length; not end idx
        std::string line = buffer.substr(1, crlf - 1);
        
        // set type
        outResp.type = static_cast<RespType>(typeByte);
        outResp.data = {line};
        buffer.erase(0, crlf + 2); // Consume frame
        return true;
    }

    // Bulk Strings "$" ("$6\r\nfoobar\r\n")
    if (typeByte == '$')
    {
        int length = std::stoi(buffer.substr(1, crlf - 1));
        
        // null bulk strings
        // this happens when the server can't execute the client command and sends an error
        if (length == -1)
        {
            outResp.type = RespType::BulkString;
            outResp.data.clear();
            buffer.erase(0, crlf + 2);
            return true;
        }

        // Header ("$6\r\n"): crlf + 2
        // Payload: length
        // Trailing delimiter ("$...\r\n"): + 2
        size_t totalLen = crlf + 2 + length + 2;

        // Payload not fully received yet
        if (buffer.size() < totalLen)
        {
            return false;
        }

        // extract payload, +2 skips \r\n
        std::string payload = buffer.substr(crlf + 2, length);
        outResp.type = RespType::BulkString;
        outResp.data = {payload};
        
        // removes ingested string from buffer
        buffer.erase(0, totalLen);
        return true;
    }

    // Arrays "*" ("*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n")
    if (typeByte == '*')
    {
        std::string tempBuf = buffer.substr(crlf + 2);
        std::vector<std::string> elements;

        int arrLen = std::stoi(buffer.substr(1, crlf - 1));
        for (int i = 0; i < arrLen; ++i)
        {
            Resp elem;
            // reads each bulk string in array
            if (!parser(tempBuf, elem))
            {
                return false; // Child element is incomplete
            }
            // Command arguments inside array are bulk string payloads
            if (!elem.data.empty())
            {
                elements.push_back(elem.data[0]);
            }
        }
        
        // store consumed bytes if all elements parsed successfully
        size_t consumed = buffer.size() - tempBuf.size();
        buffer.erase(0, consumed);

        outResp.type = RespType::Array;
        outResp.data = elements;
        return true;
    }
    return false;
}

//  returns start of CRLF
size_t RedClone::Resp::findCRLF(const string &buf, size_t pos)
{
    return buf.find("\r\n", pos);
}

const vector<string> &RedClone::Resp::getData() const
{
    return this->data;
}

RedClone::RespType RedClone::Resp::getType() const
{
    return RespType(this->type);
}
