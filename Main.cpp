#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma region Global Variables:
bool running = false;
int WindowWidth = 800;
int WindowHeight = 600;
float NearZ = 1.0f;
float FarZ = 10000.0f;
float FovAngleYDeg = 90.0f;
float FovAngleY = FovAngleYDeg * 0.0174533f;
#pragma endregion

#pragma region Shader Inputs:
	struct Sphere {
		float x, y, z;
		float radius;
		float albedox, albedoy, albedoz;
		float specularx, speculary, specularz;
	};

	Sphere spheres[] = {
		{0.0f, 0.0f, 0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
		{3.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 3.0f, 0.0f, 1.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 3.0f, 1.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f},
		{7.0f, 7.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f}
	};

	struct Camera {
		DirectX::XMMATRIX cameraToWorldMatrix;
		DirectX::XMMATRIX cameraInverseProjectionMatrix;
	};

	DirectX::XMMATRIX cameraToWorldt = DirectX::XMMATRIX(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.97f, 0.21f, 8.8f, 0.0f, 0.21f, -0.97f, -17.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMMATRIX inverseProjectiont = DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixPerspectiveFovLH(FovAngleY, float(WindowWidth)/float(WindowHeight), NearZ, FarZ));
	
	Camera cam = {};
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

	ID3D11Device*					device = nullptr;
	ID3D11DeviceContext*			deviceContext = nullptr;
	IDXGISwapChain*					swapchain = nullptr;

	ID3D11ComputeShader*			computeShader = nullptr;

	ID3D11Buffer*					inputSphereBuffer = nullptr;
	ID3D11ShaderResourceView*		inputSphereBufferView = nullptr;
	ID3D11Buffer*					cameraBuffer = nullptr;

	ID3D11UnorderedAccessView*		outputBackBuffer = nullptr;
#pragma endregion

	//=============================================================================================================================================
#pragma region Create Window:
	{
		WNDCLASS wc = {};
		wc.hInstance = instance;
		wc.lpfnWndProc = DirectXWindowProc;
		wc.lpszClassName = "DirectXWindowClass";
		wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
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
		scd.BufferUsage = DXGI_USAGE_UNORDERED_ACCESS;
		scd.BufferCount = 2;
		scd.OutputWindow = windowHandle;
		scd.Windowed = true;
		//in the end, change creation flag from D3D11_CREATE_DEVICE_DEBUG to 0.
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION, &scd, &swapchain, &device, nullptr, &deviceContext);
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
		//For spheres:
		{
			//Create input buffer:
			D3D11_BUFFER_DESC bd = {};
			bd.ByteWidth = sizeof(Sphere)*ARRAYSIZE(spheres);
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.StructureByteStride = sizeof(Sphere);
			bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

			D3D11_SUBRESOURCE_DATA srd = {};
			srd.pSysMem = spheres;
			srd.SysMemPitch = 0;
			srd.SysMemSlicePitch = 0;

			device->CreateBuffer(&bd, &srd, &inputSphereBuffer);

			//create the SRV:
			D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
			srvd.Format = DXGI_FORMAT_UNKNOWN;
			srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvd.Buffer.ElementOffset = 0;
			srvd.Buffer.ElementWidth = sizeof(Sphere);
			srvd.Buffer.FirstElement = 0;
			srvd.Buffer.NumElements = ARRAYSIZE(spheres);

			device->CreateShaderResourceView(inputSphereBuffer, &srvd, &inputSphereBufferView);

		}
		//For the matricies:
		{
			cam.cameraToWorldMatrix = DirectX::XMMatrixTranspose(cameraToWorldt);
			cam.cameraInverseProjectionMatrix = DirectX::XMMatrixTranspose(inverseProjectiont);

			D3D11_BUFFER_DESC cbd = {};
			cbd.ByteWidth = sizeof(Camera);
			cbd.Usage = D3D11_USAGE_DEFAULT;
			cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbd.CPUAccessFlags = 0u;
			cbd.StructureByteStride = 0;
			cbd.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA csd1 = {};
			csd1.pSysMem = &cam;

			device->CreateBuffer(&cbd, &csd1, &cameraBuffer);
		}
	}
#pragma endregion
#pragma region Create Output for Shader:
	{
		//Get the backbuffer from the swap chain:
		ID3D11Texture2D* backBuffer = nullptr;
		D3D11_TEXTURE2D_DESC backBufferDesc = {};
		swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
		backBuffer->GetDesc(&backBufferDesc);

		//Create the UAV:
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = backBufferDesc.ArraySize;
		uavDesc.Format = backBufferDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		UINT uavInitialCount = 0;

		device->CreateUnorderedAccessView(backBuffer, &uavDesc, &outputBackBuffer);

		//Set the backbuffer as UAV:
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
			spheres[0].x += 0.1f;
			spheres[4].albedox += 0.01f;
			if (spheres[0].x > 8.0f)
			{
				spheres[0].x = -8.0f;
			}
			if (spheres[4].albedox > 1.0f)
			{
				spheres[4].albedox = 0.0f;
			}
		}
#pragma endregion
#pragma region Render to Screen:
		{

			//Set the Inputs:
			deviceContext->CSSetShaderResources(0, 1, &inputSphereBufferView);
			deviceContext->CSSetConstantBuffers(0, 1, &cameraBuffer);
			deviceContext->UpdateSubresource(inputSphereBuffer, 0, 0, spheres, 0, 0);
			deviceContext->UpdateSubresource(cameraBuffer, 0, 0, &cam, 0, 0);

			//Render:
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
	inputSphereBuffer->Release();
	inputSphereBufferView->Release();
	cameraBuffer->Release();
	outputBackBuffer->Release();
#pragma endregion

	return 0;
}
