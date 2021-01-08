// LibUniWinC.cpp

#include "pch.h"
#include "libuniwinc.h"


static HWND hTargetWnd_ = NULL;
static WINDOWINFO originalWindowInfo;
static WINDOWPLACEMENT originalWindowPlacement;
static HWND hParentWnd_ = NULL;
static BOOL bExpedtDesktopWnd = FALSE;
static HWND hDesktopWnd_ = NULL;
static SIZE szOriginaiBorder_;
static POINT ptVirtualScreen_;
static SIZE szVirtualScreen_;
static INT nPrimaryMonitorHeight_;
static BOOL bIsTransparent_ = FALSE;
static BOOL bIsBorderless_ = FALSE;
static BOOL bIsTopmost_ = FALSE;
static BOOL bIsBottommost_ = FALSE;
static BOOL bIsBackground_ = FALSE;
static BOOL bIsClickThrough_ = FALSE;
static BOOL bAllowDropFile_ = FALSE;
static COLORREF dwKeyColor_ = 0x00000000;		// AABBGGRR
static TransparentType nTransparentType_ = TransparentType::Alpha;
static TransparentType nCurrentTransparentType_ = TransparentType::Alpha;
static INT nMonitorCount_ = 0;							// ���j�^���B���j�^�𑜓x�ꗗ�擾���͈ꎞ�I��0�ɖ߂�
static RECT pMonitorRect_[UNIWINC_MAX_MONITORCOUNT];	// EnumDisplayMonitors�̏��Ԃŕێ������A�e��ʂ�RECT
static INT pMonitorIndices_[UNIWINC_MAX_MONITORCOUNT];	// ���̃��C�u�����Ǝ��̃��j�^�ԍ����L�[�Ƃ����AEnumDisplayMonitors�ł̏���
static HMONITOR hMonitors_[UNIWINC_MAX_MONITORCOUNT];	// Monitor handles
static WNDPROC lpMyWndProc_ = NULL;
static WNDPROC lpOriginalWndProc_ = NULL;
//static HHOOK hHook_ = NULL;
static WindowStyleChangedCallback hWindowStyleChangedHandler_ = nullptr;
static MonitorChangedCallback hMonitorChangedHandler_ = nullptr;
static DropFilesCallback hDropFilesHandler_ = nullptr;
static AppCommandCallback hAppCommandHandler_ = nullptr;


// ========================================================================
#pragma region Internal functions

void attachWindow(const HWND hWnd);
void detachWindow();
void refreshWindow();
void updateScreenSize();
//void BeginHook();
//void EndHook();
void CreateCustomWindowProcedure();
void DestroyCustomWindowProcedure();


/// <summary>
/// ���ɃE�B���h�E���I���ς݂Ȃ�A���̏�Ԃɖ߂��đI��������
/// </summary>
void detachWindow()
{
	if (hTargetWnd_) {
		// Restore the original window procedure
		DestroyCustomWindowProcedure();

		//// Unhook if exist
		//EndHook();

		if (IsWindow(hTargetWnd_)) {
			// �������́A�N�����͖����ł�����̂Ƃ��āA�߂��Ƃ��͖�����
			SetTransparent(false);

			//// �ǎ��������݂��Ă���΃E�B���h�E�̐e��߂�
			//if (hDesktopWnd_ != NULL) {
			//	SetParent(hTargetWnd_, hParentWnd_);
			//}

			//// ��ɍőO�ʂ́A�N�����̏�Ԃɍ��킹��悤�߂�	��SetWindowLong�Ŗ{���߂�͂��ŕs�v�H
			//SetTopmost((originalWindowInfo.dwExStyle & WS_EX_TOPMOST) == WS_EX_TOPMOST);

			// �ŏ��̃X�^�C���ɖ߂�
			SetWindowLong(hTargetWnd_, GWL_STYLE, originalWindowInfo.dwStyle);
			SetWindowLong(hTargetWnd_, GWL_EXSTYLE, originalWindowInfo.dwExStyle);

			// �E�B���h�E�ʒu��߂�
			SetWindowPlacement(hTargetWnd_, &originalWindowPlacement);

			// �\�����X�V
			refreshWindow();
		}
	}
	hTargetWnd_ = NULL;
}

/// <summary>
/// �w��n���h���̃E�B���h�E������g���悤�ɂ���
/// </summary>
/// <param name="hWnd"></param>
void attachWindow(const HWND hWnd) {
	// �I���ς݃E�B���h�E���قȂ���̂ł���΁A���ɖ߂�
	if (hTargetWnd_ != hWnd) {
		detachWindow();
	}

	// �Ƃ肠�������̃^�C�~���O�ŉ�ʃT�C�Y���X�V
	//   �{���͉�ʉ𑜓x�ύX���ɍX�V�������B�E�B���h�E�v���V�[�W���łǂ��H
	updateScreenSize();

	// Set the target
	hTargetWnd_ = hWnd;

	if (hWnd) {
		// Save the original state
		GetWindowInfo(hWnd, &originalWindowInfo);
		GetWindowPlacement(hWnd, &originalWindowPlacement);
		//hParentWnd_ = GetParent(hWnd);

		// Apply current settings
		SetTransparent(bIsTransparent_);
		SetBorderless(bIsBorderless_);
		SetTopmost(bIsTopmost_);
		SetBottommost(bIsBottommost_);
		//SetBackground(bIsBackground_);
		SetClickThrough(bIsClickThrough_);
		SetAllowDrop(bAllowDropFile_);

		// Replace the window procedure
		CreateCustomWindowProcedure();
	}
}

/// <summary>
/// �I�[�i�[�E�B���h�E�n���h����T���ۂ̃R�[���o�b�N
/// </summary>
/// <param name="hWnd"></param>
/// <param name="lParam"></param>
/// <returns></returns>
BOOL CALLBACK findOwnerWindowProc(const HWND hWnd, const LPARAM lParam)
{
	DWORD currentPid = (DWORD)lParam;
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);

	// �v���Z�XID����v����Ύ����̃E�B���h�E�Ƃ���
	if (pid == currentPid) {

		// �I�[�i�[�E�B���h�E��T��
		// Unity�G�f�B�^���Ɩ{�̂��I�΂�ēƗ�Game�r���[���I�΂�Ȃ��c
		HWND hOwner = GetWindow(hWnd, GW_OWNER);
		if (hOwner) {
			// ����΃I�[�i�[��I��
			attachWindow(hOwner);
		}
		else {
			// �I�[�i�[��������΂��̃E�B���h�E��I��
			attachWindow(hWnd);
		}
		return FALSE;

		//// �����v���Z�XID�ł��A�\������Ă���E�B���h�E�݂̂�I��
		//LONG style = GetWindowLong(hWnd, GWL_STYLE);
		//if (style & WS_VISIBLE) {
		//	hTargetWnd_ = hWnd;
		//	return FALSE;
		//}
	}

	return TRUE;
}

/// <summary>
/// �f�X�N�g�b�v�̃E�B���h�E�n���h����T���ۂ̃R�[���o�b�N
/// </summary>
/// <param name="hWnd"></param>
/// <param name="lParam"></param>
/// <returns></returns>
BOOL CALLBACK findDesktopWindowProc(const HWND hWnd, const LPARAM lParam)
{
	WCHAR className[UNIWINC_MAX_CLASSNAME];
	int len = GetClassName(hWnd, className, UNIWINC_MAX_CLASSNAME);

	if (len > 0) {
		// �N���X�����擾�ł��AWorkerW �܂��� Progman �Ȃ炻�̎q�� SHELLDLL_DefView ��ΏۂƂ���
		// �Q�l http://www.orangemaker.sakura.ne.jp/labo/memo/sdk-mfc/win7Desktop.html
		if ((lstrcmp(TEXT("WorkerW"), className) == 0) || (lstrcmp(TEXT("Progman"), className) == 0)) {
			if (bExpedtDesktopWnd) {
				hDesktopWnd_ = hWnd;
				return FALSE;
			}

			HWND hChild = FindWindowEx(hWnd, NULL, TEXT("SHELLDLL_DefView"), NULL);
			if (hChild != NULL) {
				//hDesktopWnd_ = hChild;
				//return FALSE;

				bExpedtDesktopWnd = TRUE;
				return TRUE;
			}
		}
	}

	return TRUE;
}

/// <summary>
/// ���j�^���擾���̃R�[���o�b�N
/// EnumDisplayMonitors()�ŌĂ΂��B���̍ۂ͍ŏ���nMonitorCount��0�ɃZ�b�g�������̂Ƃ���B
/// </summary>
/// <param name="hMon"></param>
/// <param name="hDc"></param>
/// <param name="lpRect"></param>
/// <param name="lParam"></param>
/// <returns></returns>
BOOL CALLBACK monitorEnumProc(HMONITOR hMon, HDC hDc, LPRECT lpRect, LPARAM lParam)
{
	// �ő��舵�����j�^���ɒB������T���I��
	if (nMonitorCount_ >= UNIWINC_MAX_MONITORCOUNT) return FALSE;

	// RECT���L��
	pMonitorRect_[nMonitorCount_] = *lpRect;

	// �v���C�}�����j�^�̍������L��
	if (lpRect->left == 0 && lpRect->top == 0) {
		// ���_�Ɉʒu���郂�j�^���v���C�}�����j�^���Ɣ��f
		nPrimaryMonitorHeight_ = lpRect->bottom;
	}

	// �C���f�b�N�X����U�o�ꏇ�ŕۑ�
	pMonitorIndices_[nMonitorCount_] = nMonitorCount_;

	// Store the monitor handle
	hMonitors_[nMonitorCount_] = hMon;

	// ���j�^���J�E���g
	nMonitorCount_++;

	return TRUE;
}

/// <summary>
/// �ڑ����j�^���Ƃ����̃T�C�Y�ꗗ���擾
/// </summary>
/// <returns>�����Ȃ�TRUE</returns>
BOOL updateMonitorRectangles() {
	//  �J�E���g���邽�߈ꎞ�I��0�ɖ߂�
	nMonitorCount_ = 0;

	// ���j�^��񋓂���RECT��ۑ�
	if (!EnumDisplayMonitors(NULL, NULL, monitorEnumProc, NULL)) {
		return FALSE;
	}

	// ���j�^�̈ʒu����Ƀo�u���\�[�g
	for (int i = 0; i < (nMonitorCount_ - 1); i++) {
		for (int j = (nMonitorCount_ - 1); j > i; j--) {
			RECT pr = pMonitorRect_[pMonitorIndices_[j - 1]];
			RECT cr = pMonitorRect_[pMonitorIndices_[j]];

			// ���ɂ��郂�j�^����A���������Ȃ牺�ɂ��郂�j�^����ƂȂ�悤�\�[�g
			if (pr.left >  cr.left || ((pr.left == cr.left) && (pr.bottom < cr.bottom))) {
				int index = pMonitorIndices_[j - 1];
				pMonitorIndices_[j - 1] = pMonitorIndices_[j];
				pMonitorIndices_[j] = index;
			}
		}
	}

	return TRUE;
}

void enableTransparentByDWM()
{
	if (!hTargetWnd_) return;

	// �S�ʂ�Glass�ɂ���
	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(hTargetWnd_, &margins);
}

void disableTransparentByDWM()
{
	if (!hTargetWnd_) return;

	// �g�̂�Glass�ɂ���
	//	�� �{���̃E�B���h�E�����炩�͈͎̔w���Glass�ɂ��Ă����ꍇ�́A�c�O�Ȃ���\�����߂�܂���
	MARGINS margins = { 0, 0, 0, 0 };
	DwmExtendFrameIntoClientArea(hTargetWnd_, &margins);
}

/// <summary>
/// SetLayeredWindowsAttributes �ɂ���Ďw��F�𓧉߂�����
/// </summary>
void enableTransparentBySetLayered()
{
	if (!hTargetWnd_) return;

	LONG exstyle = GetWindowLong(hTargetWnd_, GWL_EXSTYLE);
	exstyle |= WS_EX_LAYERED;
	SetWindowLong(hTargetWnd_, GWL_EXSTYLE, exstyle);
	SetLayeredWindowAttributes(hTargetWnd_, dwKeyColor_, 0xFF, LWA_COLORKEY);
}

/// <summary>
/// SetLayeredWindowsAttributes �ɂ��w��F���߂�����
/// </summary>
void disableTransparentBySetLayered()
{
	COLORREF cref = { 0 };
	SetLayeredWindowAttributes(hTargetWnd_, cref, 0xFF, LWA_ALPHA);

	LONG exstyle = originalWindowInfo.dwExStyle;
	//exstyle &= ~WinApi.WS_EX_LAYERED;
	SetWindowLong(hTargetWnd_, GWL_EXSTYLE, exstyle);
}

/// <summary>
/// �ǎ��̐e�ƂȂ�E�B���h�E�n���h�����擾
/// </summary>
void findDesktopWindow() {
	bExpedtDesktopWnd = FALSE;
	EnumWindows(findDesktopWindowProc, NULL);
}

/// <summary>
/// �g���������ۂɕ`��T�C�Y������Ȃ��Ȃ邱�ƂɑΉ����邽�߁A�E�B���h�E���������T�C�Y���čX�V
/// </summary>
void refreshWindow() {
	if (!hTargetWnd_) return;

	if (IsZoomed(hTargetWnd_)) {
		// �ő剻����Ă����ꍇ�́A�E�B���h�E�T�C�Y�ύX�̑���Ɉ�x�ŏ������čēx�ő剻
		ShowWindow(hTargetWnd_, SW_MINIMIZE);
		ShowWindow(hTargetWnd_, SW_MAXIMIZE);
	}
	else if (IsIconic(hTargetWnd_)) {
		// �ŏ�������Ă����ꍇ�́A���ɕ\�������Ƃ��ɍX�V�������̂Ƃ��āA�������Ȃ�
	}
	else if (IsWindowVisible(hTargetWnd_)) {
		// �ʏ�̃E�B���h�E�������ꍇ�́A�E�B���h�E�T�C�Y��1px�ς��邱�Ƃōĕ`��

		// ���݂̃E�B���h�E�T�C�Y���擾
		RECT rect;
		GetWindowRect(hTargetWnd_, &rect);

		// 1px�������L���āA���T�C�Y�C�x���g�������I�ɋN����
		SetWindowPos(
			hTargetWnd_,
			NULL,
			0, 0, (rect.right - rect.left + 1), (rect.bottom - rect.top + 1),
			SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS
		);

		// ���̃T�C�Y�ɖ߂��B���̎������T�C�Y�C�x���g�͔�������͂�
		SetWindowPos(
			hTargetWnd_,
			NULL,
			0, 0, (rect.right - rect.left), (rect.bottom - rect.top),
			SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS
		);

		ShowWindow(hTargetWnd_, SW_SHOW);
	}
}

BOOL compareRect(const RECT rcA, const RECT rcB) {
	return ((rcA.left == rcB.left) && (rcA.right == rcB.right) && (rcA.top == rcB.top) && (rcA.bottom == rcB.bottom));
}

/// <summary>
/// Update current monitor information
/// </summary>
/// <returns></returns>
void updateScreenSize() {
	//nPrimaryMonitorHeight_ = GetSystemMetrics(SM_CYSCREEN);	// 150% �Ȃǂ̎��͎��𑜓x�ƈ�v���Ȃ�

	// Update the monitor resolution list.
	//   To use the nPrimaryMonitorHeight, do this after its acquisition.
	updateMonitorRectangles();
}

/// <summary>
/// ���݁A���ۂɏ�ɍőO�ʂɂȂ��Ă��邩�𒲂ׂ�
/// </summary>
/// <returns></returns>
BOOL getTopMost() {
	if ((hTargetWnd_ == NULL) || !IsWindow(hTargetWnd_)) {
		return FALSE;
	}
	LONG ex = GetWindowLong(hTargetWnd_, GWL_EXSTYLE);
	return (ex & WS_EX_TOPMOST) == WS_EX_TOPMOST;
}

#pragma endregion Internal functions


// ========================================================================
#pragma region For window style

/// <summary>
/// ���p�\�ȏ�ԂȂ�true��Ԃ�
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API IsActive() {
	if (hTargetWnd_ && IsWindow(hTargetWnd_)) {
		return TRUE;
	}
	return FALSE;
}

/// <summary>
/// ���߂ɂ��Ă��邩�ۂ���Ԃ�
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API IsTransparent() {
	return bIsTransparent_;
}

/// <summary>
/// �g���������Ă��邩�ۂ���Ԃ�
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API IsBorderless() {
	return bIsBorderless_;
}

/// <summary>
/// �őO�ʂɂ��Ă��邩�ۂ���Ԃ�
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API IsTopmost() {
	return bIsTopmost_;
}

/// <summary>
/// �Ŕw�ʂɂ��Ă��邩�ۂ���Ԃ�
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API IsBottommost() {
	return bIsBottommost_;
}

/// <summary>
/// �ǎ��ɂ��Ă��邩�ۂ���Ԃ�
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API IsBackground() {
	return bIsBackground_;
}

/// <summary>
/// Return true if the window is zoomed
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API IsMaximized() {
	return (hTargetWnd_ && IsZoomed(hTargetWnd_));
}

/// <summary>
/// Return true if the window is iconic
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API IsMinimized() {
	return (hTargetWnd_ && IsIconic(hTargetWnd_));
}

/// <summary>
/// Restore and release the target window
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API DetachWindow() {
	detachWindow();
	return true;
}

/// <summary>
/// Find my own window and attach (Same as the AttachMyOwnerWindow)
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API AttachMyWindow() {
	return AttachMyOwnerWindow();
}

/// <summary>
/// Find and select the window with the current process ID
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API AttachMyOwnerWindow() {
	DWORD currentPid = GetCurrentProcessId();
	return EnumWindows(findOwnerWindowProc, (LPARAM)currentPid);
}

/// <summary>
/// Find and select the active window with the current process ID
///   (To attach the process with multiple windows)
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API AttachMyActiveWindow() {
	DWORD currentPid = GetCurrentProcessId();
	HWND hWnd = GetActiveWindow();
	DWORD pid;

	GetWindowThreadProcessId(hWnd, &pid);
	if (pid == currentPid) {
		attachWindow(hWnd);
		return TRUE;
	}
	return FALSE;
}

/// <summary>
/// Select the transparentize method
/// </summary>
/// <param name="type"></param>
/// <returns></returns>
void UNIWINC_API SetTransparentType(const TransparentType type) {
	if (bIsTransparent_) {
		// ��������Ԃł���΁A��x�������Ă���ݒ�
		SetTransparent(FALSE);
		nTransparentType_ = type;
		SetTransparent(TRUE);
	}
	else {
		// ��������ԂłȂ���΁A���̂܂ܐݒ�
		nTransparentType_ = type;
	}
}

/// <summary>
/// �P�F���ߎ��ɓ��߂Ƃ���F���w��
/// </summary>
/// <param name="color">���߂���F</param>
/// <returns></returns>
void UNIWINC_API SetKeyColor(const COLORREF color) {
	if (bIsTransparent_ && (nTransparentType_ == TransparentType::ColorKey)) {
		// ��������Ԃł���΁A��x�������Ă���ݒ�
		SetTransparent(FALSE);
		dwKeyColor_ = color;
		SetTransparent(TRUE);
	}
	else {
		// ��������ԂłȂ���΁A���̂܂ܐݒ�
		dwKeyColor_ = color;
	}
}

/// <summary>
/// ���߂���јg������ݒ�^����
/// </summary>
/// <param name="bTransparent"></param>
/// <returns></returns>
void UNIWINC_API SetTransparent(const BOOL bTransparent) {
	if (hTargetWnd_) {
		if (bTransparent) {
			switch (nTransparentType_)
			{
			case TransparentType::Alpha:
				enableTransparentByDWM();
				break;
			case TransparentType::ColorKey:
				enableTransparentBySetLayered();
				break;
			default:
				break;
			}
		}
		else {
			switch (nCurrentTransparentType_)
			{
			case TransparentType::Alpha:
				disableTransparentByDWM();
				break;
			case TransparentType::ColorKey:
				disableTransparentBySetLayered();
				break;
			default:
				break;
			}
		}

		// �߂����@�����߂邽�߁A���������ύX���ꂽ���̃^�C�v���L��
		nCurrentTransparentType_ = nTransparentType_;
	}

	// ��������Ԃ��L��
	bIsTransparent_ = bTransparent;
}


/// <summary>
/// �E�B���h�E�g��L���^�����ɂ���
/// </summary>
/// <param name="bBorderless"></param>
void UNIWINC_API SetBorderless(const BOOL bBorderless) {
	if (hTargetWnd_) {
		int newW, newH, newX, newY;
		RECT rcWin, rcCli;
		GetWindowRect(hTargetWnd_, &rcWin);
		GetClientRect(hTargetWnd_, &rcCli);

		int w = rcWin.right - rcWin.left;
		int h = rcWin.bottom - rcWin.top;

		int bZoomed = IsZoomed(hTargetWnd_);
		int bIconic = IsIconic(hTargetWnd_);

		// �ő剻����Ă�����A��x�ő剻�͉���
		if (bZoomed) {
			ShowWindow(hTargetWnd_, SW_NORMAL);
		}

		if (bBorderless) {
			// �g�����E�B���h�E�ɂ���
			LONG currentWS = (WS_VISIBLE | WS_POPUP);
			SetWindowLong(hTargetWnd_, GWL_STYLE, currentWS);

			newW = rcCli.right - rcCli.left;
			newH = rcCli.bottom - rcCli.top;

			int bw = (w - newW) / 2;	// �g�̕Б��� [px]
			newX = rcWin.left + bw;
			newY = rcWin.top + ((h - newH) - bw);	// �{���͘g�̉��������ƍ��E�̕��������ۏ؂͂Ȃ����A�Ƃ肠���������Ƃ݂Ȃ��Ă���

		}
		else {
			// �E�B���h�E�X�^�C����߂�
			SetWindowLong(hTargetWnd_, GWL_STYLE, originalWindowInfo.dwStyle);

			int dx = (originalWindowInfo.rcWindow.right - originalWindowInfo.rcWindow.left) - (originalWindowInfo.rcClient.right - originalWindowInfo.rcClient.left);
			int dy = (originalWindowInfo.rcWindow.bottom - originalWindowInfo.rcWindow.top) - (originalWindowInfo.rcClient.bottom - originalWindowInfo.rcClient.top);
			int bw = dx / 2;	// �g�̕Б��� [px]

			newW = rcCli.right - rcCli.left + dx;
			newH = rcCli.bottom- rcCli.top + dy;

			newX = rcWin.left - bw;
			newY = rcWin.top - (dy - bw);	// �{���͘g�̉��������ƍ��E�̕��������ۏ؂͂Ȃ����A�Ƃ肠���������Ƃ݂Ȃ��Ă���
		}

		// �E�B���h�E�T�C�Y���ω����Ȃ����A�ő剻��ŏ�����ԂȂ�W���̃T�C�Y�X�V
		if (bZoomed) {
			// �ő剻����Ă�����A�����ōēx�ő剻
			ShowWindow(hTargetWnd_, SW_MAXIMIZE);
		} else if (bIconic) {
			// �ŏ�������Ă�����A���ɕ\�������Ƃ��̍ĕ`������҂��āA�������Ȃ�
		} else if (newW == w && newH == h) {
			// �E�B���h�E�ĕ`��
			refreshWindow();
		}
		else
		{
			// �N���C�A���g�̈�T�C�Y���ێ�����悤�T�C�Y�ƈʒu�𒲐�
			SetWindowPos(
				hTargetWnd_,
				NULL,
				newX, newY, newW, newH,
				SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS
			);

			ShowWindow(hTargetWnd_, SW_SHOW);
		}
	}

	// �g���������L��
	bIsBorderless_ = bBorderless;
}

/// <summary>
/// �őO�ʉ��^����
/// </summary>
/// <param name="bTopmost"></param>
/// <returns></returns>
void UNIWINC_API SetTopmost(const BOOL bTopmost) {
	// �Ŕw�ʉ�����Ă�����A����
	bIsBottommost_ = false;

	if (hTargetWnd_) {
		SetWindowPos(
			hTargetWnd_,
			(bTopmost ? HWND_TOPMOST : HWND_NOTOPMOST),
			0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS // | SWP_FRAMECHANGED
		);

		// Run callback if the topmost state changed
		if (bIsTopmost_ != bTopmost) {
			if (hWindowStyleChangedHandler_ != nullptr) {
				hWindowStyleChangedHandler_((INT32)EventType::Style);
			}
		}
	}

	bIsTopmost_ = bTopmost;
}

/// <summary>
/// �Ŕw�ʉ��^����
/// </summary>
/// <param name="bBottommost"></param>
/// <returns></returns>
void UNIWINC_API SetBottommost(const BOOL bBottommost) {
	// �őO�ʉ�����Ă�����A����
	bIsTopmost_ = false;

	if (hTargetWnd_) {
		SetWindowPos(
			hTargetWnd_,
			(bBottommost ? HWND_BOTTOM : HWND_NOTOPMOST),
			0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS // | SWP_FRAMECHANGED
		);

		// Run callback if the bottommost state changed
		if (bIsBottommost_ != bBottommost) {
			if (hWindowStyleChangedHandler_ != nullptr) {
				hWindowStyleChangedHandler_((INT32)EventType::Style);
			}
		}
	}

	bIsBottommost_ = bBottommost;
}

/// <summary>
/// �ǎ����^����
/// </summary>
/// <param name="bEnabled"></param>
/// <returns></returns>
void UNIWINC_API SetBackground(const BOOL bEnabled) {
	if (hTargetWnd_) {
		if (bEnabled) {
			// �f�X�N�g�b�v�ɂ�����E�B���h�E�����擾�Ȃ�A�����Ŏ擾
			if (hDesktopWnd_ == NULL) {
				findDesktopWindow();
			}

			if (hDesktopWnd_ != NULL) {
				SetParent(hTargetWnd_, hDesktopWnd_);
				//SetBottommost(true);
				//SetWindowPos(
				//	hTargetWnd_,
				//	HWND_BOTTOM,
				//	0, 0, 0, 0,
				//	SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS // | SWP_FRAMECHANGED
				//);
				//refreshWindow();
			}
		}
		else
		{
			SetParent(hTargetWnd_, hParentWnd_);
			//SetBottommost(false);
		}

		// Run callback if the bottommost state changed
		if (bIsBackground_!= bEnabled) {
			if (hWindowStyleChangedHandler_ != nullptr) {
				hWindowStyleChangedHandler_((INT32)EventType::Style);
			}
		}
	}

	bIsBackground_= bEnabled;
}

/// <summary>
/// Zoom the window or normalize
/// </summary>
/// <param name="bZoomed"></param>
/// <returns></returns>
void UNIWINC_API SetMaximized(const BOOL bZoomed) {
	if (hTargetWnd_) {
		if (bZoomed) {
			ShowWindow(hTargetWnd_, SW_MAXIMIZE);
		}
		else
		{
			ShowWindow(hTargetWnd_, SW_NORMAL);
		}
	}
}

/// <summary>
/// �N���b�N�X���[�i�}�E�X���얳�����j��ݒ�^����
/// </summary>
/// <param name="bTransparent"></param>
/// <returns></returns>
void UNIWINC_API SetClickThrough(const BOOL bTransparent) {
	if (hTargetWnd_) {
		if (bTransparent) {
			LONG exstyle = GetWindowLong(hTargetWnd_, GWL_EXSTYLE);
			exstyle |= WS_EX_TRANSPARENT;
			exstyle |= WS_EX_LAYERED;
			SetWindowLong(hTargetWnd_, GWL_EXSTYLE, exstyle);
		}
		else
		{
			LONG exstyle = GetWindowLong(hTargetWnd_, GWL_EXSTYLE);
			exstyle &= ~WS_EX_TRANSPARENT;
			if (!bIsTransparent_ && !(originalWindowInfo.dwExStyle & WS_EX_LAYERED)) {
				exstyle &= ~WS_EX_LAYERED;
			}
			SetWindowLong(hTargetWnd_, GWL_EXSTYLE, exstyle);
		}
	}
	bIsClickThrough_ = bTransparent;
}

/// <summary>
/// Set the window position
/// </summary>
/// <param name="x">�E�B���h�E���[���W [px]</param>
/// <param name="y">�v���C�}���[��ʉ��[�����_�Ƃ��A�オ����Y���W [px]</param>
/// <returns>��������� true</returns>
BOOL UNIWINC_API SetPosition(const float x, const float y) {
	if (hTargetWnd_ == NULL) return FALSE;

	// ���݂̃E�B���h�E�ʒu�ƃT�C�Y���擾
	RECT rect;
	GetWindowRect(hTargetWnd_, &rect);

	// ������ y ��Cocoa�����̍��W�n�ŃE�B���h�E�����Ȃ̂ŁA�ϊ�
	int newY = (nPrimaryMonitorHeight_ - (int)y) - (rect.bottom - rect.top);
	int newX = (int)(x);

	return SetWindowPos(
		hTargetWnd_, NULL,
		newX, newY,
		0, 0,
		SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_ASYNCWINDOWPOS
		);
}

/// <summary>
/// Get the window position
/// </summary>
/// <param name="x">�E�B���h�E���[���W [px]</param>
/// <param name="y">�v���C�}���[��ʉ��[�����_�Ƃ��A�オ����Y���W [px]</param>
/// <returns>��������� true</returns>
BOOL UNIWINC_API GetPosition(float* x, float* y) {
	*x = 0;
	*y = 0;

	if (hTargetWnd_ == NULL) return FALSE;

	RECT rect;
	if (GetWindowRect(hTargetWnd_, &rect)) {
		*x = (float)(rect.left);
		*y = (float)(nPrimaryMonitorHeight_- rect.bottom);	// ������Ƃ���
		return TRUE;
	}
	return FALSE;
}

/// <summary>
/// Set the window size
/// </summary>
/// <param name="width">�� [px]</param>
/// <param name="height">���� [px]</param>
/// <returns>��������� true</returns>
BOOL UNIWINC_API SetSize(const float width, const float height) {
	if (hTargetWnd_ == NULL) return FALSE;

	// ���݂̃E�B���h�E�ʒu�ƃT�C�Y���擾
	RECT rect;
	GetWindowRect(hTargetWnd_, &rect);

	int x = rect.left;
	int y = rect.bottom;
	int w = (int)(width);
	int h = (int)(height);

	// �������_�Ƃ��邽�߂ɒ��������A�V�KY���W
	y = y - h;

	return SetWindowPos(
		hTargetWnd_, NULL,
		x, y, w, h,
		SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS
	);
}

/// <summary>
/// Get the window size with the border
/// </summary>
/// <param name="width">�� [px]</param>
/// <param name="height">���� [px]</param>
/// <returns>��������� true</returns>
BOOL  UNIWINC_API GetSize(float* width, float* height) {
	*width = 0;
	*height = 0;

	if (hTargetWnd_ == NULL) return FALSE;
	RECT rect;
	if (GetWindowRect(hTargetWnd_, &rect)) {
		*width = (float)(rect.right - rect.left);	// +1 �͕s�v�Ȃ悤
		*height = (float)(rect.bottom - rect.top);	// +1 �͕s�v�Ȃ悤

		return TRUE;
	}
	return FALSE;
}

/// <summary>
/// Register the callback fucnction called when window style changed
/// </summary>
/// <param name="callback"></param>
/// <returns></returns>
BOOL UNIWINC_API RegisterWindowStyleChangedCallback(WindowStyleChangedCallback callback) {
	if (callback == nullptr) return FALSE;

	hWindowStyleChangedHandler_= callback;
	return TRUE;
}

/// <summary>
/// Unregister the callback function
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API UnregisterWindowStyleChangedCallback() {
	hWindowStyleChangedHandler_ = nullptr;
	return TRUE;
}


#pragma endregion For window style


// ========================================================================
#pragma region For monitor Info.

/// <summary>
/// ���̃E�B���h�E�����ݕ\������Ă��郂�j�^�ԍ����擾
/// </summary>
/// <returns></returns>
INT32 UNIWINC_API GetCurrentMonitor() {
	int primaryIndex = 0;

	//  �E�B���h�E���擾�Ȃ�v���C�}�����j�^��T��
	if (hTargetWnd_ == NULL) {
		for (int i = 0; i < nMonitorCount_; i++) {
			RECT mr = pMonitorRect_[pMonitorIndices_[i]];

			// ���_�ɂ��郂�j�^�̓v���C�}���Ɣ���
			if (mr.left == 0 && mr.top == 0) {
				primaryIndex = i;
				break;
			}
		}
		return primaryIndex;
	}

	// ���݂̃E�B���h�E�̒��S���W���擾
	RECT rect;
	GetWindowRect(hTargetWnd_, &rect);
	LONG cx = (rect.right - 1 + rect.left) / 2;
	LONG cy = (rect.bottom - 1 + rect.top) / 2;

	// �E�B���h�E�̒������܂܂�Ă��郂�j�^������
	for (int i = 0; i < nMonitorCount_; i++) {
		RECT mr = pMonitorRect_[pMonitorIndices_[i]];

		// �E�B���h�E���S�������Ă���΂��̉�ʔԍ���Ԃ��ďI��
		if (mr.left <= cx && cx < mr.right && mr.top <= cy && cy < mr.bottom) {
			return i;
		}

		// ���_�ɂ��郂�j�^�̓v���C�}���Ɣ���
		if (mr.left == 0 && mr.top == 0) {
			primaryIndex = i;
		}
	}

	// ����ł��Ȃ���΃v���C�}�����j�^�̉�ʔԍ���Ԃ�
	return primaryIndex;
}


/// <summary>
/// �ڑ�����Ă��郂�j�^�����擾
/// </summary>
/// <returns>���j�^��</returns>
INT32  UNIWINC_API GetMonitorCount() {
	//// SM_CMONITORS �ł͕\������Ă��郂�j�^�̂ݑΏۂƂȂ�iEnumDisplay�Ƃ͈قȂ�j
	//return GetSystemMetrics(SM_CMONITORS);
	return nMonitorCount_;
}

/// <summary>
/// ���j�^�̈ʒu�A�T�C�Y���擾
/// </summary>
/// <param name="width">�� [px]</param>
/// <param name="height">���� [px]</param>
/// <returns>��������� true</returns>
BOOL  UNIWINC_API GetMonitorRectangle(const INT32 monitorIndex, float* x, float* y, float* width, float* height) {
	*x = 0;
	*y = 0;
	*width = 0;
	*height = 0;

	if (monitorIndex < 0 || monitorIndex >= nMonitorCount_) {
		return FALSE;
	}

	RECT rect = pMonitorRect_[pMonitorIndices_[monitorIndex]];
	*x = (float)(rect.left);
	*y = (float)(nPrimaryMonitorHeight_ - rect.bottom);		// ������Ƃ���
	*width = (float)(rect.right - rect.left);
	*height = (float)(rect.bottom - rect.top);
	return TRUE;
}

/// <summary>
/// Register the callback fucnction called when updated monitor information
/// </summary>
/// <param name="callback"></param>
/// <returns></returns>
BOOL UNIWINC_API RegisterMonitorChangedCallback(MonitorChangedCallback callback) {
	if (callback == nullptr) return FALSE;

	hMonitorChangedHandler_ = callback;
	return TRUE;
}

/// <summary>
/// Unregister the callback function
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API UnregisterMonitorChangedCallback() {
	hMonitorChangedHandler_ = nullptr;
	return TRUE;
}

#pragma endregion For monitor Info.


// ========================================================================
#pragma region For mouse cursor

/// <summary>
/// �}�E�X�J�[�\�����W���擾
/// </summary>
/// <param name="x">�E�B���h�E���[���W [px]</param>
/// <param name="y">�v���C�}���[��ʉ��[�����_�Ƃ��A�オ����Y���W [px]</param>
/// <returns>��������� true</returns>
BOOL UNIWINC_API GetCursorPosition(float* x, float* y) {
	*x = 0;
	*y = 0;

	POINT pos;
	if (GetCursorPos(&pos)) {
		*x = (float)pos.x;
		*y = (float)(nPrimaryMonitorHeight_ - pos.y - 1);	// ������Ƃ���
		return TRUE;
	}
	return FALSE;

}

/// <summary>
/// �}�E�X�J�[�\�����W��ݒ�
/// </summary>
/// <param name="x">�E�B���h�E���[���W [px]</param>
/// <param name="y">�v���C�}���[��ʉ��[�����_�Ƃ��A�オ����Y���W [px]</param>
/// <returns>��������� true</returns>
BOOL UNIWINC_API SetCursorPosition(const float x, const float y) {
	POINT pos;

	pos.x = (int)x;
	pos.y = nPrimaryMonitorHeight_ - (int)y - 1;

	return SetCursorPos(pos.x, pos.y);
}

#pragma endregion For mouse cursor


// ========================================================================
#pragma region For file dropping and window procedure

/// <summary>
/// Process drop files
/// </summary>
/// <param name="hDrop"></param>
/// <returns></returns>
BOOL ReceiveDropFiles(HDROP hDrop) {
	UINT num = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

	if (num > 0) {
		// Retrieve total buffer size
		UINT bufferSize = 0;
		for (UINT i = 0; i < num; i++) {
			UINT size = DragQueryFile(hDrop, i, NULL, 0);
			bufferSize += size + sizeof(L'\n');		// Add a delimiter size
		}
		bufferSize++;

		// Allocate buffer
		LPWSTR buffer;
		buffer = new (std::nothrow)WCHAR[bufferSize];

		if (buffer != NULL) {
			// Retrieve file paths
			UINT bufferIndex = 0;
			for (UINT i = 0; i < num; i++) {
				UINT cch = bufferSize - 1 - bufferIndex;
				UINT size = DragQueryFile(hDrop, i, buffer + bufferIndex, cch);
				bufferIndex += size;
				buffer[bufferIndex] = L'\n';	// Delimiter of each path
				bufferIndex++;
			}
			buffer[bufferIndex] = NULL;

			// Do callback function
			if (hDropFilesHandler_ != nullptr) {
				hDropFilesHandler_((WCHAR*)buffer);	// Charset of this project must be set U
			}

			delete[] buffer;
		}
	}

	return (num > 0);
}

/// <summary>
/// Custom window proceture to accept dropped files and display-changed event
/// </summary>
/// <param name="hWnd"></param>
/// <param name="uMsg"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK CustomWindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDROP hDrop;
	INT32 count;

	switch (uMsg)
	{
	case WM_DROPFILES:
		hDrop = (HDROP)wParam;
		ReceiveDropFiles(hDrop);
		DragFinish(hDrop);
		break;

	case WM_DISPLAYCHANGE:
		updateScreenSize();

		// Run callback
		if (hMonitorChangedHandler_ != nullptr) {
			count = GetMonitorCount();
			hMonitorChangedHandler_(count);
		}
		break;

	case WM_WINDOWPOSCHANGING:
		// ��ɍŔw��
		if (bIsBottommost_) {
			((WINDOWPOS*)lParam)->hwndInsertAfter = HWND_BOTTOM;
		}
		break;

	case WM_STYLECHANGED:	// �X�^�C���̕ω������o
		// Run callback
		if (hWindowStyleChangedHandler_ != nullptr) {
			hWindowStyleChangedHandler_((INT32)EventType::Style);
		}
		break;

	case WM_SIZE:		// �ő剻�A�ŏ����ɂ��ω������o
		switch (wParam)
		{
		case SIZE_RESTORED:
		case SIZE_MAXIMIZED:
		case SIZE_MINIMIZED:
			// Run callback
			if (hWindowStyleChangedHandler_ != nullptr) {
				hWindowStyleChangedHandler_((INT32)EventType::Size);
			}
			break;
		}
		break;
		
	// �}���`���f�B�A�L�[���擾�ł��邩�e�X�g
	case WM_APPCOMMAND:
		if (lParam != NULL) {
			if (hAppCommandHandler_ != nullptr) {
				hAppCommandHandler_((INT32)lParam);
			}
		}

	default:
		break;
	}

	if (lpOriginalWndProc_ != NULL) {
		return CallWindowProc(lpOriginalWndProc_, hWnd, uMsg, wParam, lParam);
	}
	else {
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

/// <summary>
/// Wrapper of SetWindowLongPtr to set window procedure
/// </summary>
/// <param name="wndProc"></param>
/// <returns></returns>
WNDPROC SetWindowProcedure(WNDPROC wndProc) {
	//return (WNDPROC)SetWindowLongPtr(hTargetWnd_, GWLP_WNDPROC, (LONG_PTR)wndProc);

#ifdef _WIN64
	// 64bit
	return (WNDPROC)SetWindowLongPtr(hTargetWnd_, GWLP_WNDPROC, (LONG_PTR)wndProc);
#else
	return (WNDPROC)SetWindowLong(hTargetWnd_, GWLP_WNDPROC, (LONG)wndProc);
#endif
}

/// <summary>
/// Remove the custom window procedure
/// </summary>
void DestroyCustomWindowProcedure() {
	if (lpMyWndProc_ == NULL) return;

	if (lpOriginalWndProc_ != NULL) {
		if (hTargetWnd_ != NULL && IsWindow(hTargetWnd_)) {
			SetWindowProcedure(lpOriginalWndProc_);
		}
		lpOriginalWndProc_ = NULL;
	}
	lpMyWndProc_ = NULL;
}

/// <summary>
/// Create and attach the custom window procedure
/// </summary>
void CreateCustomWindowProcedure() {
	if (lpMyWndProc_ != NULL) {
		DestroyCustomWindowProcedure();
	}

	if (hTargetWnd_ != NULL) {
		lpMyWndProc_ = CustomWindowProcedure;
		lpOriginalWndProc_ = SetWindowProcedure(lpMyWndProc_);
	}
}


// ���E�B���h�E�v���V�[�W���ł͂Ȃ����b�Z�[�W���t�b�N����ꍇ�͂�������g��
//    �𑜓x�ύX�����o���邽�߂ɃE�B���h�E�v���V�[�W�����g�����̂Ƃ���

///// <summary>
///// Callback when received WM_DROPFILE message
///// </summary>
///// <param name="nCode"></param>
///// <param name="wParam"></param>
///// <param name="lParam"></param>
///// <returns></returns>
//LRESULT CALLBACK MessageHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//	if (nCode < 0) {
//		return CallNextHookEx(NULL, nCode, wParam, lParam);
//	}
//
//	// lParam is a pointer to an MSG structure for WH_GETMESSAGE
//	LPMSG msg = (LPMSG)lParam;
//
//	switch (msg->message) {
//	case WM_DROPFILES:
//		if (hTargetWnd_ != NULL && msg->hwnd == hTargetWnd_) {
//			HDROP hDrop = (HDROP)msg->wParam;
//			ReceiveDropFiles(hDrop);
//			DragFinish(hDrop);
//		}
//		return TRUE;
//		break;
//
//	case WM_DISPLAYCHANGE:
//		updateScreenSize();
//		break;
//
//	case WM_STYLECHANGED:
//		break;
//	}
//
//	return CallNextHookEx(NULL, nCode, wParam, lParam);
//}
//
///// <summary>
///// Set the hook
///// </summary>
//void BeginHook() {
//	if (hTargetWnd_ == NULL) return;
//
//	// Return if the hook is already set
//	if (hHook_ != NULL) return;
//
//	//HMODULE hMod = GetModuleHandle(NULL);
//	DWORD dwThreadId = GetCurrentThreadId();
//
//	hHook_ = SetWindowsHookEx(WH_GETMESSAGE, MessageHookCallback, NULL, dwThreadId);
//}
//
///// <summary>
///// Unset the hook
///// </summary>
//void EndHook() {
//	if (hTargetWnd_ == NULL) return;
//
//	// Return if the hook is not set
//	if (hHook_ == NULL) return;
//
//	UnhookWindowsHookEx(hHook_);
//	hHook_ = NULL;
//}


/// <summary>
/// Enable or disable file dropping
/// </summary>
/// <returns>Previous window procedure</returns>
BOOL UNIWINC_API SetAllowDrop(const BOOL bEnabled)
{
	if (hTargetWnd_ == NULL) return FALSE;

	bAllowDropFile_ = bEnabled;
	DragAcceptFiles(hTargetWnd_, bAllowDropFile_);

	//if (bEnabled && hHook == NULL) {
	//	BeginHook();
	//}
	////else if (!bEnabled && hHook != NULL) {
	////	EndHook();
	////}

	return TRUE;
}

/// <summary>
/// Register the callback fucnction for dropping files
/// </summary>
/// <param name="callback"></param>
/// <returns></returns>
BOOL UNIWINC_API RegisterDropFilesCallback(DropFilesCallback callback) {
	if (callback == nullptr) return FALSE;

	hDropFilesHandler_ = callback;
	return TRUE;
}

/// <summary>
/// Unregister the callback function
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API UnregisterDropFilesCallback() {
	hDropFilesHandler_ = nullptr;
	return TRUE;
}

#pragma endregion For file dropping and window procedure


// ========================================================================
#pragma region Windows only public functions

/// <summary>
/// ���ݑI������Ă���E�B���h�E�n���h�����擾
/// </summary>
/// <returns></returns>
HWND UNIWINC_API GetWindowHandle() {
	return hTargetWnd_;
}

/// <summary>
/// �ǎ����̐e�ƂȂ�E�B���h�E�n���h�����擾
/// </summary>
/// <returns></returns>
HWND UNIWINC_API GetDesktopWindowHandle() {
	return hDesktopWnd_;
}

/// <summary>
/// �����̃v���Z�XID���擾
/// </summary>
/// <returns></returns>
DWORD UNIWINC_API GetMyProcessId() {
	return GetCurrentProcessId();
}


/// <summary>
/// Register the callback fucnction for App command message
/// </summary>
/// <param name="callback"></param>
/// <returns></returns>
BOOL UNIWINC_API RegisterAppCommandCallback(AppCommandCallback callback) {
	if (callback == nullptr) return FALSE;

	hAppCommandHandler_ = callback;
	return TRUE;
}

/// <summary>
/// Unregister the callback function
/// </summary>
/// <returns></returns>
BOOL UNIWINC_API UnregisterAppCommandCallback() {
	hAppCommandHandler_ = nullptr;
	return TRUE;
}

#pragma endregion Windows-only public functions
