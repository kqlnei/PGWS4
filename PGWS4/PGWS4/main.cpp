﻿#include <Windows.h>//ウィンドウの作成や操作、メッセージ処理などのOSレベルの操作
#include <tchar.h>//_Tマクロを利用しマルチバイト文字セットとUnicode文字セットの両方をサポートするため
#include <d3d12.h>
#include <dxgi1_6.h> 
#include <vector>
#ifdef _DEBUG //デバッグビルド時にのみ _DEBUG 起動
#include <iostream>
#endif

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")

using namespace std;

// @brief コンソール画面にフォーマット付き文字列を表示
// @param format フォーマット (%d とか %f とかの)
// @param 可変長引数
// @ remarks この関数はデバッグ用です。デバッグ時にしか動作しない
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;//可変長引数のリストを管理
	va_start(valist, format);//valistを初期化し、追加の引数へのアクセスを可能にする
	printf(format, valist);
	va_end(valist);
#endif
}

//ウィンドウに送られるメッセージを処理する関数
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);// OSに対して「もうこのアプリは終わる」と伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//既定の処理を行う
}

//デバッグモードではコンソールアプリケーション用の main 関数がエントリーポイント
//リリースモードではGUIアプリケーションのエントリーポイントである WinMain
#ifdef _DEBUG
int main()
{
	#else
	int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
	{
	#endif
	const unsigned int window_width = 1280;//ウィンドウの幅
	const unsigned int window_Height = 720;//ウィンドウの高

	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;
	ID3D12CommandQueue* _cmdQueue = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	//ウィンドウクラスの生成＆登録
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);//構造体のサイズを指定
	w.lpfnWndProc = (WNDPROC)WindowProcedure;//コールバック関数の指定(メッセージ処理用)
	w.lpszClassName = _T("DX12Sample");//アプリケーションクラス名（今後ライブラリを作るときにそれ用の名前に変える）
	w.hInstance = GetModuleHandle(nullptr);//ハンドルの取得

	RegisterClassEx(&w);//ウィンドウサイズを決める

	RECT wrc = { 0, 0, window_width, window_Height };//描画可能領域のサイズを設定

	//関数を使ってウィンドウのサイズを補正する(これにより、タイトルバーや境界線の分を考慮したサイズに)
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	//ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow(w.lpszClassName,//クラス名指定
		_T("DX12 テスト"),//タイトルバー文字
		WS_OVERLAPPEDWINDOW,//タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT,//表示 x 座標は OS におまかせ
		CW_USEDEFAULT,//表示 y 座標は OS におまかせ
		wrc.right - wrc.left,//ウィンドウ幅
		wrc.bottom - wrc.top,//ウィンドウ高
		nullptr,//親ウィンドウハンドル
		nullptr,//メニューハンドル
		w.hInstance,//呼び出しアプリケーションハンドル
		nullptr);//追加パラメータ

	HRESULT D3D12CreateDevice(
		IUnknown * pAdapter, //ひとまずは nullptr でOK
		D3D_FEATURE_LEVEL MinimumFeatureLevel, //最低限必要なフューチャーレベル
		REFIID riid,
		void** ppDevice
	);

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	HRESULT CreateSwapChainForHwnd(
		IUnknown * pDevice,
		HWND hWnd,
		const DXGI_SWAP_CHAIN_DESC1 * pDesc,
		const DXGI_SWAP_CHAIN_FULLSCREEN_DESC * pFullscreenDesc,
		IDXGIOutput * pRestricToOutput,
		IDXGISwapChain1 * ppSwapChain
	);



	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	//アダプターの列挙用
	std::vector <IDXGIAdapter*> adapters;
	//ここに特定の名前を持つアダプターオブジェクトが入る
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);//アダプターの説明オブジェクト取得
		std::wstring strDesc = adesc.Description;
		//探したいアダプターの名前を確認
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	//Direct3Dデバイスの初期化
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = lv;
			break;//生成可能なバージョンが見つかったらループを打ち切り
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr,
		IID_PPV_ARGS(&_cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	cmdQueueDesc.NodeMask = 0;

	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));


	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_Height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;

	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	swapchainDesc.SwapEffect = 


	ShowWindow(hwnd, SW_SHOW);

	//メッセージループ
	while (true)
	{
		MSG msg;//メッセージの種類やウィンドウハンドル、送信者などの情報
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			//アプリケーションが終わるときに message が　WM_QUIT　になる
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//もうこのクラスは使わないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}