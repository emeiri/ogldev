#include "gli/gli.hpp"
#include "gli/texture2d.hpp"
#include "gli/load_ktx.hpp"

#include "ogldev_glfw.h"
#include "brdf_lut_technique.h"
#include "data.h"

constexpr int brdfW = 256;
constexpr int brdfH = 256;

const uint32_t bufferSize = 2 * sizeof(float) * brdfW * brdfH;


gli::texture convertLUTtoTexture(const float* data)
{
	gli::texture lutTexture = gli::texture2d(gli::FORMAT_RG16_SFLOAT_PACK16, gli::extent2d(brdfW, brdfH), 1);

	for (int y = 0; y < brdfH; y++)
	{
		for (int x = 0; x < brdfW; x++)
		{
			int ofs = y * brdfW + x;
			gli::vec2 value(data[ofs * 2 + 0], data[ofs * 2 + 1]);
			//value.x = 1.0f;
			//value.y = 1.0f;

			gli::texture::extent_type uv = { x, y, 0 };
			lutTexture.store<glm::uint32>(uv, 0, 0, 0, gli::packHalf2x16(value));
		}
	}

	return lutTexture;
}

int main()
{
	int major_ver = 0;
	int minor_ver = 0;
	bool is_full_screen = false;
	GLFWwindow* pWindow = glfw_init(major_ver, minor_ver, 1920, 1080, is_full_screen, "brdf");

	Particles foo;
	foo.Init();

	printf("Calculating LUT texture...\n");
	foo.Render();

	printf("Saving LUT texture...\n");
	gli::texture lutTexture = convertLUTtoTexture((float*)foo.GetOutput().data());

	// use Pico Pixel to view https://pixelandpolygon.com/ 
	gli::save_ktx(lutTexture, "brdfLUT.ktx");

	glfwTerminate();

	return 0;
}
