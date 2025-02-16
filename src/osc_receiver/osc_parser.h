#pragma once

#include <vector>
#include <string>

class OSCParser {
public:
    struct ParsedMessage {
        std::string address;
        std::vector<float> floats;
        std::vector<uint8_t> blob;
        std::string stringValue;
        bool booleanValue = false;
    };

    static ParsedMessage Parse(const char* data, size_t size);

private:
    static std::string ReadString(const char* data, size_t size, size_t& offset);
    static std::vector<uint8_t> ReadBlob(const char* data, size_t size, size_t& offset);
    static float ReadFloat(const char* data, size_t size, size_t& offset);

    static uint32_t ConvertEndianUint32(uint32_t value);
    static float ConvertEndianFloat(const char* data);

    static bool is_little_endian_;
    static void CheckEndian(); 
};
