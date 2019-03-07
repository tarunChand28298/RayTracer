#include <Windows.h>
#include <d3d11.h>

struct Vertex
{
	float position[3];
	float colour[3];
};

#pragma region Global Variables:
	bool Running = false;
	int WindowSizeX = 800;
	int WindowSizeY = 600;
	Vertex verticies[] = { { 0.0f, 0.5f, 0.0f }, { 0.45f, -0.5f, 0.0f }, { -0.45f, -0.5f, 0.0f }, };
#pragma endregion

LRESULT CALLBACK RenderWindowProcedure(HWND windowHandle, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
		case WM_QUIT: {
			DestroyWindow(windowHandle);
			break;
		}
		case WM_DESTROY: {
			Running = false;
			break;
		}
		default: {

			break;
		}	 
	}

	return DefWindowProc(windowHandle, message, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, char* cmdArgs, int cmdShow) {
	
#pragma region Stack Varibles:
	HWND windowHandle = 0;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	IDXGISwapChain* swapChain = nullptr;
	ID3D11Buffer* inputBuffer = nullptr;
	ID3D11ShaderResourceView* inputBufferView = nullptr; //to be initialized once I understand why it is needed.
	ID3D11UnorderedAccessView* outputRenderTarget = nullptr;
#pragma endregion

	#pragma region Window Creation:
	{
		WNDCLASS wc = {};
		wc.hInstance = instance;
		wc.lpfnWndProc = RenderWindowProcedure;
		wc.lpszClassName = "Direct3dRenderWindow";
		wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		RegisterClass(&wc);

		windowHandle = CreateWindow("Direct3dRenderWindow", "DirectX render window", WS_OVERLAPPED|WS_SYSMENU|WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, WindowSizeX, WindowSizeY, nullptr, nullptr, instance, nullptr);
		ShowWindow(windowHandle, SW_SHOW);
	}
	#pragma endregion

	#pragma region Device and SwapChain:
	{
		DXGI_SWAP_CHAIN_DESC scd = {};
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.SampleDesc.Count = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
		scd.BufferCount = 2;
		scd.OutputWindow = windowHandle;
		scd.Windowed = true;

		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &deviceContext);
	}
	#pragma endregion

	#pragma region Shader Input:
	{
		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(verticies);
		bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA srd = { verticies, 0, 0 };

		device->CreateBuffer(&bd, &srd, &inputBuffer);
	}
	#pragma endregion

	#pragma region Shader and Input Layout:
	{

	}
	#pragma endregion

	#pragma region Render Target:
	{
		ID3D11Texture2D * backBuffer = nullptr;
		D3D11_TEXTURE2D_DESC backBufferDesc = {};
		UINT initCount = 0;

		swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
		backBuffer->GetDesc(&backBufferDesc);

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = backBufferDesc.ArraySize;
		uavDesc.Format = backBufferDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

		device->CreateUnorderedAccessView(backBuffer, &uavDesc, &outputRenderTarget);
		deviceContext->CSSetUnorderedAccessViews(0, 1, &outputRenderTarget, &initCount);
	}
	#pragma endregion



	Running = true;
	while (Running)
	{
		{
			MSG message = {};
			while (PeekMessage(&message, windowHandle, 0, 0, PM_REMOVE) > 0) {
				if (message.message == WM_CLOSE) { 
					PostQuitMessage(0);
				}
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}

		{
			//update
		}

		{
			//render
		}

	}

#pragma region Clean up:
	device->Release();
	deviceContext->Release();
	swapChain->Release();
	inputBuffer->Release();
	outputRenderTarget->Release();
#pragma endregion

	
	return 0;
}
