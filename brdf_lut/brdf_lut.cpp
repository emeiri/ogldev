#include "gli/gli.hpp"
#include "gli/texture2d.hpp"
#include "gli/load_ktx.hpp"

#include "ogldev_glfw.h"
#include "brdf_lut_technique.h"

int brdfW = 256;
int brdfH = 256;

uint32_t bufferSize = 2 * sizeof(float) * brdfW * brdfH;

static gli::texture ConvertLUTtoTexture(const float* pData)
{
	gli::texture lutTexture = gli::texture2d(gli::FORMAT_RG16_SFLOAT_PACK16, gli::extent2d(brdfW, brdfH), 1);

	for (int y = 0; y < brdfH; y++)	{
		for (int x = 0; x < brdfW; x++) {
			int ofs = y * brdfW + x;
			gli::vec2 value(pData[ofs * 2 + 0], pData[ofs * 2 + 1]);

			gli::texture::extent_type uv = { x, y, 0 };
			lutTexture.store<glm::uint32>(uv, 0, 0, 0, gli::packHalf2x16(value));
		}
	}

	return lutTexture;
}


class BRDF_LUT
{
public:
	BRDF_LUT();

	void Init();
	void Render();
	const std::vector<char>& GetOutput() const { return m_outputData; }

private:

	void InitBuffers();
	void ReadResults();

	BRDF_LUT_Technique m_brdfTech;
	std::vector<char> m_outputData;
	GLuint m_vao;
	GLuint m_outputBuf = 0;
};


BRDF_LUT::BRDF_LUT()
{
}

void BRDF_LUT::Init()
{
	m_brdfTech.Init();
	InitBuffers();
}

void BRDF_LUT::Render()
{
	m_brdfTech.Enable();
	glBindVertexArray(m_vao);
	glDispatchCompute(brdfW, brdfH, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindVertexArray(0);

	ReadResults();
}

void BRDF_LUT::InitBuffers()
{
	GLuint InputBuf = 0;

	glGenBuffers(1, &InputBuf);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, InputBuf);
	float f = 0.0f;
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float), &f, GL_DYNAMIC_DRAW);

	glCreateBuffers(1, &m_outputBuf);
	glNamedBufferStorage(m_outputBuf, bufferSize, m_outputData.data(), GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);

	m_outputData.resize(bufferSize);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_outputBuf);
	glBindVertexArray(0);
}


void BRDF_LUT::ReadResults()
{
	// Map buffer for reading
	void* p = glMapNamedBufferRange(m_outputBuf, 0, bufferSize, GL_MAP_READ_BIT);

	if (!p) {
		printf("Map error\n");
		exit(1);
	}

	memcpy(m_outputData.data(), p, bufferSize);

	bool result = glUnmapNamedBuffer(m_outputBuf);
}


int main()
{
	int major_ver = 0;
	int minor_ver = 0;
	bool is_full_screen = false;
	GLFWwindow* pWindow = glfw_init(major_ver, minor_ver, 1920, 1080, is_full_screen, "brdf");

	BRDF_LUT brdf_lut;
	brdf_lut.Init();

	printf("Calculating LUT texture...\n");
	brdf_lut.Render();

	printf("Saving LUT texture...\n");
	gli::texture lutTexture = ConvertLUTtoTexture((float*)brdf_lut.GetOutput().data());

	// use Pico Pixel to view https://pixelandpolygon.com/ 
	gli::save_ktx(lutTexture, "brdfLUT.ktx");

	glfwTerminate();

	return 0;
}
