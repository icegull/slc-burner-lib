#pragma once
#include <cstdint>

namespace SLC
{
    class Burner
    {
    public:
        Burner();
        ~Burner();

        enum class SourceType
        {
            UYVY,
            YUV422,
            V210
        };

        struct BurnerTimeId
        {
            uint32_t hour = 0;
            uint32_t minute = 0;
            uint32_t second = 0;
            uint32_t decimal = 0;
        };

        int32_t init(uint32_t width, uint32_t height, SourceType type, int32_t fontSize = 64);

        int32_t burn(const uint8_t *src, uint64_t srcSize, uint8_t *dst, const BurnerTimeId& timeId, int64_t xPos, int64_t yPos);

        int32_t burn(uint8_t *src, uint64_t srcSize, const BurnerTimeId& timeId, int64_t xPos, int64_t yPos);        

    private:
        int32_t burnCommon(const uint8_t *src, uint64_t srcSize, uint8_t *dst, const BurnerTimeId& timeId, int64_t xPos, int64_t yPos);

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        SourceType m_type = SourceType::UYVY;
        int32_t m_fontSize = 64;
    };
}