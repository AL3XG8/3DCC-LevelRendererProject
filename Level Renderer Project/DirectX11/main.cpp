// Simple basecode showing how to create a window and attatch a d3d11surface
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_GRAPHICS // Enables all Graphics Libraries
// Ignore some GRAPHICS libraries we aren't going to use
#define GATEWARE_DISABLE_GDIRECTX12SURFACE // we have another template for this
#define GATEWARE_DISABLE_GRASTERSURFACE // we have another template for this
#define GATEWARE_DISABLE_GOPENGLSURFACE // we have another template for this
#define GATEWARE_DISABLE_GVULKANSURFACE // we have another template for this

#define GATEWARE_ENABLE_MATH 
#define GATEWARE_ENABLE_INPUT
#define GATEWARE_ENABLE_AUDIO
#define GATEWARE_ENABLE_MUSIC

// With what we want & what we don't defined we can include the API
#include "../gateware-main/gateware.h"
#include <shobjidl.h>
#include "FileIntoString.h"
#include "renderManager.h" // example rendering code (not Gateware code!)
// open some namespaces to compact the code a bit
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;

GWindow win;
Level_Objects gameLevel;
Model model;
GEventResponder msgs;
GDirectX11Surface d3d11;

GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE uwh = {};

static void ChangeLevel(GWindow& wind, GW::GRAPHICS::GDirectX11Surface& d3d)
{
	float f1State = 0.0f;
	gInput.GetState(G_KEY_F1, f1State);
	if (f1State == 1.0f)
	{
		wind.GetWindowHandle(uwh);
		OPENFILENAME ofn;
		wchar_t szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = reinterpret_cast<HWND>(uwh.window);
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = L"Open Level";
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn) == TRUE)
		{
			std::wstring ws(ofn.lpstrFile);
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
			std::string fullLevelPath = converter.to_bytes(ws);

			size_t lastSlashPos = fullLevelPath.find_last_of("\\/");
			std::string levelFilename;

			if (lastSlashPos != std::string::npos)
			{
				levelFilename = fullLevelPath.substr(lastSlashPos + 1);
			}
			else
			{
				levelFilename = fullLevelPath;
			}

			changedLevel = true;

			const char* levelPath = levelFilename.c_str();
			const char* modelPath = "Models";

			info.Create("ChangeLevel.txt");
			//info.EnableConsoleLogging(true);
			//bool currentLevel = gameLevel.LoadLevel(levelPath, "Models", info);

			std::cout << "Changing level to: " << fullLevelPath << std::endl;
			gameLevel.LoadLevel(levelPath, "Models", info);
			gameLevel.PrintAllObjectsInLevel(gameLevel.GetAllObjectsInLevel());

			gameLevel.UploadLevelToGPU(win, d3d);
		}
	}
}

// lets pop a window and use D3D11 to clear to a green screen
int main()
{
	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERED))
	{
		//TODO: part 1A 
		win.SetWindowName("AGurwell - DirectX11");

		//float clr[] = { 57/255.0f, 1.0f, 20/255.0f, 1 }; // TODO: Part 1A (optional)

		float backgroundColor[] = { 0.0f, 0.5f, 1.0f, 1.0f };
		float triangleColor[] = { 0.0f, 0.0f, 0.2f, 1.0f };

		msgs.Create([&](const GW::GEvent& e) {
			GW::SYSTEM::GWindow::Events q;
			if (+e.Read(q) && q == GWindow::Events::RESIZE)
				//clr[2] += 0.01f; // move towards a cyan as they resize

				backgroundColor[2] += 0.01f;
			});
		win.Register(msgs);
		if (+d3d11.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		{
			RenderManager renderer(win, d3d11);
			while (+win.ProcessWindowEvents())
			{
				IDXGISwapChain* swap;
				ID3D11DeviceContext* con;
				ID3D11RenderTargetView* view;
				ID3D11DepthStencilView* depth;
				if (+d3d11.GetImmediateContext((void**)&con) &&
					+d3d11.GetRenderTargetView((void**)&view) &&
					+d3d11.GetDepthStencilView((void**)&depth) &&
					+d3d11.GetSwapchain((void**)&swap))
				{
					con->ClearRenderTargetView(view, backgroundColor);
					con->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);

					ChangeLevel(win, d3d11);
					renderer.UpdateCamera();
					renderer.Render();

					swap->Present(1, 0);
					// release incremented COM reference counts
					swap->Release();
					view->Release();
					depth->Release();
					con->Release();
				}
			}
		}
	}
	return 0; // that's all folks
}