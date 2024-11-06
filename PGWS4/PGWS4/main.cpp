﻿#include <Windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <iostream>
#endif

using namespace std;

// @brief コンソール画面にフォーマット付き文字列を表示
// @param format フォーマット (%d とか %f とかの)
// @param 可変長引数
// @ remarks この関数はデバッグ用です。デバッグ時にしか動作しない
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

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

#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
const unsigned int window_width = 1280;
const unsigned int window_Height = 720;

//ウィンドウクラスの生成＆登録
WNDCLASSEX w = {};

w.cbSize = sizeof(WNDCLASSEX);
w.lpfnWndProc = (WNDPROC)WindowProcedure;//コールバック関数の指定
w.lpszClassName = _T("DX12Sample");//アプリケーションクラス名（今後ライブラリを作るときにそれ用の名前に変える）
w.hInstance = GetModuleHandle(nullptr);//ハンドルの取得

RegisterClassEx(&w);//ウィンドウサイズを決める

RECT wrc = { 0, 0, window_width, window_Height };

//関数を使ってウィンドウのサイズを補正する
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

//ウィンドウ表示
ShowWindow(hwnd, SW_SHOW);

//もうこのクラスは使わないので登録解除する
UnregisterClass(w.lpszClassName, w.hInstance);

DebugOutputFormatString("Show window test.");

return 0;
}