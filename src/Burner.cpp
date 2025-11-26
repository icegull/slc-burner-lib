#include "Burner.h"
#include <cstring>
#include <cstdio>
#include <vector>
#include <string>

namespace SLC
{
    // Simple 8x16 bitmap font for digits 0-9, ':', '.'
    // Each byte represents a row of 8 pixels. 1 is set, 0 is clear.
    static const uint8_t font8x16[12][16] = {
        // 0
        {0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00, 0x00},
        // 1
        {0x00, 0x08, 0x18, 0x28, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00, 0x00},
        // 2
        {0x00, 0x3C, 0x42, 0x42, 0x02, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x42, 0x42, 0x7E, 0x00, 0x00},
        // 3
        {0x00, 0x3C, 0x42, 0x42, 0x02, 0x02, 0x1C, 0x02, 0x02, 0x02, 0x02, 0x42, 0x42, 0x3C, 0x00, 0x00},
        // 4
        {0x00, 0x04, 0x0C, 0x14, 0x24, 0x24, 0x44, 0x44, 0x7E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00},
        // 5
        {0x00, 0x7E, 0x40, 0x40, 0x40, 0x5C, 0x62, 0x02, 0x02, 0x02, 0x02, 0x42, 0x44, 0x38, 0x00, 0x00},
        // 6
        {0x00, 0x38, 0x44, 0x40, 0x40, 0x5C, 0x62, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00, 0x00},
        // 7
        {0x00, 0x7E, 0x42, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00},
        // 8
        {0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00, 0x00},
        // 9
        {0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x46, 0x3A, 0x02, 0x02, 0x22, 0x1C, 0x00, 0x00},
        // : (index 10)
        {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00},
        // . (index 11)
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00}
    };

    Burner::Burner()
    {
    }

    Burner::~Burner()
    {
    }

    // Helper to get character index
    static int getCharIndex(char c)
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c == ':') return 10;
        if (c == '.') return 11;
        return -1; // Space or unknown
    }

    // UYVY colors
    // Black: Y=16, U=128, V=128
    // White: Y=235, U=128, V=128
    
    static void drawCharUYVY(uint8_t* dst, uint32_t width, uint32_t height, int64_t startX, int64_t startY, int charIdx, int32_t fontSize)
    {
        if (charIdx < 0 || charIdx > 11) return;

        const uint8_t* glyph = font8x16[charIdx];
        
        int targetHeight = fontSize;
        int targetWidth = fontSize / 2;
        if (targetWidth < 1) targetWidth = 1;

        for (int y = 0; y < targetHeight; ++y)
        {
            int64_t drawY = startY + y;
            if (drawY < 0 || drawY >= height) continue;

            int srcY = (y * 16) / targetHeight;
            if (srcY >= 16) srcY = 15;

            uint8_t row = glyph[srcY];
            for (int x = 0; x < targetWidth; ++x)
            {
                int64_t drawX = startX + x;
                if (drawX < 0 || drawX >= width) continue;

                int srcX = (x * 8) / targetWidth;
                if (srcX >= 8) srcX = 7;

                // Check if bit is set (font is MSB first usually, let's assume 0x80 is left-most)
                // My font definition: 0x3C = 00111100. 
                // Let's assume bit 7 is left.
                bool isWhite = (row >> (7 - srcX)) & 1;

                // Calculate pixel index in UYVY buffer
                // UYVY has 2 bytes per pixel.
                // Sequence: U0 Y0 V0 Y1
                // Pixel 0: U0 Y0 V0 (shared)
                // Pixel 1: U0 Y1 V0 (shared)
                
                // Index of the macropixel (pair of pixels)
                uint64_t macroPixelIndex = (drawY * width + drawX) / 2;
                uint64_t byteIndex = macroPixelIndex * 4;
                
                // Are we the even or odd pixel in the pair?
                bool isEven = (drawX % 2) == 0;

                // Ptr to the 4 bytes: U, Y0, V, Y1
                uint8_t* p = dst + byteIndex;

                // Set Y value
                if (isEven) {
                    // Y0 is at offset 1
                    p[1] = isWhite ? 235 : 16;
                } else {
                    // Y1 is at offset 3
                    p[3] = isWhite ? 235 : 16;
                }

                // Set U and V values (shared)
                // We overwrite U and V for both pixels in the pair to be neutral (gray/black/white are all colorless)
                // U=128, V=128
                // Only set this if we want to force the color. 
                // Since we are drawing on a black box, we want the whole box to be black/white.
                // So we should set U and V to 128.
                p[0] = 128; // U
                p[2] = 128; // V
            }
        }
    }

    static void drawCharYUV422(uint8_t* dst, uint32_t width, uint32_t height, int64_t startX, int64_t startY, int charIdx, int32_t fontSize)
    {
        if (charIdx < 0 || charIdx > 11) return;

        const uint8_t* glyph = font8x16[charIdx];
        
        int targetHeight = fontSize;
        int targetWidth = fontSize / 2;
        if (targetWidth < 1) targetWidth = 1;

        // Planar YUV422 offsets
        uint64_t yPlaneSize = (uint64_t)width * height;
        uint64_t uPlaneSize = yPlaneSize / 2;
        
        uint8_t* yPlane = dst;
        uint8_t* uPlane = dst + yPlaneSize;
        uint8_t* vPlane = uPlane + uPlaneSize;

        for (int y = 0; y < targetHeight; ++y)
        {
            int64_t drawY = startY + y;
            if (drawY < 0 || drawY >= height) continue;

            int srcY = (y * 16) / targetHeight;
            if (srcY >= 16) srcY = 15;

            uint8_t row = glyph[srcY];
            for (int x = 0; x < targetWidth; ++x)
            {
                int64_t drawX = startX + x;
                if (drawX < 0 || drawX >= width) continue;

                int srcX = (x * 8) / targetWidth;
                if (srcX >= 8) srcX = 7;

                bool isWhite = (row >> (7 - srcX)) & 1;

                // Set Y value
                yPlane[drawY * width + drawX] = isWhite ? 235 : 16;

                // Set U and V values (shared for every 2 pixels)
                // Only set when we are at an even pixel, or just overwrite for both?
                // Since we iterate pixel by pixel, we can just set it every time or check for even.
                // Checking for even is safer to avoid double writing, but overwriting is fine too.
                // Let's set it for the pair.
                
                uint64_t uvIndex = drawY * (width / 2) + (drawX / 2);
                uPlane[uvIndex] = 128;
                vPlane[uvIndex] = 128;
            }
        }
    }

    static void setV210Word(uint32_t* wordPtr, int componentIndex, uint32_t value)
    {
        // componentIndex: 0, 1, 2
        // value: 10-bit
        uint32_t shift = componentIndex * 10;
        uint32_t mask = 0x3FF << shift;
        *wordPtr = (*wordPtr & ~mask) | ((value & 0x3FF) << shift);
    }

    static void drawCharV210(uint8_t* dst, uint32_t width, uint32_t height, int64_t startX, int64_t startY, int charIdx, int32_t fontSize)
    {
        if (charIdx < 0 || charIdx > 11) return;

        const uint8_t* glyph = font8x16[charIdx];
        
        int targetHeight = fontSize;
        int targetWidth = fontSize / 2;
        if (targetWidth < 1) targetWidth = 1;

        // V210 stride
        // 6 pixels = 16 bytes (4 uint32_t)
        // Stride in bytes = ceil(width / 6) * 16
        uint32_t strideBytes = ((width + 5) / 6) * 16;

        for (int y = 0; y < targetHeight; ++y)
        {
            int64_t drawY = startY + y;
            if (drawY < 0 || drawY >= height) continue;

            int srcY = (y * 16) / targetHeight;
            if (srcY >= 16) srcY = 15;

            uint8_t row = glyph[srcY];
            
            uint8_t* lineStart = dst + drawY * strideBytes;

            for (int x = 0; x < targetWidth; ++x)
            {
                int64_t drawX = startX + x;
                if (drawX < 0 || drawX >= width) continue;

                int srcX = (x * 8) / targetWidth;
                if (srcX >= 8) srcX = 7;

                bool isWhite = (row >> (7 - srcX)) & 1;
                uint16_t yVal = isWhite ? 940 : 64;
                uint16_t uvVal = 512;

                // Calculate V210 location
                uint64_t blockIndex = drawX / 6;
                int pixelInBlock = drawX % 6;
                
                uint32_t* blockPtr = (uint32_t*)(lineStart) + blockIndex * 4;

                // Word 0: Cb0, Y0, Cr0
                // Word 1: Y1, Cb2, Y2
                // Word 2: Cr2, Y3, Cb4
                // Word 3: Y4, Cr4, Y5

                switch (pixelInBlock)
                {
                case 0: // Y0, Cb0, Cr0
                    setV210Word(&blockPtr[0], 1, yVal); // Y0
                    setV210Word(&blockPtr[0], 0, uvVal); // Cb0
                    setV210Word(&blockPtr[0], 2, uvVal); // Cr0
                    break;
                case 1: // Y1. Shares Cb0, Cr0
                    setV210Word(&blockPtr[1], 0, yVal); // Y1
                    setV210Word(&blockPtr[0], 0, uvVal); // Cb0 (Force gray)
                    setV210Word(&blockPtr[0], 2, uvVal); // Cr0 (Force gray)
                    break;
                case 2: // Y2, Cb2, Cr2
                    setV210Word(&blockPtr[1], 2, yVal); // Y2
                    setV210Word(&blockPtr[1], 1, uvVal); // Cb2
                    setV210Word(&blockPtr[2], 0, uvVal); // Cr2
                    break;
                case 3: // Y3. Shares Cb2, Cr2
                    setV210Word(&blockPtr[2], 1, yVal); // Y3
                    setV210Word(&blockPtr[1], 1, uvVal); // Cb2
                    setV210Word(&blockPtr[2], 0, uvVal); // Cr2
                    break;
                case 4: // Y4, Cb4, Cr4
                    setV210Word(&blockPtr[3], 0, yVal); // Y4
                    setV210Word(&blockPtr[2], 2, uvVal); // Cb4
                    setV210Word(&blockPtr[3], 1, uvVal); // Cr4
                    break;
                case 5: // Y5. Shares Cb4, Cr4
                    setV210Word(&blockPtr[3], 2, yVal); // Y5
                    setV210Word(&blockPtr[2], 2, uvVal); // Cb4
                    setV210Word(&blockPtr[3], 1, uvVal); // Cr4
                    break;
                }
            }
        }
    }

    int32_t Burner::init(uint32_t width, uint32_t height, SourceType type, int32_t fontSize)
    {
        m_width = width;
        m_height = height;
        m_type = type;
        m_fontSize = fontSize;
        return 0;
    }

    int32_t Burner::burn(uint8_t *src, uint64_t srcSize, const BurnerTimeId& timeId, int64_t xPos, int64_t yPos)
    {
        return burnCommon(src, srcSize, src, timeId, xPos, yPos);
    }

    int32_t Burner::burn(const uint8_t *src, uint64_t srcSize, uint8_t *dst, const BurnerTimeId& timeId, int64_t xPos, int64_t yPos)
    {
        return burnCommon(src, srcSize, dst, timeId, xPos, yPos);
    }

    int32_t Burner::burnCommon(const uint8_t *src, uint64_t srcSize, uint8_t *dst, const BurnerTimeId& timeId, int64_t xPos, int64_t yPos)
    {
        if (!src || !dst) return -1;
        
        uint32_t width = m_width;
        uint32_t height = m_height;
        
        if (width == 0 || height == 0) return -2; // Not initialized
                
        // Check size
        uint64_t expectedSize = 0;
        if (m_type == SourceType::UYVY || m_type == SourceType::YUV422) {
             expectedSize = (uint64_t)width * height * 2;
        } else if (m_type == SourceType::V210) {
             expectedSize = (uint64_t)((width + 5) / 6) * 16 * height;
        }

        if (srcSize < expectedSize) return -3;
        
        if (src != dst) {
            memcpy(dst, src, expectedSize);
        }

        // 2. Format string
        char buf[64];
        // Format: HH:MM:SS.decimal
        // Assuming decimal is milliseconds or similar, let's just print it as integer.
        snprintf(buf, sizeof(buf), "%02u:%02u:%02u.%u", 
            timeId.hour, timeId.minute, timeId.second, timeId.decimal);

        // 3. Draw string
        // Font is 8x16.
        // We want a black box background.
        // Let's calculate text size.
        size_t len = strlen(buf);
        int charWidth = m_fontSize / 2;
        if (charWidth < 1) charWidth = 1;

        // Draw background box (optional, but requested "black bottom")
        // We can just draw the characters which draw their own background (0 bits are black).
        // My drawChar function sets Y=16 for 0 bits, so it draws the black background for the character cell.
        
        int64_t currentX = xPos;
        for (size_t i = 0; i < len; ++i)
        {
            int idx = getCharIndex(buf[i]);
            if (idx != -1)
            {
                if (m_type == SourceType::UYVY)
                    drawCharUYVY(dst, width, height, currentX, yPos, idx, m_fontSize);
                else if (m_type == SourceType::YUV422)
                    drawCharYUV422(dst, width, height, currentX, yPos, idx, m_fontSize);
                else if (m_type == SourceType::V210)
                    drawCharV210(dst, width, height, currentX, yPos, idx, m_fontSize);
            }
            currentX += charWidth;
        }

        return 0;
    }
}
