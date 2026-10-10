/*

        Copyright 2026 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


    Test mode for the golden-image regression suite (Testing/regress.py).

    Everything here is inert unless the process was started with --test-frames:

        --test-frames 10,60,200   capture these frames, then quit after the last one
        --screenshot-dir DIR      write DIR/f0010.png ... and DIR/info.json
        --test-seed S             seed for rand() and for Regress::Seed()
        --test-strict             OpenGL/Vulkan errors are fatal (exit code 3)

    The tutorials are not patched. On Windows the GLFW and freeglut headers are
    wrapped by Testing/shim (see Testing/README.md), which calls:

        Regress::Time(glfwGetTime())      for every clock read
        Regress::EndFrame()               before the buffer swap / the Vulkan present

    The capture itself is API specific and is registered with Regress::SetCapture
    (see regress_capture_gl.h for OpenGL and VulkanCore for Vulkan).

    The header has no dependencies so that it can be included by every project.
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

namespace Regress {

#define REGRESS_WINDOW_WIDTH  800
#define REGRESS_WINDOW_HEIGHT 600
#define REGRESS_FPS           60.0

#define REGRESS_EXIT_CAPTURE_FAILED 2
#define REGRESS_EXIT_API_ERROR      3

typedef bool (*CaptureFn)(const char* pPath, int Width, int Height, void* pApiContext);
typedef void (*QuitFn)();

struct State {
    bool Initialized = false;
    bool Active = false;
    bool Strict = false;
    unsigned int Seed = 1;
    std::vector<int> Frames;
    std::string Dir = ".";
    int Frame = 0;          // number of completed frames
    int Width = 0;
    int Height = 0;
    CaptureFn Capture = NULL;
    QuitFn Quit = NULL;
    std::string Api;
    std::string Renderer;
    std::string Version;
    bool InfoWritten = false;
};


inline State& GetState()
{
    static State s;
    return s;
}


inline FILE* OpenFile(const char* pPath, const char* pMode)
{
#ifdef _WIN32
    FILE* f = NULL;
    return (fopen_s(&f, pPath, pMode) == 0) ? f : NULL;
#else
    return fopen(pPath, pMode);
#endif
}


// Call once at startup. Returns true when running in test mode.
inline bool Init(int argc, char** argv)
{
    State& s = GetState();

    if (s.Initialized) {
        return s.Active;
    }

    s.Initialized = true;

    for (int i = 1; i < argc; i++) {
        bool HasValue = (i + 1 < argc);

        if (!strcmp(argv[i], "--test-frames") && HasValue) {
            for (const char* p = argv[++i]; *p; ) {
                char* pEnd = NULL;
                long Frame = strtol(p, &pEnd, 10);
                if (pEnd == p) {
                    break;
                }
                s.Frames.push_back((int)Frame);
                p = (*pEnd == ',') ? pEnd + 1 : pEnd;
            }
        } else if (!strcmp(argv[i], "--screenshot-dir") && HasValue) {
            s.Dir = argv[++i];
        } else if (!strcmp(argv[i], "--test-seed") && HasValue) {
            s.Seed = (unsigned int)strtoul(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "--test-strict")) {
            s.Strict = true;
        }
    }

    s.Active = !s.Frames.empty();

    if (s.Active) {
        srand(s.Seed);
        // The runner reads the output through a pipe
        setvbuf(stdout, NULL, _IONBF, 0);
        printf("Regress: test mode, %d capture(s), last frame %d, output '%s'\n",
               (int)s.Frames.size(), s.Frames.back(), s.Dir.c_str());
    }

    return s.Active;
}


// For code that has no access to the arguments of main (e.g. a shared framework)
inline bool Init()
{
    if (GetState().Initialized) {
        return GetState().Active;
    }

#ifdef _WIN32
    return Init(__argc, __argv);
#else
    static std::vector<std::string> Args;
    static std::vector<char*> Argv;

    FILE* f = fopen("/proc/self/cmdline", "rb");

    if (f) {
        std::string Arg;
        int c;
        while ((c = fgetc(f)) != EOF) {
            if (c == 0) {
                Args.push_back(Arg);
                Arg.clear();
            } else {
                Arg += (char)c;
            }
        }
        fclose(f);
    }

    for (std::string& Arg : Args) {
        Argv.push_back(&Arg[0]);
    }

    return Init((int)Argv.size(), Argv.data());
#endif
}


inline bool Active()
{
    State& s = GetState();
    return s.Initialized ? s.Active : Init();
}


inline bool Strict()
{
    return Active() && GetState().Strict;
}


inline unsigned int Seed()
{
    Active();
    return GetState().Seed;
}


// Number of frames that have been completed so far
inline int Frame()
{
    return GetState().Frame;
}


// Sets a fixed window size in test mode, else leaves the values alone
template<typename T>
inline void ForceWindowSize(T& Width, T& Height)
{
    if (Active()) {
        Width = (T)REGRESS_WINDOW_WIDTH;
        Height = (T)REGRESS_WINDOW_HEIGHT;
        GetState().Width = REGRESS_WINDOW_WIDTH;
        GetState().Height = REGRESS_WINDOW_HEIGHT;
    }
}


// Frame * (1/60) in test mode, else returns its argument
inline double Time(double RealTimeSeconds)
{
    return Active() ? (double)GetState().Frame / REGRESS_FPS : RealTimeSeconds;
}


// Same for clocks that count milliseconds
inline long long TimeMillis(long long RealTimeMillis)
{
    return Active() ? (long long)((double)GetState().Frame * 1000.0 / REGRESS_FPS) : RealTimeMillis;
}


inline void SetCapture(CaptureFn Fn)
{
    GetState().Capture = Fn;
}


// Called once after the last capture, for the case where EndFrame is called
// by shared code that cannot break out of the application's main loop
inline void SetQuit(QuitFn Fn)
{
    GetState().Quit = Fn;
}


// Goes into info.json. The runner keeps a separate set of goldens per renderer.
inline void SetInfo(const char* pApi, const char* pRenderer, const char* pVersion)
{
    State& s = GetState();
    s.Api = pApi ? pApi : "";
    s.Renderer = pRenderer ? pRenderer : "";
    s.Version = pVersion ? pVersion : "";
}


// Call from the OpenGL debug callback or the Vulkan debug messenger
inline void ReportApiError(const char* pMsg)
{
    if (Strict()) {
        fprintf(stderr, "Regress: API error in strict mode: %s\n", pMsg ? pMsg : "");
        exit(REGRESS_EXIT_API_ERROR);
    }
}


inline std::string JsonEscape(const std::string& In)
{
    std::string Out;

    for (char c : In) {
        if ((c == '"') || (c == '\\')) {
            Out += '\\';
            Out += c;
        } else if ((unsigned char)c >= 0x20) {
            Out += c;
        }
    }

    return Out;
}


inline void WriteInfo(int Width, int Height)
{
    State& s = GetState();
    std::string Path = s.Dir + "/info.json";
    FILE* f = OpenFile(Path.c_str(), "w");

    if (!f) {
        fprintf(stderr, "Regress: cannot write '%s'\n", Path.c_str());
        exit(REGRESS_EXIT_CAPTURE_FAILED);
    }

    fprintf(f, "{\n \"api\": \"%s\",\n \"renderer\": \"%s\",\n \"version\": \"%s\",\n \"width\": %d,\n \"height\": %d,\n \"seed\": %u\n}\n",
            JsonEscape(s.Api).c_str(), JsonEscape(s.Renderer).c_str(), JsonEscape(s.Version).c_str(), Width, Height, s.Seed);
    fclose(f);
}


// A minimal PNG writer (8 bit RGBA, no compression) so that the hook doesn't need
// stb_image_write to be linked into every project. The runner recompresses the
// images that it keeps as goldens.
inline bool WritePNG(const char* pPath, int Width, int Height, const unsigned char* pRGBA, bool FlipVertically)
{
    static unsigned int CrcTable[256];
    static bool CrcTableReady = false;

    if (!CrcTableReady) {
        for (unsigned int n = 0; n < 256; n++) {
            unsigned int c = n;
            for (int k = 0; k < 8; k++) {
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            }
            CrcTable[n] = c;
        }
        CrcTableReady = true;
    }

    // The image data: every row is prefixed by a filter byte (0 = none)
    size_t RowSize = (size_t)Width * 4 + 1;
    std::vector<unsigned char> Raw(RowSize * Height);

    for (int y = 0; y < Height; y++) {
        int SrcY = FlipVertically ? (Height - 1 - y) : y;
        Raw[y * RowSize] = 0;
        memcpy(&Raw[y * RowSize + 1], pRGBA + (size_t)SrcY * Width * 4, (size_t)Width * 4);
    }

    // zlib stream made of stored deflate blocks
    std::vector<unsigned char> Z;
    Z.reserve(Raw.size() + Raw.size() / 65535 * 5 + 16);
    Z.push_back(0x78);
    Z.push_back(0x01);

    unsigned int A = 1, B = 0;    // adler32

    for (size_t Pos = 0; Pos < Raw.size(); ) {
        size_t Len = Raw.size() - Pos;
        if (Len > 65535) {
            Len = 65535;
        }
        bool Last = (Pos + Len == Raw.size());
        Z.push_back(Last ? 1 : 0);
        Z.push_back((unsigned char)(Len & 0xFF));
        Z.push_back((unsigned char)(Len >> 8));
        Z.push_back((unsigned char)(~Len & 0xFF));
        Z.push_back((unsigned char)((~Len >> 8) & 0xFF));
        Z.insert(Z.end(), Raw.begin() + Pos, Raw.begin() + Pos + Len);

        for (size_t i = 0; i < Len; i++) {
            A = (A + Raw[Pos + i]) % 65521;
            B = (B + A) % 65521;
        }

        Pos += Len;
    }

    unsigned int Adler = (B << 16) | A;
    for (int Shift = 24; Shift >= 0; Shift -= 8) {
        Z.push_back((unsigned char)(Adler >> Shift));
    }

    FILE* f = OpenFile(pPath, "wb");

    if (!f) {
        return false;
    }

    auto WriteChunk = [&](const char* pType, const unsigned char* pData, size_t Size) {
        unsigned char Len[4] = { (unsigned char)(Size >> 24), (unsigned char)(Size >> 16), (unsigned char)(Size >> 8), (unsigned char)Size };
        fwrite(Len, 1, 4, f);
        fwrite(pType, 1, 4, f);
        if (Size) {
            fwrite(pData, 1, Size, f);
        }

        unsigned int c = 0xFFFFFFFFu;
        for (int i = 0; i < 4; i++) {
            c = CrcTable[(c ^ (unsigned char)pType[i]) & 0xFF] ^ (c >> 8);
        }
        for (size_t i = 0; i < Size; i++) {
            c = CrcTable[(c ^ pData[i]) & 0xFF] ^ (c >> 8);
        }
        c ^= 0xFFFFFFFFu;

        unsigned char Crc[4] = { (unsigned char)(c >> 24), (unsigned char)(c >> 16), (unsigned char)(c >> 8), (unsigned char)c };
        fwrite(Crc, 1, 4, f);
    };

    const unsigned char Signature[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    fwrite(Signature, 1, 8, f);

    unsigned char Header[13] = { (unsigned char)(Width >> 24), (unsigned char)(Width >> 16), (unsigned char)(Width >> 8), (unsigned char)Width,
                                 (unsigned char)(Height >> 24), (unsigned char)(Height >> 16), (unsigned char)(Height >> 8), (unsigned char)Height,
                                 8, 6, 0, 0, 0 };    // 8 bits per channel, RGBA
    WriteChunk("IHDR", Header, sizeof(Header));
    WriteChunk("IDAT", Z.data(), Z.size());
    WriteChunk("IEND", NULL, 0);

    bool Ok = !ferror(f);
    fclose(f);

    return Ok;
}


// Call after the frame has been drawn and before the swap/present.
// Returns true when the application should quit.
inline bool EndFrame(void* pApiContext = NULL)
{
    if (!Active()) {
        return false;
    }

    State& s = GetState();

    s.Frame++;

    for (int Frame : s.Frames) {
        if (Frame != s.Frame) {
            continue;
        }

        if (!s.Capture) {
            fprintf(stderr, "Regress: no capture function was registered\n");
            exit(REGRESS_EXIT_CAPTURE_FAILED);
        }

        char Name[32];
        snprintf(Name, sizeof(Name), "/f%04d.png", s.Frame);
        std::string Path = s.Dir + Name;

        if (!s.Capture(Path.c_str(), s.Width, s.Height, pApiContext)) {
            fprintf(stderr, "Regress: capture of frame %d to '%s' failed\n", s.Frame, Path.c_str());
            exit(REGRESS_EXIT_CAPTURE_FAILED);
        }

        printf("Regress: captured frame %d\n", s.Frame);
        break;
    }

    int LastFrame = 0;
    for (int Frame : s.Frames) {
        if (Frame > LastFrame) {
            LastFrame = Frame;
        }
    }

    bool Done = (s.Frame >= LastFrame);

    if (Done && s.Quit) {
        s.Quit();
        s.Quit = NULL;
    }

    return Done;
}

}
