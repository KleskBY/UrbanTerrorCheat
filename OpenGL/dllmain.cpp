#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <gl/GL.h>

#include "minhook/MinHook.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_win32.h"

#include "Renderer.h"
#include "Structs.h"
#include "Memory.h"

int MENUWIDTH = 300;
int MENUHEIGHT = 300;

HMODULE openglDll = nullptr;
HWND GameHWND = NULL;
WNDPROC wndproc;
uintptr_t cDll;

int Width = 1920;
int Height = 1080;

bool esp = true;
bool drawfov = true;
bool drawdot = true;
bool aimbot = true;
int bone = 1;
float aimfov = 15;
float aimsmooth = 1.5;
int SelectedItem = 0;
int MaxItems = 6;

View view;
float fovscale1 = 1.000000119;
float fovscale2 = 1.333333492;
std::vector<Vector3> Targets;
std::vector<float> TargetsDistance;
std::vector<Vector3> TargetsWS;

void* WglSwapBuffers;
void* dwglClear;
void* dwglBegin;

using fn_wgl_swap_buffers = bool(__stdcall*)(_In_ HDC);
fn_wgl_swap_buffers oWglSwapBuffers;
typedef void (APIENTRY* glClear_t)(GLbitfield mask);
glClear_t oWglClear;
typedef void (APIENTRY* glBegin_t)(GLenum);
glBegin_t oGlBegin;

bool init = false;
bool Unhook = false;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool ShowMenu = true;
ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings;
ImFont* font_default;

void GetStyle()
{
	font_default = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f, NULL);

	ImGuiStyle* style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);
	style->Colors[ImGuiCol_TextDisabled] = ImColor(53, 53, 53, 255);
	style->Colors[ImGuiCol_WindowBg] = ImColor(25, 25, 25, 255);
	style->Colors[ImGuiCol_ChildWindowBg] = ImColor(25, 25, 25, 255);
	style->Colors[ImGuiCol_PopupBg] = ImColor(25, 25, 25, 255);
	style->Colors[ImGuiCol_Border] = ImColor(36, 35, 35, 255);
	style->Colors[ImGuiCol_FrameBg] = ImColor(31, 31, 31, 255);
	style->Colors[ImGuiCol_FrameBgHovered] = ImColor(230, 0, 0, 255);
	style->Colors[ImGuiCol_FrameBgActive] = ImColor(255, 0, 0, 175);
	style->Colors[ImGuiCol_TitleBg] = ImColor(31, 31, 31, 255);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0, 255);
	style->Colors[ImGuiCol_TitleBgActive] = ImColor(22, 22, 22, 255);
	style->Colors[ImGuiCol_MenuBarBg] = ImColor(25, 25, 25, 255);
	style->Colors[ImGuiCol_ScrollbarBg] = ImColor(20, 20, 20, 255);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImColor(242, 0, 0, 232);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(241, 0, 0, 174);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(255, 0, 0, 133);
	style->Colors[ImGuiCol_CheckMark] = ImColor(255, 0, 0, 253);
	style->Colors[ImGuiCol_SliderGrab] = ImColor(246, 0, 0, 255);
	style->Colors[ImGuiCol_SliderGrabActive] = ImColor(255, 0, 0, 175);
	style->Colors[ImGuiCol_Button] = ImColor(13, 13, 13, 249);
	style->Colors[ImGuiCol_ButtonHovered] = ImColor(255, 0, 0, 223);
	style->Colors[ImGuiCol_ButtonActive] = ImColor(0, 0, 0, 168);
	style->Colors[ImGuiCol_Header] = ImColor(255, 0, 0, 175);
	style->Colors[ImGuiCol_HeaderHovered] = ImColor(244, 0, 0, 223);
	style->Colors[ImGuiCol_HeaderActive] = ImColor(255, 0, 0, 255);
	style->Colors[ImGuiCol_Separator] = ImColor(0, 0, 0, 78);

	style->FrameBorderSize = 1;
	style->FrameRounding = 0;
	style->GrabRounding = 0;
	style->WindowRounding = 0;
	style->ScrollbarRounding = 0;
	style->ScrollbarSize = 0;
	style->AntiAliasedLines = false;
	style->AntiAliasedFill = false;
}


void GuiItem(const char* String, int number, bool variable, int x, int y)
{
	if (SelectedItem == number) 
	{
		RenderRectFilled(ImVec2(0, y - 4), ImVec2(MENUWIDTH, y + 24), ImVec4(0.6f, 0.f, 0.f, 0.8f), false, 0);
		RenderText(">", ImVec2(x, y), 20.f, ImVec4(0.1f, 1.f, 0.1f, 1.f), false, font_default);
		RenderText(String, ImVec2(x+15.f, y), 20.f, ImVec4(1.f, 1.f, 1.f, 1.f), false, font_default);
	}
	else RenderText(String, ImVec2(x, y), 20.f, ImVec4(0.5f, 0.5f, 0.5f, 0.9f), false, font_default);
	if (variable) RenderText("ON", ImVec2(200, y), 20.f, ImVec4(0.f, 1.f, 0.f, 1.f), false, font_default);
	else RenderText("OFF", ImVec2(200, y), 20.f, ImVec4(1.f, 0.f, 0.f, 1.f), false, font_default);
}
void GuiItemNum(const char* String, int number, float variable, int x, int y)
{
	if (SelectedItem == number)
	{
		RenderRectFilled(ImVec2(0, y - 4), ImVec2(MENUWIDTH, y + 24), ImVec4(0.6f, 0.f, 0.f, 0.8f), false, 0);
		RenderText(">", ImVec2(x, y), 20.f, ImVec4(0.1f, 1.f, 0.1f, 1.f), false, font_default);
		RenderText(String, ImVec2(x + 15.f, y), 20.f, ImVec4(1.f, 1.f, 1.f, 1.f), false, font_default);
	}
	else RenderText(String, ImVec2(x, y), 20.f, ImVec4(0.5f, 0.5f, 0.5f, 0.9f), false, font_default);

	std::string VarText = std::to_string(variable);
	if (variable >= 10) 	VarText = VarText.substr(0, VarText.size() - 5);
	else VarText = VarText.substr(0, VarText.size() - 4);

	RenderText(VarText, ImVec2(200, y), 20.f, ImVec4(0.f, 1.f, 0.f, 1.f), false, font_default);
}

Vector3 W2S(Vector3 WorldPos)
{
	Vector3 screen = Vector3(0, 0, 0);
	Vector3 vector = Vector3(0, 0, 0);
	Vector3 worldLocation = WorldPos - view.vOrigin;
	vector.x = worldLocation.Dot(view.vRight);
	vector.y = worldLocation.Dot(view.vUpward);
	vector.z = worldLocation.Dot(view.vForward);
	if ((double)vector.z >= 0.01)
	{
		screen.x = (float)(Width / 2) + (float)(Width / 2) / vector.z * fovscale1 * vector.x;
		screen.y = (float)(Height / 2) - (float)(Height / 2) / vector.z * fovscale2 * vector.y;
		screen.x = Width - screen.x;
		return screen;
	}
	else return Vector3(0, 0, 0);
}
float GetDistance(float Xx, float Yy, float xX, float yY)
{
	return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
}

bool __stdcall hkWglSwapBuffers(_In_ HDC hdc)
{
	if (GetAsyncKeyState(VK_END)) Unhook = true;
	if (!Unhook)
	{
		if (!init)
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			ImGui_ImplWin32_Init(GameHWND);
			ImGui_ImplOpenGL2_Init();
			GetStyle();
			init = true;
		}
		else
		{
			ImGui_ImplWin32_NewFrame();
			ImGui_ImplOpenGL2_NewFrame();
			ImGui::NewFrame();

			ImGuiIO& io = ImGui::GetIO();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
			ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });
			ImGui::Begin("##Backbuffer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

			
			Width = io.DisplaySize.x;
			Height = io.DisplaySize.y;

			fovscale1 = getValueFloat(cDll + 0xC254DC);
			fovscale2 = getValueFloat(cDll + 0xC254F0);
			
			view.vOrigin = getValueVector(cDll + 0xC2521C);
			view.vOrigin.z = view.vOrigin.z - 32.f;
			view.vForward = getValueVector(cDll + 0xC25228);
			view.vRight = getValueVector(cDll + 0xC25234);
			view.vUpward = getValueVector(cDll + 0xC25240);

			UINT BestTarget = -1;
			DOUBLE fClosestPos = 99999;
			float ScreenCenterX = Width / 2;
			float ScreenCenterY = Height / 2;
			float radiusx = aimfov * (ScreenCenterX / 100);
			float radiusy = aimfov * (ScreenCenterY / 100);

			TargetsWS.clear();
			for (int i = 0; i < 40; i++)
			{
				Vector3 Enemy = getValueVector(cDll + 0xB2EB70 + 14 * sizeof(float) * i);
				Vector3 scren = W2S(Enemy);
				Vector3 aimpoint = Vector3(0, 0, 0);
				if (bone == 0) aimpoint = W2S(Vector3(Enemy.x, Enemy.y, Enemy.z));
				if (bone == 1) aimpoint = W2S(Vector3(Enemy.x, Enemy.y, Enemy.z-8.f));
				if (bone == 2) aimpoint = W2S(Vector3(Enemy.x, Enemy.y, Enemy.z - 15.f));
				TargetsWS.push_back(Vector3(aimpoint.x, aimpoint.y, 0));
				if (Enemy.x == 0) continue;
				float dist = view.vOrigin.DistTo(Enemy);
				dist = dist * 0.0254;

				if (dist > 1.6f && scren.x >4.f)
				{
					Vector3 screen2 = W2S(Vector3(Enemy.x, Enemy.y, Enemy.z - 50));
					int boxheight = (screen2.y - scren.y) * 1.25;
					if (drawdot) RenderLine(ImVec2(aimpoint.x - 1, aimpoint.y - 1), ImVec2(aimpoint.x + 1, aimpoint.y + 1), ImVec4(1.f, 0, 0, 1.f), 2);
					if (esp )RenderRect(ImVec2(scren.x-boxheight/4, scren.y - boxheight * 0.15), ImVec2(scren.x + boxheight / 4, scren.y + boxheight), ImVec4(1.f, 1.f, 1.f, 0.85f), 2.f, ImDrawCornerFlags_All, 2.f);				}

				if (aimbot)
				{
					if (aimpoint.x < 100) continue;
					float CrosshairDistance = GetDistance(aimpoint.x, aimpoint.y, ScreenCenterX, ScreenCenterY);
					if (CrosshairDistance > 200) continue;

					if (aimpoint.x >= ScreenCenterX - radiusx && aimpoint.x <= ScreenCenterX + radiusx && aimpoint.y >= ScreenCenterY - radiusy && aimpoint.y <= ScreenCenterY + radiusy)
					{
						if (CrosshairDistance < fClosestPos)
						{
							fClosestPos = CrosshairDistance;
							BestTarget = i;
						}
					}
				}

				setValue(cDll + 0xB2EB70 + 14 * sizeof(float) * i, Vector3(0, 0, 0));
			}
			if (BestTarget != -1 && aimbot)
			{
				if (TargetsWS[BestTarget].x > 4.f)
				{
					double DistX = (double)TargetsWS[BestTarget].x - ScreenCenterX;
					double DistY = (double)TargetsWS[BestTarget].y - ScreenCenterY;

					DistX /= aimsmooth;
					DistY /= aimsmooth;

					if (GetAsyncKeyState(VK_MENU) & 0x8000) mouse_event(MOUSEEVENTF_MOVE, (int)DistX, (int)DistY, NULL, NULL);
				}
			}
			if(drawfov) RenderRect(ImVec2(ScreenCenterX - radiusx, ScreenCenterY - radiusy), ImVec2(ScreenCenterX + radiusx, ScreenCenterY + radiusy), ImVec4(1.f, 1.f, 1.f, 1.f), 4.f, ImDrawCornerFlags_All, 1.f);
			

			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (ShowMenu)
			{
				window->DrawList->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(300, 260), IM_COL32(240, 0, 0, 200), IM_COL32(240, 0, 0, 200), IM_COL32(20, 0, 0, 200), IM_COL32(20, 0, 0, 200));
				window->DrawList->AddRectFilled(ImVec2(0, 0), ImVec2(300, 22), IM_COL32(250, 10, 10, 255));
				ImGui::Dummy(ImVec2(50, 0)); ImGui::SameLine();
				ImGui::Text("INTERIUM - KleskBY 2019");
				GuiItem("Esp", 0, esp, 10, 25);
				GuiItem("Draw FOV", 1, drawfov, 10, 50);
				GuiItem("Aim dot", 2, drawdot, 10, 75);
				GuiItem("Aimbot", 3, aimbot, 10, 100);
				GuiItemNum("Aim bone", 4, bone, 10, 125);
				GuiItemNum("Aim FOV", 5, aimfov, 10, 150);
				GuiItemNum("Aim smooth", 6, aimsmooth, 10, 175);
			}

			ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Once);
			ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Once);

			window->DrawList->PushClipRectFullScreen();

			ImGui::End();
			ImGui::PopStyleColor();
			ImGui::PopStyleVar(2);

			ImGui::Render();
			ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		}
	}

	return oWglSwapBuffers(hdc);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		while (!(openglDll = GetModuleHandleA("opengl32.dll")))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		GameHWND = FindWindowA("Quake3-UrT", NULL);

		while (!GameHWND) std::this_thread::sleep_for(std::chrono::milliseconds(300));

		WglSwapBuffers = GetProcAddress(openglDll, "wglSwapBuffers");
		cDll = (uintptr_t)GetModuleHandle(TEXT("Quake3-UrT.exe"));

		MH_Initialize();
		MH_CreateHook(WglSwapBuffers, hkWglSwapBuffers, reinterpret_cast<void**>(&oWglSwapBuffers));
		MH_EnableHook(WglSwapBuffers);

		while (!Unhook)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
			if (ShowMenu)
			{
				if (GetAsyncKeyState(VK_UP))
				{
					if (SelectedItem > 0) SelectedItem -= 1;
					else if (SelectedItem == 0)SelectedItem = MaxItems;
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				if (GetAsyncKeyState(VK_DOWN))
				{
					if (SelectedItem < MaxItems)  SelectedItem += 1;
					else if (SelectedItem == MaxItems)SelectedItem = 0;
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				if (GetAsyncKeyState(VK_RIGHT))
				{
					if (SelectedItem == 0) esp = true;
					if (SelectedItem == 1) drawfov = true;
					if (SelectedItem == 2) drawdot = true;
					if (SelectedItem == 3) aimbot = true;
					if (SelectedItem == 4) if (bone < 2) bone += 1;
					if (SelectedItem == 5) if (aimfov < 15) aimfov += 0.25;
					if (SelectedItem == 6) if (aimsmooth < 25) aimsmooth += 0.25;

					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				if (GetAsyncKeyState(VK_LEFT))
				{
					if (SelectedItem == 0) esp = false;
					if (SelectedItem == 1) drawfov = false;
					if (SelectedItem == 2) drawdot = false;
					if (SelectedItem == 3) aimbot = false;
					if (SelectedItem == 4) if (bone > 0) bone -= 1;
					if (SelectedItem == 5) if (aimfov > 1) aimfov -= 0.25;
					if (SelectedItem == 6) if (aimsmooth > 1) aimsmooth -= 0.25;

					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
			if (GetAsyncKeyState(VK_INSERT))
			{
				ShowMenu = !ShowMenu;
				std::this_thread::sleep_for(std::chrono::milliseconds(250));
			}
		}

		ImGui::DestroyContext();
		ImGui_ImplOpenGL2_DestroyDeviceObjects();
		MH_RemoveHook(WglSwapBuffers);
		MH_Uninitialize();
		FreeLibraryAndExitThread(hModule, 1);
	}
    return TRUE;
}

