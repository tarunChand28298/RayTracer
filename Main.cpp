#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

struct Vertex
{
	float position[3];
};
struct Face {
	int verticiesIndex[3];
	UINT32 colour;
};

#pragma region Global Variables:
	bool Running = false;
	int WindowSizeX = 800;
	int WindowSizeY = 600;
	Vertex verticies[] = { { 0.0f, 0.5f, 0.0f }, { 0.45f, -0.5f, 0.0f }, { -0.45f, -0.5f, 0.0f } };
	Face triangles[] = { {1, 2, 3, 0xFFFFFF} };
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
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* faceBuffer = nullptr;

	ID3D11ShaderResourceView* vertexBufferView = nullptr; //I will initialize this once I understand why it is needed. 
	ID3D11ShaderResourceView* faceBufferView = nullptr;  //Will creating an input layout not work?

	ID3D11UnorderedAccessView* outputRenderTarget = nullptr;
	ID3D10Blob* computeShaderData = nullptr;
	ID3D11ComputeShader* computeShader = nullptr;
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
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS; //Will these flags let me write to the backbuffer and then show that to the screen?
		scd.BufferCount = 2;
		scd.OutputWindow = windowHandle;
		scd.Windowed = true;

		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &deviceContext);
	}
	#pragma endregion

	#pragma region Shader Input:
	{
		//for verticies:
		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(verticies);
		vbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA vSrDesc = { verticies, 0, 0 };
		device->CreateBuffer(&vbDesc, &vSrDesc, &vertexBuffer);

		//for faces:
		D3D11_BUFFER_DESC fbDesc = {};
		fbDesc.ByteWidth = sizeof(Face) * ARRAYSIZE(triangles);
		fbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA fSrDesc = { triangles, 0, 0 };
		device->CreateBuffer(&fbDesc, &fSrDesc, &faceBuffer);
	}
	#pragma endregion

	#pragma region Shader and Input Layout:
	{
		//D3DReadFileToBlob(L"ComputeShader.cso", &computeShaderData);
		//device->CreateComputeShader(computeShaderData->GetBufferPointer(), computeShaderData->GetBufferSize(), nullptr, &computeShader);
		//deviceContext->CSSetShader(computeShader, nullptr, 0);

		//do I need to set the input layout, or do I have to bind the shader resource views?
		//I have this doubt because when I tried to return values from compute shader, the error said that compute shaders can't have return values.
		//In the same way, I think input can't be passed to these shaders as arguments. They can only read from and write to buffers.
	}
	#pragma endregion

	#pragma region Render Target:
	{
		ID3D11Texture2D * backBuffer = nullptr;
		D3D11_TEXTURE2D_DESC backBufferDesc = {};
		UINT initCount = 0; //I don't understand what this does. 
		//I put this here because I saw it on this website: http://www.voidcn.com/article/p-evagmova-h.html

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

	#pragma region Setting up viewport:
	{
		//Is this part needed? because I am not going through the pipeline and the rasterizer stage is useless for me.
		//but maybe the backbuffer resizes according to this? I don't know.
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
			//render:
			//deviceContext->Dispatch(WindowSizeX, WindowSizeY, 1); 
			//What is the threadgroup, I do not understand. I tried to see how it is used and in this code: 
			//	https://github.com/adasm/xphoton/blob/master/Raytracer/main.cpp, 
			//he seems to use the dimensions of the window, but I have no clue what these arguements do.
			swapChain->Present(1, 0);
		}

	}

#pragma region Clean up:
	device->Release();
	deviceContext->Release();
	swapChain->Release();
	vertexBuffer->Release();
	faceBuffer->Release();
	outputRenderTarget->Release();
	//computeShaderData->Release();
	//computeShader->Release();
#pragma endregion

	
	return 0;
}
