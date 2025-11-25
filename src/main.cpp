#include <iostream>
#include <vector>
#include <fstream>
#include "Burner.h"
#include <chrono>

class ClockTimer
{
public:
	void reset()
	{
		m_timer = clock::now();
	}

	double elapse_ms() const
	{
		return duration(clock::now() - m_timer).count();
	}
private:
	using timer = std::chrono::time_point<std::chrono::high_resolution_clock>;
	using clock = std::chrono::high_resolution_clock;
	using duration = std::chrono::duration<double, std::milli>;
	timer m_timer = clock::now();
};

void testBurner()
{
    SLC::Burner burner;
    uint32_t width = 1920;
    uint32_t height = 1080;
    
    burner.init(width, height, SLC::Burner::SourceType::UYVY);

    // Create a gray image (Y=128, U=128, V=128)
    // UYVY: U=128, Y=128, V=128, Y=128 -> 0x80, 0x80, 0x80, 0x80
    size_t frameSize = width * height * 2;
    std::vector<uint8_t> src(frameSize, 0x80);
    std::vector<uint8_t> dst(frameSize);

    SLC::Burner::BurnerTimeId timeId;
    timeId.hour = 12;
    timeId.minute = 34;
    timeId.second = 56;
    timeId.decimal = 12800;

    // Burn at position (100, 100)
    ClockTimer timer;
    for (int i = 0; i < 100; ++i)
    {
        timer.reset();

        //burner.burn(src.data(), src.size(), dst.data(), timeId, 100, 100);
        burner.burn(src.data(), src.size(), timeId, 100, 100);
        timeId.decimal += 100;

        double elapsed = timer.elapse_ms();
        std::cout << "Burn time: " << elapsed << " ms" << std::endl;
    }

    // Save to file
    std::ofstream outFile("output_1920x1080.yuv", std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(src.data()), src.size());
    outFile.close();

    std::cout << "Burn test completed. Output saved to output_1920x1080.yuv" << std::endl;
}

void testBurnerV210()
{
    SLC::Burner burner;
    uint32_t width = 1920;
    uint32_t height = 1080;
    
    burner.init(width, height, SLC::Burner::SourceType::V210);

    // V210 size
    // 1920 / 6 = 320 blocks.
    // 320 * 16 bytes = 5120 bytes per line.
    size_t stride = ((width + 5) / 6) * 16;
    size_t frameSize = stride * height;
    std::vector<uint8_t> src(frameSize);
    
    // Fill with gray (Y=512, U=512, V=512)
    // 0x200 | (0x200 << 10) | (0x200 << 20) = 0x20080200
    uint32_t* p = reinterpret_cast<uint32_t*>(src.data());
    size_t numWords = frameSize / 4;
    for (size_t i = 0; i < numWords; ++i)
    {
        p[i] = 0x20080200;
    }

    SLC::Burner::BurnerTimeId timeId;
    timeId.hour = 12;
    timeId.minute = 34;
    timeId.second = 56;
    timeId.decimal = 12800;
    // Burn at position (100, 100)
    ClockTimer timer;
    for (int i = 0; i < 100; ++i)
    {
        timer.reset();

        burner.burn(src.data(), src.size(), timeId, 100, 100);
        timeId.decimal += 100;

        double elapsed = timer.elapse_ms();
        std::cout << "Burn V210 time: " << elapsed << " ms" << std::endl;
    }

    // Save to file
    std::ofstream outFile("output_1920x1080.v210", std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(src.data()), src.size());
    outFile.close();

    std::cout << "Burn V210 test completed. Output saved to output_1920x1080.v210" << std::endl;
}

void testBurnerYUV422()
{
    SLC::Burner burner;
    uint32_t width = 1920;
    uint32_t height = 1080;
    
    burner.init(width, height, SLC::Burner::SourceType::YUV422);

    // Create a gray image (Y=128, U=128, V=128)
    // YUV422 Planar: Y plane (128), U plane (128), V plane (128)
    size_t frameSize = width * height * 2;
    std::vector<uint8_t> src(frameSize, 0x80);

    SLC::Burner::BurnerTimeId timeId;
    timeId.hour = 12;
    timeId.minute = 34;
    timeId.second = 56;
    timeId.decimal = 12800;

    // Burn at position (100, 100)
    ClockTimer timer;
    for (int i = 0; i < 100; ++i)
    {
        timer.reset();

        burner.burn(src.data(), src.size(), timeId, 100, 100);
        timeId.decimal += 100;

        double elapsed = timer.elapse_ms();
        std::cout << "Burn YUV422 time: " << elapsed << " ms" << std::endl;
    }

    // Save to file
    std::ofstream outFile("output_1920x1080.yuv422", std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(src.data()), src.size());
    outFile.close();

    std::cout << "Burn YUV422 test completed. Output saved to output_1920x1080.yuv422" << std::endl;
}

int main() {
    testBurner();
    testBurnerV210();
    testBurnerYUV422();
    system("pause");
    return 0;
}