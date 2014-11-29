
#include "menu.h"
#include "loader/wbfs.h"

#include <wiiuse/wpad.h>

#define ARRAY_SIZE(a)	(sizeof a / sizeof a[0])

using namespace std;

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

void CMenu::_hideGameSettings(bool instant)
{
	m_btnMgr.hide(m_gameSettingsLblPage, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPageP, instant);
	m_btnMgr.hide(m_gameSettingsBtnBack, instant);
	m_btnMgr.hide(m_gameSettingsLblTitle, instant);
	m_btnMgr.hide(m_gameSettingsLblGameLanguage, instant);
	m_btnMgr.hide(m_gameSettingsLblLanguage, instant);
	m_btnMgr.hide(m_gameSettingsBtnLanguageP, instant);
	m_btnMgr.hide(m_gameSettingsBtnLanguageM, instant);
	m_btnMgr.hide(m_gameSettingsLblGameVideo, instant);
	m_btnMgr.hide(m_gameSettingsLblVideo, instant);
	m_btnMgr.hide(m_gameSettingsBtnVideoP, instant);
	m_btnMgr.hide(m_gameSettingsBtnVideoM, instant);
	m_btnMgr.hide(m_gameSettingsLblOcarina, instant);
	m_btnMgr.hide(m_gameSettingsBtnOcarina, instant);
	m_btnMgr.hide(m_gameSettingsLblCheat, instant);
	m_btnMgr.hide(m_gameSettingsBtnCheat, instant);
	m_btnMgr.hide(m_gameSettingsLblVipatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnVipatch, instant);
	m_btnMgr.hide(m_gameSettingsLblCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsBtnCountryPatch, instant);
	m_btnMgr.hide(m_gameSettingsLblErr2Fix, instant);
	m_btnMgr.hide(m_gameSettingsBtnErr2Fix, instant);
	m_btnMgr.hide(m_gameSettingsLblCover, instant);
	m_btnMgr.hide(m_gameSettingsBtnCover, instant);
	m_btnMgr.hide(m_gameSettingsLblBlockIOSReload, instant);
	m_btnMgr.hide(m_gameSettingsBtnBlockIOSReload, instant);
	m_btnMgr.hide(m_gameSettingsLblPatchVidModes, instant);
	m_btnMgr.hide(m_gameSettingsLblPatchVidModesVal, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatchVidModesM, instant);
	m_btnMgr.hide(m_gameSettingsBtnPatchVidModesP, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if (m_gameSettingsLblUser[i] != -1u)
			m_btnMgr.hide(m_gameSettingsLblUser[i], instant);
}

std::string CMenu::_optBoolToString(int i)
{
	switch (i)
	{
		case 0:
			return _t("off", "Off");
		case 1:
			return _t("on", "On");
		default:
			return _t("def", "Default");
	}
}

void CMenu::_showGameSettings(void)
{
	int i;
	std::string title(_t("cfgg1", "Settings"));
	//title += " [";
	//title += std::string(m_cf.getId());
	//title += "]";

	m_btnMgr.setText(m_gameSettingsLblTitle, title);

	_setBg(m_gameSettingsBg, m_gameSettingsBg);
	m_btnMgr.show(m_gameSettingsLblPage);
	m_btnMgr.show(m_gameSettingsBtnPageM);
	m_btnMgr.show(m_gameSettingsBtnPageP);
	m_btnMgr.show(m_gameSettingsBtnBack);
	m_btnMgr.show(m_gameSettingsLblTitle);
	
	if (m_gameSettingsPage == 1)
	{
		m_btnMgr.show(m_gameSettingsLblGameLanguage);
		m_btnMgr.show(m_gameSettingsLblLanguage);
		m_btnMgr.show(m_gameSettingsBtnLanguageP);
		m_btnMgr.show(m_gameSettingsBtnLanguageM);
		m_btnMgr.show(m_gameSettingsLblOcarina);
		m_btnMgr.show(m_gameSettingsBtnOcarina);
		m_btnMgr.show(m_gameSettingsLblCheat);
		m_btnMgr.show(m_gameSettingsBtnCheat);
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblGameLanguage);
		m_btnMgr.hide(m_gameSettingsLblLanguage);
		m_btnMgr.hide(m_gameSettingsBtnLanguageP);
		m_btnMgr.hide(m_gameSettingsBtnLanguageM);
		m_btnMgr.hide(m_gameSettingsLblOcarina);
		m_btnMgr.hide(m_gameSettingsBtnOcarina);
		m_btnMgr.hide(m_gameSettingsLblCheat);
		m_btnMgr.hide(m_gameSettingsBtnCheat);
	}
	if (m_gameSettingsPage == 2)
	{
		m_btnMgr.show(m_gameSettingsLblVipatch);
		m_btnMgr.show(m_gameSettingsBtnVipatch);
		m_btnMgr.show(m_gameSettingsLblCountryPatch);
		m_btnMgr.show(m_gameSettingsBtnCountryPatch);
		m_btnMgr.show(m_gameSettingsLblErr2Fix);
		m_btnMgr.show(m_gameSettingsBtnErr2Fix);
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblVipatch);
		m_btnMgr.hide(m_gameSettingsBtnVipatch);
		m_btnMgr.hide(m_gameSettingsLblCountryPatch);
		m_btnMgr.hide(m_gameSettingsBtnCountryPatch);
		m_btnMgr.hide(m_gameSettingsLblErr2Fix);
		m_btnMgr.hide(m_gameSettingsBtnErr2Fix);
	}
	if (m_gameSettingsPage == 3)
	{
		m_btnMgr.show(m_gameSettingsLblCover);
		m_btnMgr.show(m_gameSettingsBtnCover);
		m_btnMgr.show(m_gameSettingsLblBlockIOSReload);
		m_btnMgr.show(m_gameSettingsBtnBlockIOSReload);
		m_btnMgr.show(m_gameSettingsLblPatchVidModes);
		m_btnMgr.show(m_gameSettingsLblPatchVidModesVal);
		m_btnMgr.show(m_gameSettingsBtnPatchVidModesM);
		m_btnMgr.show(m_gameSettingsBtnPatchVidModesP);
		m_btnMgr.show(m_gameSettingsLblGameVideo);
		m_btnMgr.show(m_gameSettingsLblVideo);
		m_btnMgr.show(m_gameSettingsBtnVideoP);
		m_btnMgr.show(m_gameSettingsBtnVideoM);
	}
	else
	{
		m_btnMgr.hide(m_gameSettingsLblCover);
		m_btnMgr.hide(m_gameSettingsBtnCover);
		m_btnMgr.hide(m_gameSettingsLblBlockIOSReload);
		m_btnMgr.hide(m_gameSettingsBtnBlockIOSReload);
		m_btnMgr.hide(m_gameSettingsLblPatchVidModes);
		m_btnMgr.hide(m_gameSettingsLblPatchVidModesVal);
		m_btnMgr.hide(m_gameSettingsBtnPatchVidModesM);
		m_btnMgr.hide(m_gameSettingsBtnPatchVidModesP);
		m_btnMgr.hide(m_gameSettingsLblGameVideo);
		m_btnMgr.hide(m_gameSettingsLblVideo);
		m_btnMgr.hide(m_gameSettingsBtnVideoP);
		m_btnMgr.hide(m_gameSettingsBtnVideoM);
	}
	
	for (u32 i = 0; i < ARRAY_SIZE(m_gameSettingsLblUser); ++i)
		if (m_gameSettingsLblUser[i] != -1u)
			m_btnMgr.show(m_gameSettingsLblUser[i]);

	std::string id(m_cf.getId());
	m_btnMgr.setText(m_gameSettingsLblPage, wfmt("%i / %i", m_gameSettingsPage, 3));
	m_btnMgr.setText(m_gameSettingsBtnOcarina, _optBoolToString(m_cfg.getInt(id, "cheat")));
	m_btnMgr.setText(m_gameSettingsBtnVipatch, _optBoolToString(m_cfg.getInt(id, "vipatch", 0)));
	m_btnMgr.setText(m_gameSettingsBtnCountryPatch, _optBoolToString(m_cfg.getInt(id, "country_patch", 0)));
	m_btnMgr.setText(m_gameSettingsBtnErr2Fix, _optBoolToString(m_cfg.getInt(id, "error_002_fix")));
	m_btnMgr.setText(m_gameSettingsBtnBlockIOSReload, m_cfg.getInt(id, "block_ios_reload", false) ? gOn : gOff);
	i = min((u32)m_cfg.getInt(id, "video_mode", 0), ARRAY_SIZE(CMenu::_videoModes) - 1u);
	m_btnMgr.setText(m_gameSettingsLblVideo, _t(CMenu::_videoModes[i].id, CMenu::_videoModes[i].text));
	i = min((u32)m_cfg.getInt(id, "language", 0), ARRAY_SIZE(CMenu::_languages) - 1u);
	m_btnMgr.setText(m_gameSettingsLblLanguage, _t(CMenu::_languages[i].id, CMenu::_languages[i].text));
	i = min((u32)m_cfg.getInt(id, "patch_video_modes", 0), ARRAY_SIZE(CMenu::_vidModePatch) - 1u);
	m_btnMgr.setText(m_gameSettingsLblPatchVidModesVal, _t(CMenu::_vidModePatch[i].id, CMenu::_vidModePatch[i].text));
}

void CMenu::_gameSettings(void)
{
	s32 padsState;
	WPADData *wd;
	u32 btn;
	string id(m_cf.getId());

	m_gameSettingsPage = 1;
	_showGameSettings();
	while (true)
	{
		WPAD_ScanPads();
		padsState = WPAD_ButtonsDown(0);
		wd = WPAD_Data(0);
		btn = _btnRepeat(wd->btns_h);
		if ((padsState & (WPAD_BUTTON_HOME | WPAD_BUTTON_B)) != 0)
			break;
		if (wd->ir.valid)
			m_btnMgr.mouse(wd->ir.x - m_cur.width() / 2, wd->ir.y - m_cur.height() / 2);
		else if ((padsState & WPAD_BUTTON_UP) != 0)
			m_btnMgr.up();
		else if ((padsState & WPAD_BUTTON_DOWN) != 0)
			m_btnMgr.down();
		if ((padsState & WPAD_BUTTON_MINUS) != 0)
		{
			if (m_gameSettingsPage > 1)
				--m_gameSettingsPage;
			_showGameSettings();
			m_btnMgr.click(m_gameSettingsBtnPageM);
		}
		else if ((padsState & WPAD_BUTTON_PLUS) != 0)
		{
			if (m_gameSettingsPage < 3)
				++m_gameSettingsPage;
			_showGameSettings();
			m_btnMgr.click(m_gameSettingsBtnPageP);
		}
		else if ((padsState & WPAD_BUTTON_A) != 0)
		{
			m_btnMgr.click();
			if (m_btnMgr.selected() == m_gameSettingsBtnBack)
				break;
			else if (m_btnMgr.selected() == m_gameSettingsBtnPageM)
			{
				if (m_gameSettingsPage > 1)
					--m_gameSettingsPage;
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnPageP && !m_locked)
			{
				if (m_gameSettingsPage < 3)
					++m_gameSettingsPage;
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnOcarina)
			{
				m_cfg.setInt(id, "cheat", loopNum(m_cfg.getInt(id, "cheat") + 1, 3));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnVipatch)
			{
//				m_cfg.setOptBool(id, "vipatch", loopNum(m_cfg.getInt(id, "vipatch") + 1, 3));
				m_cfg.setInt(id, "vipatch", !m_cfg.getInt(id, "vipatch"));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnCountryPatch)
			{
//				m_cfg.setOptBool(id, "country_patch", loopNum(m_cfg.getInt(id, "country_patch") + 1, 3));
				m_cfg.setInt(id, "country_patch", !m_cfg.getInt(id, "country_patch"));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnErr2Fix)
			{
				m_cfg.setInt(id, "error_002_fix", loopNum(m_cfg.getInt(id, "error_002_fix") + 1, 3));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnBlockIOSReload)
			{
				m_cfg.setInt(id, "block_ios_reload", !m_cfg.getInt(id, "block_ios_reload", false));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnLanguageP)
			{
				m_cfg.setInt(id, "language", (int)loopNum((u32)m_cfg.getInt(id, "language", 0) + 1, ARRAY_SIZE(CMenu::_languages)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnLanguageM)
			{
				m_cfg.setInt(id, "language", (int)loopNum((u32)m_cfg.getInt(id, "language", 0) - 1, ARRAY_SIZE(CMenu::_languages)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnVideoP)
			{
				m_cfg.setInt(id, "video_mode", (int)loopNum((u32)m_cfg.getInt(id, "video_mode", 0) + 1, ARRAY_SIZE(CMenu::_videoModes)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnVideoM)
			{
				m_cfg.setInt(id, "video_mode", (int)loopNum((u32)m_cfg.getInt(id, "video_mode", 0) - 1, ARRAY_SIZE(CMenu::_videoModes)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnPatchVidModesP)
			{
				m_cfg.setInt(id, "patch_video_modes", (int)loopNum((u32)m_cfg.getInt(id, "patch_video_modes", 0) + 1, ARRAY_SIZE(CMenu::_vidModePatch)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnPatchVidModesM)
			{
				m_cfg.setInt(id, "patch_video_modes", (int)loopNum((u32)m_cfg.getInt(id, "patch_video_modes", 0) - 1, ARRAY_SIZE(CMenu::_vidModePatch)));
				_showGameSettings();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnCover)
			{
				m_cf.stopPicLoader(true);	// Empty cover cache
				_hideGameSettings();
				_download(id);
				_showGameSettings();
				m_cf.startPicLoader();
			}
			else if (m_btnMgr.selected() == m_gameSettingsBtnCheat)
			{
				//Handler for Cheat Button
				_hideGameSettings();
				_CheatSettings();
				_showGameSettings();
			}
		}
		_mainLoopCommon(wd, false);
	}
	_hideGameSettings();
}

void CMenu::_initGameSettingsMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_gameSettingsLblUser, ARRAY_SIZE(m_gameSettingsLblUser), "GAME_SETTINGS");
	m_gameSettingsBg = _texture(theme.texSet, "GAME_SETTINGS/BG", "texture", theme.bg);
	m_gameSettingsLblTitle = _addLabel(theme, "GAME_SETTINGS/TITLE", theme.titleFont, "", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	// Page 1
	m_gameSettingsLblGameLanguage = _addLabel(theme, "GAME_SETTINGS/GAME_LANG", theme.lblFont, "", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblLanguage = _addLabel(theme, "GAME_SETTINGS/GAME_LANG_BTN", theme.btnFont, "", 386, 130, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnLanguageM = _addPicButton(theme, "GAME_SETTINGS/GAME_LANG_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 130, 56, 56);
	m_gameSettingsBtnLanguageP = _addPicButton(theme, "GAME_SETTINGS/GAME_LANG_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_gameSettingsLblOcarina = _addLabel(theme, "GAME_SETTINGS/OCARINA", theme.lblFont, "", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnOcarina = _addButton(theme, "GAME_SETTINGS/OCARINA_BTN", theme.btnFont, "", 330, 190, 270, 56, theme.btnFontColor);
	m_gameSettingsLblCheat = _addLabel(theme, "GAME_SETTINGS/CHEAT", theme.lblFont, "", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCheat = _addButton(theme, "GAME_SETTINGS/CHEAT_BTN", theme.btnFont, "", 330, 250, 270, 56, theme.btnFontColor);
	// Page 2
	m_gameSettingsLblCountryPatch = _addLabel(theme, "GAME_SETTINGS/COUNTRY_PATCH", theme.lblFont, "", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCountryPatch = _addButton(theme, "GAME_SETTINGS/COUNTRY_PATCH_BTN", theme.btnFont, "", 380, 130, 220, 56, theme.btnFontColor);
	m_gameSettingsLblErr2Fix = _addLabel(theme, "GAME_SETTINGS/ERR2FIX", theme.lblFont, "", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnErr2Fix = _addButton(theme, "GAME_SETTINGS/ERR2FIX_BTN", theme.btnFont, "", 380, 190, 220, 56, theme.btnFontColor);
	m_gameSettingsLblVipatch = _addLabel(theme, "GAME_SETTINGS/VIPATCH", theme.lblFont, "", 40, 250, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnVipatch = _addButton(theme, "GAME_SETTINGS/VIPATCH_BTN", theme.btnFont, "", 380, 250, 220, 56, theme.btnFontColor);
	// Page 3
	m_gameSettingsLblCover = _addLabel(theme, "GAME_SETTINGS/COVER", theme.lblFont, "", 40, 130, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnCover = _addButton(theme, "GAME_SETTINGS/COVER_BTN", theme.btnFont, "", 330, 130, 270, 56, theme.btnFontColor);
	m_gameSettingsLblBlockIOSReload = _addLabel(theme, "GAME_SETTINGS/BLOCK_IOSRELOAD", theme.lblFont, "", 40, 190, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsBtnBlockIOSReload = _addButton(theme, "GAME_SETTINGS/BLOCK_IOSRELOAD_BTN", theme.btnFont, "", 330, 190, 270, 56, theme.btnFontColor);
	m_gameSettingsLblPatchVidModes = _addLabel(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE", theme.lblFont, "", 40, 250, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblPatchVidModesVal = _addLabel(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_BTN", theme.btnFont, "", 386, 250, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPatchVidModesM = _addPicButton(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 250, 56, 56);
	m_gameSettingsBtnPatchVidModesP = _addPicButton(theme, "GAME_SETTINGS/PATCH_VIDEO_MODE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 250, 56, 56);
	m_gameSettingsLblGameVideo = _addLabel(theme, "GAME_SETTINGS/VIDEO", theme.lblFont, "", 40, 310, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_gameSettingsLblVideo = _addLabel(theme, "GAME_SETTINGS/VIDEO_BTN", theme.btnFont, "", 386, 310, 158, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnVideoM = _addPicButton(theme, "GAME_SETTINGS/VIDEO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 330, 310, 56, 56);
	m_gameSettingsBtnVideoP = _addPicButton(theme, "GAME_SETTINGS/VIDEO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 310, 56, 56);
	// 
	m_gameSettingsLblPage = _addLabel(theme, "GAME_SETTINGS/PAGE_BTN", theme.btnFont, "", 76, 410, 80, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_gameSettingsBtnPageM = _addPicButton(theme, "GAME_SETTINGS/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 410, 56, 56);
	m_gameSettingsBtnPageP = _addPicButton(theme, "GAME_SETTINGS/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 156, 410, 56, 56);
	m_gameSettingsBtnBack = _addButton(theme, "GAME_SETTINGS/BACK_BTN", theme.btnFont, "", 420, 410, 200, 56, theme.btnFontColor);
	// 
	_setHideAnim(m_gameSettingsLblTitle, "GAME_SETTINGS/TITLE", 0, -200, 0.f, 1.f);
	_setHideAnim(m_gameSettingsLblGameVideo, "GAME_SETTINGS/VIDEO", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblVideo, "GAME_SETTINGS/VIDEO_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVideoM, "GAME_SETTINGS/VIDEO_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVideoP, "GAME_SETTINGS/VIDEO_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblGameLanguage, "GAME_SETTINGS/GAME_LANG", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblLanguage, "GAME_SETTINGS/GAME_LANG_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLanguageM, "GAME_SETTINGS/GAME_LANG_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnLanguageP, "GAME_SETTINGS/GAME_LANG_PLUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblOcarina, "GAME_SETTINGS/OCARINA", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnOcarina, "GAME_SETTINGS/OCARINA_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCheat, "GAME_SETTINGS/CHEAT", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCheat, "GAME_SETTINGS/CHEAT_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCountryPatch, "GAME_SETTINGS/COUNTRY_PATCH", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCountryPatch, "GAME_SETTINGS/COUNTRY_PATCH_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblErr2Fix, "GAME_SETTINGS/ERR2FIX", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnErr2Fix, "GAME_SETTINGS/ERR2FIX_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblVipatch, "GAME_SETTINGS/VIPATCH", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnVipatch, "GAME_SETTINGS/VIPATCH_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblCover, "GAME_SETTINGS/COVER", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnCover, "GAME_SETTINGS/COVER_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblBlockIOSReload, "GAME_SETTINGS/BLOCK_IOSRELOAD", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnBlockIOSReload, "GAME_SETTINGS/BLOCK_IOSRELOAD_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPage, "GAME_SETTINGS/PAGE_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPageM, "GAME_SETTINGS/PAGE_MINUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPageP, "GAME_SETTINGS/PAGE_PLUS", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnBack, "GAME_SETTINGS/BACK_BTN", 0, 200, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPatchVidModes, "GAME_SETTINGS/PATCH_VIDEO_MODE", -200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsLblPatchVidModesVal, "GAME_SETTINGS/PATCH_VIDEO_MODE_BTN", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatchVidModesM, "GAME_SETTINGS/PATCH_VIDEO_MODE_MINUS", 200, 0, 1.f, 0.f);
	_setHideAnim(m_gameSettingsBtnPatchVidModesP, "GAME_SETTINGS/PATCH_VIDEO_MODE_PLUS", 200, 0, 1.f, 0.f);
	_hideGameSettings(true);
	_textGameSettings();
}

void CMenu::_textGameSettings(void)
{
	/*
	m_btnMgr.setText(m_gameSettingsLblTitle, _t("cfgg1", "Settings"));
	m_btnMgr.setText(m_gameSettingsLblGameVideo, _t("cfgg2", "Video mode"));
	m_btnMgr.setText(m_gameSettingsLblGameLanguage, _t("cfgg3", "Language"));
	m_btnMgr.setText(m_gameSettingsLblCountryPatch, _t("cfgg4", "Patch country strings"));
	m_btnMgr.setText(m_gameSettingsLblOcarina, _t("cfgg5", "Ocarina"));
	m_btnMgr.setText(m_gameSettingsLblCheat, _t("cfgg15", "Cheat Codes"));
	m_btnMgr.setText(m_gameSettingsBtnCheat, _t("cfgg16", "Select"));
	m_btnMgr.setText(m_gameSettingsLblErr2Fix, _t("cfgg6", "Error 002 fix"));
	m_btnMgr.setText(m_gameSettingsLblVipatch, _t("cfgg7", "Vipatch"));
	m_btnMgr.setText(m_gameSettingsBtnBack, _t("cfgg8", "Back"));
	m_btnMgr.setText(m_gameSettingsLblBlockIOSReload, _t("cfgg11", "Block IOS reload"));
	m_btnMgr.setText(m_gameSettingsLblCover, _t("cfgg12", "Download cover"));
	m_btnMgr.setText(m_gameSettingsBtnCover, _t("cfgg13", "Download"));
	m_btnMgr.setText(m_gameSettingsLblPatchVidModes, _t("cfgg14", "Patch video modes"));
	*/

	m_btnMgr.setText(m_gameSettingsLblTitle, "Settings");
	m_btnMgr.setText(m_gameSettingsLblGameVideo, "Video mode");
	m_btnMgr.setText(m_gameSettingsLblGameLanguage, "Language");
	m_btnMgr.setText(m_gameSettingsLblCountryPatch, "Patch country strings");
	m_btnMgr.setText(m_gameSettingsLblOcarina,"Ocarina");
	m_btnMgr.setText(m_gameSettingsLblCheat, "Cheat Codes");
	m_btnMgr.setText(m_gameSettingsBtnCheat, "Select");
	m_btnMgr.setText(m_gameSettingsLblErr2Fix, "Error 002 fix");
	m_btnMgr.setText(m_gameSettingsLblVipatch, "Vipatch");
	m_btnMgr.setText(m_gameSettingsBtnBack, "Back");
	m_btnMgr.setText(m_gameSettingsLblBlockIOSReload, "Block IOS reload");
	m_btnMgr.setText(m_gameSettingsLblCover, "Download cover");
	m_btnMgr.setText(m_gameSettingsBtnCover, "Download");
	m_btnMgr.setText(m_gameSettingsLblPatchVidModes, "Patch video modes");
}
