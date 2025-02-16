#include "osc_parser.h"

#include <cstring>

#include "driverlog.h"

bool OSCParser::is_little_endian_ = [] {
    uint16_t value = 0x1;
    return *reinterpret_cast<uint8_t*>(&value) == 0x1;
}();

uint32_t OSCParser::ConvertEndianUint32(uint32_t value) {
    if (is_little_endian_) {
        return ((value >> 24) & 0x000000FF) |
               ((value >> 8)  & 0x0000FF00) |
               ((value << 8)  & 0x00FF0000) |
               ((value << 24) & 0xFF000000);
    }
    return value;
}

float OSCParser::ConvertEndianFloat(const char* data) {
    uint32_t temp;
    std::memcpy(&temp, data, sizeof(uint32_t));
    temp = ConvertEndianUint32(temp);
    float result;
    std::memcpy(&result, &temp, sizeof(float));
    return result;
}

OSCParser::ParsedMessage OSCParser::Parse(const char* data, size_t size) {
    ParsedMessage msg;
    
    size_t offset = 0;
    msg.address = ReadString(data, size, offset);

    std::string typeTags = ReadString(data, size, offset);
    
    for (char type : typeTags) {
        switch (type) {
            case 's':
                msg.stringValue = ReadString(data, size, offset);
                break;
            case 'b':
                msg.blob = ReadBlob(data, size, offset);
                break;
            case 'T':
                msg.booleanValue = true;
                break;
            case 'F':
                msg.booleanValue = false;
                break;
            case 'f':
                msg.floats.push_back(ReadFloat(data, size, offset));
                break;
            default:
                DriverLog("Unsupported type: " + type);
        }
    }

    return msg;
}

std::string OSCParser::ReadString(const char* data, size_t size, size_t& offset) {
    std::string result;
    while (offset < size && data[offset] != '\0') {
        result += data[offset++];
    }
    offset++;

    while (offset % 4 != 0 && offset < size) {
        offset++;
    }

    return result;
}

std::vector<uint8_t> OSCParser::ReadBlob(const char* data, size_t size, size_t& offset) {
    if (offset + 4 > size) return {};

    uint32_t blobSize;
    std::memcpy(&blobSize, data + offset, sizeof(uint32_t));
    blobSize = ConvertEndianUint32(blobSize);
    offset += 4;

    if (offset + blobSize > size) return {};
    std::vector<uint8_t> blob(data + offset, data + offset + blobSize);
    offset += blobSize;

    while (offset % 4 != 0 && offset < size) {
        offset++;
    }

    return blob;
}

float OSCParser::ReadFloat(const char* data, size_t size, size_t& offset) {
    if (offset + 4 > size) return 0.0f;
    float value = ConvertEndianFloat(data + offset);
    offset += 4;
    return value;
}
