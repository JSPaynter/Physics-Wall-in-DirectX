//**************************************************************************//
// This is really where we kick off, and is a modified version of the		//
// Microsoft sample code.  This one makes a two spinning cubes.				//
//																			//
// NOTE there is only actually one cube, but that cube is drawn twice on two//
// different places with two different sizes.								//
//																			//
// Look for the Nigel style comments, like these, for the bits you need to  //
// look at.																	//
//**************************************************************************//

//**************************************************************************//
// Modifications to the MS sample code is copyright of Dr Nigel Barlow,		//
// lecturer in computing, University of Plymouth, UK.						//
// email: nigel@soc.plymouth.ac.uk.											//
//																			//
// You may use, modify and distribute this (rather cack-handed in places)	//
// code subject to the following conditions:								//
//																			//
//	1:	You may not use it, or sell it, or use it in any adapted form for	//
//		financial gain, without my written premission.						//
//																			//
//	2:	You must not remove the copyright messages.							//
//																			//
//	3:	You should correct at least 10% of the typing and spelling errors.  //
//**************************************************************************//

//--------------------------------------------------------------------------------------
// File: Tutorial05 - Matrices.cpp
//
// This application demonstrates animation using matrix transformations
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11.h>
#include <d3dx9.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "resource.h"
#include <D3DX10.h>
#include <vector>
#include <iostream>
#include <tchar.h>

//**************************************************************************//
// Nothing is easy in DirectX.  Before we can even create a single vertex,	//
// we need to define what it looks like.									//
//																			//
// The data types seems to be inhereted from XNA.							//
// An XMFLOAT3 is a float containing 3 numbers, an x, y, x position here.	//
// An XMFLOAT4 is a float containing 4 values, an RGBA colour.	Not that	//
// alpha effects work without additional effort.							//
//**************************************************************************//
struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};


//**************************************************************************//
// These are structures we pass to the pixel shader.  						//
//																			//
// These structures must be identical to those defined in the shader that	//
// you use.  So much for encapsulation; Roy	Tucker (Comp Sci students will  //
// know him) will not approve.												//
//**************************************************************************//
struct MatrixConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};



//**************************************************************************//
// Global Variables.  There are many global variables here (we aren't OO	//
// yet.  I doubt  Roy Tucker (Comp Sci students will know him) will			//
// approve of this either.  Sorry, Roy.										//
//**************************************************************************//
HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11Texture2D*        g_pDepthStencil = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;
ID3D11VertexShader*     g_pVertexShader = NULL;
ID3D11PixelShader*      g_pPixelShader = NULL;
ID3D11InputLayout*      g_pVertexLayout = NULL;
ID3D11Buffer*           g_pVertexBuffer = NULL;
ID3D11Buffer*           g_pIndexBuffer = NULL;
XMMATRIX                g_MatView;
XMMATRIX                g_MatProjection;

//keyboard binds

SHORT kbQ;
SHORT kbE;
SHORT kbW;
SHORT kbA;
SHORT kbS;
SHORT kbD;
SHORT kbZ;
SHORT kbX;
SHORT kbR;
SHORT kbPGUP;
SHORT kbPGDOWN;

//camera variables
float cameraX = 0.0f;
float cameraY = 1.0f;
float cameraZ = -7.0f;
float cameraSpeed = 0.002;

float cameraLookX = 0.0f;
float cameraLookY = 1.0f;
float cameraLookZ = 0.0f;

bool cameraLookQE = true;

//cube variables
int numOfCubes = 9; //number of total cubes
float cubeSizeX = 0.6f; // cube size X
float cubeSizeZ = 0.6f; // cube size Z
float cubeSizeY = 0.6f; // cube size y
float cubeSpeed = 0.0f; //movement speed of cubes hitting wall, toggles to 0.0025f
float cubeFallAcc = 0.000025f; //speed up for y axis on cubes
float cubeDrag = 0.99f; //multiply x and z to slow down
float cubeMass = 1.0f; //mass for force formula

std::vector<float> cubeForce;

std::vector<float> cubeMoveX; //these 3 vectors determine the movement of the cube in each direction
std::vector<float> cubeMoveY; //these are adjusted on every update with the cubes drag and fall acceleration
std::vector<float> cubeMoveZ;

std::vector<float> cubeCoorsOriginal;
std::vector<float> cubeCoors;

//for wall cubes
MatrixConstantBuffer cubeBuilders[9];
XMMATRIX matWorlds[9];
XMMATRIX mTranslationList[9];

//**************************************************************************//
// Now a global instance of each constant buffer.							//
//**************************************************************************//
ID3D11Buffer*           g_pConstantBufferMatrices = NULL;



//**************************************************************************//
// Forward declarations.													//
//																			//
// If you are not used to "C" you will find that functions (or methods in	//
// "C++" must have templates defined in advance.  It is usual to define the //
// prototypes in a header file, but we'll put them here for now to keep		//
// things simple.															//
//**************************************************************************//
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();
void charStrToWideChar(WCHAR *dest, char *source);
void CheckKeyboard();
void ActionKeyboard();
void CheckCollisions();
void CheckOutOfBounds();
void CubeReset();
void AddMovements();
void RenderText();
std::vector<float> AngleCalculator(int firstCube, int secondCube);
std::vector<float> forceCalculator(int firstCube, int secondCube);

//**************************************************************************//
// A Windows program always kicks off in WinMain.							//
// Initializes everything and goes into a message processing				//
// loop. Idle time is used to render the scene.								//
//																			//
// In other words, run the computer flat out.  Is this good?				//
//**************************************************************************//
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine, int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	//**************************************************************************//
	// Main Windows message loop.												//
	//																			//
	// Gamers will see this as a game loop, though you will find something like //
	// this main loop deep within any Windows application.						//
	//**************************************************************************//
	MSG msg = { 0 };

	//create coor list, default start
	cubeCoorsOriginal = { 0.0f, 0.0f, 0.0f, -0.6f, 0.0f, 0.6f, -0.3f, 0.3f, 0.0f, //x
							0.3f, 0.3f, 0.3f, 1.6f, 1.6f, 1.6f, 2.6f, 2.6f, 4.8f, //y
							-4.0f, -4.0f, -4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; //z

	cubeCoors.resize(numOfCubes*3);

	for (int i = 0; i < numOfCubes * 3; i++) {
		cubeCoors[i] = cubeCoorsOriginal[i];
	}

	cubeMoveX.resize(numOfCubes); //set vector size for cube movements
	cubeMoveY.resize(numOfCubes);
	cubeMoveZ.resize(numOfCubes);

	//set cube force
	cubeForce = { 0.5f, 1.0f, 1.5f }; //set different cube speeds

	//render text
	RenderText();

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{	
			//adjust cube position
			for (int i = 0; i < 3; i++) {
				cubeCoors[i + (numOfCubes * 2)] += cubeSpeed*cubeForce[i]; //adjusts speeds so all cubes are different
			}
			CheckKeyboard();
			ActionKeyboard();
			CheckCollisions();
			AddMovements();
			CheckOutOfBounds();
			Render();			
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 5", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}


//**************************************************************************//
// Compile the shader file.  These files aren't pre-compiled (well, not		//
// here, and are compiled on he fly).										//
//**************************************************************************//
HRESULT CompileShaderFromFile(WCHAR* szFileName,		// File Name
	LPCSTR szEntryPoint,		// Namee of shader
	LPCSTR szShaderModel,		// Shader model
	ID3DBlob** ppBlobOut)	// Blob returned
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		WCHAR errorCharsW[200];
		if (pErrorBlob != NULL)
		{
			charStrToWideChar(errorCharsW, (char *)pErrorBlob->GetBufferPointer());
			MessageBox(0, errorCharsW, L"Error", 0);
		}
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);


	//**********************************************************************//
	// Compile the shader file.  These files aren't pre-compiled (well, not //
	// here, and are compiles on he fly).									//
	//																		//
	// This is DirectX11, but what shader model do you see here?			//
	// Change to shader model 5 in Babbage209 and it should still work.		//
	//**********************************************************************//
	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(L"Tutorial05 - Matrices.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	//**********************************************************************//
	// Create the vertex shader from the shader file we previously compiled.//
	//**********************************************************************//
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}


	//**********************************************************************//
	// Compile the pixel shader.  These files aren't pre-compiled (well, not//
	// here, and are compiles on he fly).									//
	//																		//
	// This is DirectX11, but what shader model do you see here?			//
	// Change to shader model 5 in Babbage209 and it should still work.		//
	//**********************************************************************//
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"Tutorial05 - Matrices.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}


	//**********************************************************************//
	// Create the pixel shader from the shader file we previously compiled.//
	//**********************************************************************//
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;




	//**********************************************************************//
	// Define the input layout.  I won't go too much into this except that  //
	// the vertex defined here MUST be consistent with the vertex shader	//
	// input you use in your shader file and the constand buffer structure  //
	// at the top of this module.											//
	//																		//
	// Here a vertex has a position and a colour.  No texture and no		//
	// normal vector (i.e. no lighting).									//
	//**********************************************************************//
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);



	//**************************************************************************//
	// Create a Cube.  Start off by defining 8 vertices to represent the 8		//
	// corners of the cube.														//
	//																			//
	// We call this a vertex buffer.											//
	//																			//
	// Each vertex is defined on a single line as a position and a colour.		//
	//																			//
	// NOTE there is only actually one cube, but that cube is drawn twice on two//
	// different places with two different sizes.								//
	//**************************************************************************//
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 8;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);



	//**************************************************************************//
	// Now define some triangles.  That's all DirectX allows us to draw.  This  //
	// is called an index buffer, and it indexes the vertices to make triangles.//
	// e.g. the first triangle is made of vertices 3, 1 and 0.					//
	//																			//
	// This is called an index buffer.											//
	//**************************************************************************//
	WORD indices[] =
	{
		3, 1, 0,
		2, 1, 3,

		0, 5, 4,
		1, 5, 0,

		3, 4, 7,
		0, 4, 3,

		1, 6, 5,
		2, 6, 1,

		2, 7, 6,
		3, 7, 2,

		6, 4, 5,
		7, 4, 6,
	};


	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	//**************************************************************************//
	// Create the constant buffer for our 3 matrices.							//
	//**************************************************************************//
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MatrixConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantBufferMatrices);
	if (FAILED(hr))
		return hr;







	//**************************************************************************//
	// Initialize the projection matrix.  Generally you will only want to create//
	// this matrix once and then forget it.										//
	//**************************************************************************//
	g_MatProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2,			// Field of view (pi / 2 radians, or 90 degrees
		width / (FLOAT)height, // Aspect ratio.
		0.01f,					// Near clipping plane.
		100.0f);				// Far clipping plane.

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();
	if (g_pConstantBufferMatrices) g_pConstantBufferMatrices->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{
	//**************************************************************************//
	// Update our time.  This block is supposed to make the movement frame rate //
	// independent, as the frame rate we get depends of the performance of our	//
	// computer.  We may even be in reference (software emulation) mode, which	//
	// is painfully slow.														//
	//**************************************************************************//
	static float t = 0.0f;
	if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}


	//**************************************************************************//
	// Initialize the view matrix.  What you do to the viewer matrix moves the  //
	// viewer, or course.														//
	//																			//
	// The viewer matrix is created every frame here, which looks silly as the	//
	// viewer never moves.  However in general your viewer does move.			//
	//**************************************************************************//
	XMVECTOR Eye = XMVectorSet(cameraX, cameraY, cameraZ, 0.0f);
	XMVECTOR At = XMVectorSet(cameraLookX, cameraLookY, cameraLookZ, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	g_MatView = XMMatrixLookAtLH(Eye,		// The eye, or viewer's position.
								At,		// The look at point.
								Up);		// Which way is up.

	//******************************************************************//
	// Create the world matrix for the 2nd cube: create many matrices,	//
	// one for each transformation (rotate, scale etc.) and multiply	//
	// then together.   This is C++, where you can "overload" the		//
	// assignment and arithmetic operators.   ...So you can just		//
	// multiply and add  matrices like they were any other variable.	//
	//																	//
	// The order you multiply your matrices matters, try changing it!	//
	//******************************************************************//    
	
	//create matrix for all cubes, moving and wall
	for (int i = 0; i < numOfCubes; i++) {
		mTranslationList[i] = XMMatrixTranslation(cubeCoors[i], cubeCoors[i+numOfCubes], cubeCoors[i+(numOfCubes*2)]);
		XMMATRIX mScale = XMMatrixScaling(cubeSizeX / 2, cubeSizeY / 2, cubeSizeZ / 2);
		XMMATRIX matWorld = mScale * mTranslationList[i];
		matWorlds[i] = matWorld;
	}
	
	// Clear the back buffer
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red, green, blue, alpha
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);


	// Clear the depth buffer to 1.0 (max depth)
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


	//******************************************************************//    
	// Update shader variables for the first cube.						//
	// we pass the parameters to it in a constant buffer.  The buffer	//
	// we define in this module MUST match the constant buffer in the   //
	// shader.															//
	//																	//
	// it would seem that the constant buffer we pass to the shader must//
	// be global, well defined on the heap anyway.  Not a local variable//
	// it would seem.													//
	//******************************************************************//
	

	//drawing loop for all cubes, expandable
	for (int i = 0; i < numOfCubes; i++) {
		cubeBuilders[i].mWorld = XMMatrixTranspose(matWorlds[i]);
		cubeBuilders[i].mView = XMMatrixTranspose(g_MatView);
		cubeBuilders[i].mProjection = XMMatrixTranspose(g_MatProjection);
		g_pImmediateContext->UpdateSubresource(g_pConstantBufferMatrices, 0, NULL, &cubeBuilders[i], 0, 0);

		g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBufferMatrices);
		g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
		g_pImmediateContext->DrawIndexed(36, 0, 0);
	}


	//******************************************************************//
	// Present our back buffer to our front buffer.  Swap buffers, in	//
	// otherwords.														//
	//******************************************************************//
	g_pSwapChain->Present(0, 0);
}


//**************************************************************************//
// Convert an old chracter (char *) string to a WCHAR * string.  There must//
// be something built into Visual Studio to do this for me, but I can't		//
// find it - Nigel.															//
//**************************************************************************//
void charStrToWideChar(WCHAR *dest, char *source)
{
	int length = strlen(source);
	for (int i = 0; i <= length; i++)
		dest[i] = (WCHAR)source[i];
}

void CheckKeyboard()
{
	kbQ = GetAsyncKeyState(0x51);
	kbE = GetAsyncKeyState(0x45);
	kbW = GetAsyncKeyState(0x57);
	kbA = GetAsyncKeyState(0x41);
	kbS = GetAsyncKeyState(0x53);
	kbD = GetAsyncKeyState(0x44);
	kbZ = GetAsyncKeyState(0x5A);
	kbX = GetAsyncKeyState(0x58);
	kbR = GetAsyncKeyState(0x52);
	kbPGUP = GetAsyncKeyState(0x21);
	kbPGDOWN = GetAsyncKeyState(0x22);
}

void ActionKeyboard() {

		if (kbZ) {
			cubeSpeed = 0.0025; //starts movement
		}
		if (kbX) {
			cubeSpeed = 0; //stops movement
			for (int i = 18; i < 21; i++) {
				cubeCoors[i] = -4.0f; //reset position
			}
		}
		if (kbQ) {
			cameraLookZ += 14; //flip view
			cameraLookQE = true;
		}
		if (kbE) {
			cameraLookZ -= 14; //flip view
			cameraLookQE = false;
		}
		if (kbW) {
			if (cameraLookQE) {
				cameraZ += cameraSpeed;
				cameraLookZ += cameraSpeed;
			}
			else {
				cameraZ -= cameraSpeed;
				cameraLookZ -= cameraSpeed;
			}
		}
		if (kbS) {
			if (cameraLookQE) {
				cameraZ -= cameraSpeed;
				cameraLookZ -= cameraSpeed;
			}
			else {
				cameraZ += cameraSpeed;
				cameraLookZ += cameraSpeed;
			}
		}
		if (kbA) {
			if (cameraLookQE) {
				cameraX -= cameraSpeed;
				cameraLookX -= cameraSpeed;
			}
			else {
				cameraX += cameraSpeed;
				cameraLookX += cameraSpeed;
			}
		}
		if (kbD) {
			if (cameraLookQE) {
				cameraX += cameraSpeed;
				cameraLookX += cameraSpeed;
			}
			else {
				cameraX -= cameraSpeed;
				cameraLookX -= cameraSpeed;
			}
		}
		if (kbPGUP) {
			cameraSpeed += 0.00002;
		}
		if (kbPGDOWN && cameraSpeed >= 0.004) {
			cameraSpeed -= 0.00002;
		}
		if (kbR) {
			CubeReset(); // set cubes back to original positions to rebuild wall
		}
}

//check all collisions between cubes and  process them based on angle and force
void CheckCollisions() {

		//check for other blocks nearby to slow down blocks
		for (int i = 0; i < 9; i++) {
			int cubesAbsent = 0; //how many cubes are not around current
			for (int n = 0; n < 9; n++) {
				if (i != n) {
					if (cubeCoors[i] - cubeCoors[n] >= cubeSizeX || cubeCoors[i] - cubeCoors[n] <= -cubeSizeX
						|| cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes] >= cubeSizeY || cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes] <= -cubeSizeY	// check x, y, z if any are true cube
						|| cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes*2] >= cubeSizeZ || cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes*2] <= -cubeSizeZ) {	// is not touching another and may fall
						cubesAbsent += 1;
					}
				}
			}
			if (cubesAbsent == 8 && cubeMoveY[i] < 0.0075) { cubeMoveY[i] += cubeFallAcc; } //limits max speed
			else { cubeMoveY[i] = 0; }
		}

		//check if moving blocks hit the wall
		for (int i = 0; i < 3; i++) {
			for (int n = 3; n < numOfCubes; n++) {
				if ((cubeCoors[i] - cubeCoors[n] < cubeSizeX && cubeCoors[i] - cubeCoors[n] > -cubeSizeX)
					&& (cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes] < cubeSizeY && cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes] > -cubeSizeY) //if moving cube hits a wall cube
					&& (cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes * 2] < cubeSizeZ && cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes * 2] > -cubeSizeZ)) {

					std::vector<float> forces = forceCalculator(i, n); //get forces for cubes

					cubeMoveX[n] = forces[0]; //add forces to movements
					cubeMoveY[n] = forces[1];
					cubeMoveZ[n] = forces[2];
				}
			}
		}

		//check if wall blocks bump into each other
		for (int i = 3; i < 9; i++) {
			for (int n = 3; n < 9; n++) {
				if (i != n) { // dont check cube against its self
					if ((cubeCoors[i] - cubeCoors[n] < cubeSizeX && cubeCoors[i] - cubeCoors[n] > -cubeSizeX)
						&& (cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes] < cubeSizeY && cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes] > -cubeSizeY) 
						&& (cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes * 2] < cubeSizeZ && cubeCoors[i + numOfCubes] - cubeCoors[n + numOfCubes * 2] > -cubeSizeZ)) {
						
						std::vector<float> forces = forceCalculator(i, n); //get forces for cubes

						//set applied forces, all collisions are elastic, so no energy lose.
						//total net force split evenly between cubes
						cubeMoveX[i] = (forces[0] / 2);
						cubeMoveX[n] = (forces[0] / 2);
						cubeMoveY[i] = (forces[1] / 2);
						cubeMoveY[n] = (forces[1] / 2);
						cubeMoveZ[i] = (forces[2] / 2);
						cubeMoveZ[n] = (forces[2] / 2);

					}
				}
			}
		}
}



void AddMovements() {	
	for (int i = 3; i < 9; i++) { //adjust movement additions and add movements onto cubes
		cubeMoveX[i] *= cubeDrag;
		cubeMoveZ[i] *= cubeDrag;
		cubeCoors[i] += cubeMoveX[i];
		cubeCoors[i+numOfCubes] -= cubeMoveY[i];
		cubeCoors[i+(numOfCubes*2)] += cubeMoveZ[i];
	}
}

void CubeReset() {
	for (int i = 0; i < numOfCubes * 3; i++) {
		cubeCoors[i] = cubeCoorsOriginal[i];
	}
}

void CheckOutOfBounds() {
	for (int i = 18; i <= 20; i++) { // check moving cubes z so they respawn at start
		if (cubeCoors[i] > 4.0f) {
			cubeCoors[i] = 4.0f;
		}
	}
	for (int i = 12; i < 18; i++) { // check height to make sure blocks dont fall to low
		if (cubeCoors[i] < 0) {
			cubeCoors[i] = 0;
			cubeMoveY[i-numOfCubes] = 0;
		}
	}
}

void RenderText() {

	////create font object
	//ID3DXFont *m_font = 0;
	//D3DXCreateFont(g_pd3dDevice, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &m_font);

	////Create a colour for the text
	//D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 0, 0, 255);

	////Create a rectangle to indicate where on the screen it should be drawn
	//RECT rct;
	//rct.left = 2;
	//rct.right = 780;
	//rct.top = 10;
	//rct.bottom = rct.top + 20;

	////Draw text 
	//m_font->DrawText(NULL, _T("Hello World"), -1, &rct, 0, fontColor);
	//m_font->Release(); //release object
}

//calculate force in each direction
std::vector<float> forceCalculator(int firstCube, int secondCube) {

	std::vector<float> forces;

	if (firstCube >= 3) { // if the cubes are to do with wall cubes only

		std::vector<float> angles = AngleCalculator(firstCube, secondCube);
		std::vector<float> angles1 = AngleCalculator(secondCube, firstCube);

		//this calculates the forces applied to each cube when it comes into constact eg 5 -> vs <- 3 has a net of 2 ->
		forces.push_back((cubeMoveX[firstCube] * (angles[0] / 90)) +  (cubeMoveX[secondCube] * (angles1[0] / 90)));
		forces.push_back((cubeMoveY[firstCube] * (angles[1] / 90)) + (cubeMoveY[secondCube] * (angles1[1] / 90)));
		forces.push_back((cubeMoveZ[firstCube] * (angles[2] / 90)) + (cubeMoveZ[secondCube] * (angles1[2] / 90)));

	}
	else {
		//get the force for the cube
		float force = cubeForce[firstCube];

		std::vector<float> angles = AngleCalculator(firstCube, secondCube);

		//calculate portion of force and add to forces
		for (int i = 0; i < 3; i++) {
			forces.push_back(cubeSpeed*force*(angles[i] / 90));
		}
	}

	return forces;

}

//calculate angles of axis between cubes
std::vector<float> AngleCalculator(int firstCube, int secondCube) {  
	
	std::vector<float> angles;

	//x angle
	angles.push_back(tan((cubeCoors[firstCube] - cubeCoors[secondCube]) / 0.6));

	//y angle
	angles.push_back(tan( (cubeCoors[firstCube+numOfCubes]-cubeCoors[secondCube+numOfCubes])/0.6));

	//z angle
	angles.push_back(tan((cubeCoors[firstCube + (numOfCubes*2)] - cubeCoors[secondCube + (numOfCubes*2)]) / 0.6));

	return angles;
}