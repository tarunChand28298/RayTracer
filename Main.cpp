#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

struct Sphere {
	float x;
	float y;
	float z;
	float radius;
};
Sphere spheres[] = {
	{0.0f, 0.0f, 0.0f, 1.0f}
};

#pragma region Global Variables:
bool running = false;
int WindowWidth = 800;
int WindowHeight = 600;
#pragma endregion


LRESULT CALLBACK DirectXWindowProc(HWND windowHanlde, UINT message, WPARAM wparam, LPARAM lparam) {

	switch (message) {
		case WM_QUIT: {
			DestroyWindow(windowHanlde);
			break;
		}
		case WM_DESTROY: {
			running = false;
			break;
		}
		default: {
			break;
		}
	}

	return DefWindowProc(windowHanlde, message, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, char* cmdArgs, int cmdShow) {
	//=============================================================================================================================================

	#pragma region Stack Variables:
	HWND windowHandle;

	ID3D11Device* 				device = nullptr;
	ID3D11DeviceContext* 			deviceContext = nullptr;
	IDXGISwapChain* 			swapchain = nullptr;

	ID3D11ComputeShader* 			computeShader = nullptr;
	ID3D11Buffer* 				inputBuffer = nullptr;
	ID3D11UnorderedAccessView* 		outputBackBuffer = nullptr;
	#pragma endregion

	//=============================================================================================================================================
	#pragma region Create Window:
	{
		WNDCLASS wc = {};
		wc.hInstance = instance;
		wc.lpfnWndProc = DirectXWindowProc;
		wc.lpszClassName = "DirectXWindowClass";
		wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		RegisterClass(&wc);

		windowHandle = CreateWindow("DirectXWindowClass", "DirectX Render Window", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight, nullptr, nullptr, instance, nullptr);
		ShowWindow(windowHandle, SW_SHOW);
	}
	#pragma endregion
	#pragma region Create Device and SwapChain:
	{
		DXGI_SWAP_CHAIN_DESC scd = {};
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.SampleDesc.Count = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
		scd.BufferCount = 2;
		scd.OutputWindow = windowHandle;
		scd.Windowed = true;

		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &swapchain, &device, nullptr, &deviceContext);
	}
	#pragma endregion
	#pragma region Create the Shader:
	{
		ID3D10Blob* shaderData;
		D3DReadFileToBlob((LPCWSTR)L"ComputeShader.cso", &shaderData);

		device->CreateComputeShader(shaderData->GetBufferPointer(), shaderData->GetBufferSize(), nullptr, &computeShader);
		deviceContext->CSSetShader(computeShader, nullptr, 0);

		shaderData->Release();
	}
	#pragma endregion
	#pragma region Create Input for Shader:
	{
		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = sizeof(Sphere) * ARRAYSIZE(spheres);
		bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bd.StructureByteStride = sizeof(Sphere);

		D3D11_SUBRESOURCE_DATA srd = {spheres, 0, 0};
		
		device->CreateBuffer(&bd, &srd, &inputBuffer);
		deviceContext->CSSetConstantBuffers(0, 1, &inputBuffer);
	}
	#pragma endregion
	#pragma region Create Output for Shader:
	{
		ID3D11Texture2D* backBuffer = nullptr;
		D3D11_TEXTURE2D_DESC backBufferDesc = {};
		swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
		backBuffer->GetDesc(&backBufferDesc);

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = backBufferDesc.ArraySize;
		uavDesc.Format = backBufferDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

		UINT uavInitialCount = 0;

		device->CreateUnorderedAccessView(backBuffer, &uavDesc, &outputBackBuffer);
		deviceContext->CSSetUnorderedAccessViews(0, 1, &outputBackBuffer, &uavInitialCount);

		backBuffer->Release();
	}
	#pragma endregion

	//=============================================================================================================================================

	running = true;
	while (running) {
		#pragma region Handle Window Messages:
		{
			MSG message = {};
			while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&message);
				DispatchMessage(&message);

				if (message.message == WM_CLOSE) {
					PostQuitMessage(0);
					running = false;
				}
			}
		}
		#pragma endregion
		#pragma region Handle Input:
		{

		}
		#pragma endregion
		#pragma region Update Frame:
		{

		}
		#pragma endregion
		#pragma region Render to Screen:
		{
			deviceContext->Dispatch(WindowWidth / 8, WindowHeight / 8, 1);
			swapchain->Present(1, 0);
		}
		#pragma endregion
	}

	//=============================================================================================================================================

	#pragma region Clean Up:
	device->Release();
	deviceContext->Release();
	swapchain->Release();
	computeShader->Release();
	inputBuffer->Release();
	outputBackBuffer->Release();
	#pragma endregion

	return 0;
}
