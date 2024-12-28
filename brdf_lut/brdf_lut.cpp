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


class Particles
{
public:
	Particles();

	void Init();
	void Render();
	const std::vector<char>& GetOutput();

private:

	void InitBuffers();
	//  void ExecuteComputeShader(const Vector3f& BlackHolePos1, const Vector3f& BlackHolePos2);
	 // void RenderParticles(const Vector3f& BlackHolePos1, const Vector3f& BlackHolePos2, const Matrix4f& VP);

	BRDF_LUT_Technique m_brdfTech;
	std::vector<char> m_outputData;
	GLuint m_vao;
	GLuint m_outputBuf = 0;
};


Particles::Particles()
{
}

void Particles::Init()
{
	m_brdfTech.Init();
	InitBuffers();
}

void Particles::Render()
{
	m_brdfTech.Enable();
	glBindVertexArray(m_vao);
	glDispatchCompute(brdfW, brdfH, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindVertexArray(0);
}

void Particles::InitBuffers()
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


const std::vector<char>& Particles::GetOutput()
{
	// Map buffer for reading
	void* p = glMapNamedBufferRange(m_outputBuf, 0, bufferSize, GL_MAP_READ_BIT);

	if (!p) {
		printf("Map error\n");
		exit(1);
	}

	memcpy(m_outputData.data(), p, bufferSize);

	bool result = glUnmapNamedBuffer(m_outputBuf);

	return m_outputData;
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
