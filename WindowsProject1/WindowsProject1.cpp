#include "framework.h"
#include "WindowsProject1.h"

#include <d2d1.h>
#include <Wincodec.h>

#include <string>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "D2D1.lib")

using namespace D2D1;

RECT WndSize;

ID2D1Factory *gp_factory;
ID2D1HwndRenderTarget *gp_render_target;

ID2D1Bitmap *gp_bitmap;

D2D1_RECT_F g_image_rect;

POINT pre_cursor;
bool mouse_clicked = false;

bool is_falling = false;
bool is_jumping = false;
bool on_ground = false;
bool is_working = true;

int motion = 0;

int direction = 1;
int animation_number = 0;

int moving_frame = 0;
int falling_frame = 0;

int vx=0, vy=0; 

int LoadMyImage(ID2D1RenderTarget* ap_target, const wchar_t* ap_path, enum WICBitmapTransformOptions option)
{
	if (gp_bitmap != NULL) {
		gp_bitmap->Release();
		gp_bitmap = NULL;
	}

	IWICImagingFactory *p_wic_factory;
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory, 
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_PPV_ARGS(&p_wic_factory));

	IWICBitmapDecoder *p_decoder = NULL;
	IWICBitmapFrameDecode *p_frame = NULL;
	IWICBitmapFlipRotator* p_rotator = NULL;
	IWICFormatConverter *p_converter = NULL;

	hr = p_wic_factory->CreateDecoderFromFilename(ap_path,
		NULL, 
		GENERIC_READ, 
		WICDecodeMetadataCacheOnDemand, 
		&p_decoder);

	if (SUCCEEDED(hr)) {
		hr = p_decoder->GetFrame(0, &p_frame);
	}

	if (SUCCEEDED(hr)) {
		hr = p_wic_factory->CreateFormatConverter(&p_converter);
	}

	if (SUCCEEDED(hr)) {
		hr = p_converter->Initialize(p_frame,
			GUID_WICPixelFormat32bppPBGRA, 
			WICBitmapDitherTypeNone, 
			NULL, 
			0.0f, 
			WICBitmapPaletteTypeCustom);
	}

	if (SUCCEEDED(hr)) {
		hr = p_wic_factory->CreateBitmapFlipRotator(&p_rotator);
	}

	if (SUCCEEDED(hr)) {
		hr = p_rotator->Initialize(p_converter, option);
	}

	if (SUCCEEDED(hr)) {
		hr = ap_target->CreateBitmapFromWicBitmap(p_rotator, NULL, &gp_bitmap);
	}

	if (p_rotator) p_rotator->Release();
	if (p_converter) p_converter->Release();
	if (p_frame) p_frame->Release();
	if (p_decoder) p_decoder->Release();
	if (p_wic_factory) p_wic_factory->Release();

	return SUCCEEDED(hr);
}

void CALLBACK OnTimer(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime) {
	if (nIDEvent == 1) {
		if (!mouse_clicked) {
			RECT r;
			GetWindowRect(hWnd, &r);

			if (vy <= 0 && !is_jumping) {
				is_jumping = true;
				animation_number = 22;
			}
			if (vy > 0 && !is_falling) {
				is_falling = true;
				falling_frame = 0;
				animation_number = 4;
			}

			if (WndSize.bottom >= r.bottom + vy) SetWindowPos(hWnd, HWND_TOPMOST, r.left + vx, r.top + vy, 0, 0, SWP_NOSIZE);
			else {
				SetWindowPos(hWnd, HWND_TOPMOST, r.left, WndSize.bottom - r.bottom + r.top, 0, 0, SWP_NOSIZE);
				on_ground = true;
			}
			
			if (WndSize.left > r.left + vx || WndSize.right < r.right + vx) { vx *= -1, direction *= -1; }
			if(vy < 10) vy += 1;

			if (on_ground) {
				if (falling_frame == 0) animation_number = 18;
				if (falling_frame == 15) animation_number = 19;
				if (falling_frame == 30) animation_number = 1;
				if (falling_frame++ == 35) is_working = false;
			}

			if (!is_working) {
				if (motion == 1) {
					if (moving_frame % 100 == 0) animation_number = 1;
					if (moving_frame % 100 == 25) animation_number = 2;
					if (moving_frame % 100 == 50) animation_number = 3;
					if (moving_frame % 100 == 75) animation_number = 2;

					if (r.left + direction < WndSize.left) direction = 1;
					if (r.right + direction > WndSize.right) direction = -1;

					SetWindowPos(hWnd, HWND_TOPMOST, r.left + direction, r.top, 0, 0, SWP_NOSIZE);
					if (++moving_frame == 300) { animation_number = 1;  motion = 0; }
				}
				if (motion == 2) {
					if (moving_frame % 100 == 0) animation_number = 20;
					if (moving_frame % 100 == 75) animation_number = 21;

					if (r.left + direction < WndSize.left) direction = 1;
					if (r.right + direction > WndSize.right) direction = -1;

					if(moving_frame % 100 < 50) SetWindowPos(hWnd, HWND_TOPMOST, r.left + 2*direction, r.top, 0, 0, SWP_NOSIZE);
					if (++moving_frame == 300) { animation_number = 1;  motion = 0; }
				}
				if (motion == 3) {
					if (moving_frame % 100 == 0) animation_number = 15;
					if (moving_frame % 100 == 40) animation_number = 16;
					if (moving_frame % 100 == 80) animation_number = 17;

					if (++moving_frame == 300) { animation_number = 1;  motion = 0; }
				}
			}
		}
	}
	if (nIDEvent == 2) {
		if (!mouse_clicked && !is_working) {
			moving_frame = 0;
			srand(time(0));
			if (rand() % 2 == 0) direction = 1;
			else direction = -1;
			int num = rand() % 10;
			if (num < 6) motion = 1;
			else if (num < 8) motion = 2;
			else if (num < 9) motion = 3;
		}
	}
	if (nIDEvent == 3) {
		if (animation_number) {
			enum WICBitmapTransformOptions option;
			if (direction == 1) option = WICBitmapTransformFlipHorizontal;
			if (direction == -1) option = WICBitmapTransformRotate0;

			std::wstring s_path = L"";
			s_path = L"picture/cat" + std::to_wstring(animation_number) + L".png";
			const wchar_t* path = s_path.c_str();
			LoadMyImage(gp_render_target, path, option);

			animation_number = 0;
			InvalidateRect(hWnd, NULL, TRUE);
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_LBUTTONDOWN) {

		GetCursorPos(&pre_cursor);
		SetCapture(hWnd);
		mouse_clicked = true;
		is_falling = false;
		is_jumping = false;
		on_ground = false;
		is_working = true;
		motion = 0;

	} else if (uMsg == WM_LBUTTONUP) {

		if (vx > 0) direction = 1;
		if (vx < 0) direction = -1;
		if (abs(vx) > 20) vx = 20 * direction;

		mouse_clicked = false;
		ReleaseCapture();
	} else if (uMsg == WM_MOUSEMOVE) {

		if (mouse_clicked) {
			POINT temp_cursor;
			GetCursorPos(&temp_cursor);

			vx = (vx+(temp_cursor.x - pre_cursor.x)/2)/2;
			vy = (vy+(temp_cursor.y - pre_cursor.y)*1.5)/2;

			if (10 < vx) animation_number = 9;
			else if (5 < vx) animation_number = 7;
			else if (0 < vx) animation_number = 5;
			else if (vx == 0) animation_number = 1;
			else if(-5 < vx) animation_number = 6;
			else if(-10 < vx) animation_number = 8;
			else animation_number = 10;
			
			RECT r;
			GetWindowRect(hWnd, &r);
			if(r.left + temp_cursor.x - pre_cursor.x < WndSize.left) SetWindowPos(hWnd, HWND_TOPMOST, WndSize.left, r.top + temp_cursor.y - pre_cursor.y, 0, 0, SWP_NOSIZE);
			else if(r.right + temp_cursor.x - pre_cursor.x > WndSize.right) SetWindowPos(hWnd, HWND_TOPMOST, WndSize.right - r.right + r.left, r.top + temp_cursor.y - pre_cursor.y, 0, 0, SWP_NOSIZE);
			else SetWindowPos(hWnd, HWND_TOPMOST, r.left + temp_cursor.x - pre_cursor.x, r.top + temp_cursor.y - pre_cursor.y, 0, 0, SWP_NOSIZE);
			pre_cursor = temp_cursor;
		}
	} else if (uMsg == WM_PAINT) {
		ValidateRect(hWnd, NULL);
		gp_render_target->BeginDraw();
		gp_render_target->Clear();
		if (gp_bitmap != NULL) gp_render_target->DrawBitmap(gp_bitmap, &g_image_rect);
		else gp_render_target->Clear();
		
		gp_render_target->EndDraw();
		return 0;
	} else if (uMsg == WM_CREATE) {
		SystemParametersInfoA(SPI_GETWORKAREA, 0, &WndSize, 0);

		SetTimer(hWnd, 1, 10, OnTimer);
		SetTimer(hWnd, 2, 10000, OnTimer);
		SetTimer(hWnd, 3, 10, OnTimer);

		RECT r;
		GetClientRect(hWnd, &r);

		gp_factory->CreateHwndRenderTarget(
			RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)),
			HwndRenderTargetProperties(hWnd, SizeU(r.right,r.bottom)),
			&gp_render_target);

		g_image_rect.right = (float)r.right;
		g_image_rect.bottom = (float)r.bottom;

		LoadMyImage(gp_render_target, L"picture/cat1.png", WICBitmapTransformRotate0);
		return 0;
	} else if (uMsg == WM_DESTROY) {

		KillTimer(hWnd, 1);
		KillTimer(hWnd, 2);
		KillTimer(hWnd, 3);

		if (gp_render_target != NULL) {
			gp_render_target->Release();
			gp_render_target = NULL;
		}

		PostQuitMessage(0);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpCmdLine, int nCmdShow)
{

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &gp_factory);

	WNDCLASS wc;
	wchar_t m_class_name[] = L"cat";

	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hbrBackground = (HBRUSH)NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = m_class_name;
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);
	
	HWND hWnd = CreateWindow(m_class_name, L"cat",
		WS_EX_LAYERED | WS_POPUP, 400, -200, 128, 128, NULL, NULL, hInstance, NULL);
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, RGB(254, 254, 254), 255, LWA_COLORKEY);
	
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (gp_bitmap != NULL) gp_bitmap->Release();
	gp_factory->Release();
	CoUninitialize();

	return (int)msg.wParam;
}
