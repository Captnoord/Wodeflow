// Coverflow

#ifndef __COVERFLOW_HPP
#define __COVERFLOW_HPP

#include <ogcsys.h>
#include <gccore.h>
#include <string>
#include <vector>

#include "video.h"
#include "smartptr.h"
#include "FreeTypeGX.h"
#include "text.h"
#include "config.h"
#include "sound.h"
#include "wode.h"

class CCoverFlow
{
public:
	CCoverFlow(void);
	~CCoverFlow(void);
	// 
	bool init(void);
	// Cover list management
	void clear(void);
	void reserve(u32 capacity);
	void addItem(const char *id, unsigned long idx, unsigned long part, const char *title, const char *picPath, const char *boxPicPath, int playcount, int type);
	bool empty(void) const { return m_items.empty(); }
	// 
	bool start(const char *id = 0);
	void stopPicLoader(bool empty = false);
	void startPicLoader(void);
	// 
	void simulateOtherScreenFormat(bool s);
	// Commands
	void tick(void);
	bool findId(const char *id, bool instant = false);
	void pageUp(void);
	void pageDown(void);
	char nextLetter(void);
	char prevLetter(void);
	void left(void);
	void right(void);
	void up(void);
	void down(void);
	bool select(void);
	void flip(bool force = false, bool f = true);
	void cancel(void);
	bool selected(void) const { return m_selected; }
	void makeEffectTexture(CVideo &vid, const STexture &bg);
	void drawText(bool withRectangle = false);
	void draw(void);
	void drawEffect(void);
	void mouse(CVideo &vid, int x, int y);
	bool mouseOver(CVideo &vid, int x, int y);
	// Accessors for settings
	void setCompression(bool enable) { m_compressTextures = enable; }
	bool getBoxMode(void) const { return m_box;}
	void setBufferSize(u32 numCovers);
	void setTextures(const std::string &loadingPic, const std::string &loadingPicFlat, const std::string &noCoverPic, const std::string &noCoverPicFlat);
	void setFont(SFont font, const CColor &color);
	void setRange(u32 rows, u32 columns);
	void setBoxMode(bool box);
	void setTextureQuality(float lodBias, int aniso, bool edgeLOD);
	void setCameraPos(bool selected, const vec3 &pos, const vec3 &aim);
	void setCameraOsc(bool selected, const vec3 &speed, const vec3 &amp);
	void setCoverScale(bool selected, const vec3 &left, const vec3 &right, const vec3 &center, const vec3 &rowCenter);
	void setCoverPos(bool selected, const vec3 &left, const vec3 &right, const vec3 &center, const vec3 &rowCenter);
	void setCoverAngleOsc(bool selected, const vec3 &speed, const vec3 &amp);
	void setCoverPosOsc(bool selected, const vec3 &speed, const vec3 &amp);
	void setSpacers(bool selected, const vec3 &left, const vec3 &right);
	void setDeltaAngles(bool selected, const vec3 &left, const vec3 &right);
	void setAngles(bool selected, const vec3 &left, const vec3 &right, const vec3 &center, const vec3 &rowCenter);
	void setTitleAngles(bool selected, float left, float right, float center);
	void setTitlePos(bool selected, const vec3 &left, const vec3 &right, const vec3 &center);
	void setTitleWidth(bool selected, float side, float center);
	void setTitleStyle(bool selected, u16 side, u16 center);
	void setColors(bool selected, const CColor &begColor, const CColor &endColor, const CColor &offColor);
	void setShadowColors(bool selected, const CColor &centerColor, const CColor &begColor, const CColor &endColor, const CColor &offColor);
	void setShadowPos(float scale, float x, float y);
	void setMirrorAlpha(float cover, float title);
	void setMirrorBlur(bool blur);
	void setRowSpacers(bool selected, const vec3 &top, const vec3 &bottom);
	void setRowDeltaAngles(bool selected, const vec3 &top, const vec3 &bottom);
	void setRowAngles(bool selected, const vec3 &top, const vec3 &bottom);
	void setCoverFlipping(const vec3 &pos, const vec3 &angle, const vec3 &scale);
	void setBlur(u32 blurResolution, u32 blurRadius, float blurFactor);
	// 
	void setSounds(const SSoundEffect &sound, const SSoundEffect &hoverSound, const SSoundEffect &selectSound, const SSoundEffect &cancelSound);
	void setSoundVolume(u8 vol);
	void stopSound(void);
	// 
	void applySettings(void);
	void setCachePath(const char *path, bool deleteSource, bool compress);
	bool fullCoverCached(const char *id);
	bool preCacheCover(const char *id, const u8 *png, bool full);
	// 
	std::string getId(void) const;
	unsigned long getIdx(void) const;
	unsigned long getPart(void) const;
	int getType(void) const;
	std::string getNextId(void) const;
	unsigned long getNextIdx(void) const;
	int getNextType(void) const;
	std::string getTitle(void) const;
private:
	enum DrawMode { CFDR_NORMAL, CFDR_STENCIL, CFDR_SHADOW };
	struct SLayout
	{
		vec3 camera;
		vec3 cameraAim;
		vec3 leftScale;
		vec3 rightScale;
		vec3 centerScale;
		vec3 rowCenterScale;
		vec3 leftPos;
		vec3 rightPos;
		vec3 centerPos;
		vec3 rowCenterPos;
		vec3 leftAngle;
		vec3 rightAngle;
		vec3 centerAngle;
		vec3 rowCenterAngle;
		vec3 leftSpacer;
		vec3 rightSpacer;
		vec3 leftDeltaAngle;
		vec3 rightDeltaAngle;
		float txtLeftAngle;
		float txtRightAngle;
		float txtCenterAngle;
		vec3 txtLeftPos;
		vec3 txtRightPos;
		vec3 txtCenterPos;
		float txtSideWidth;
		float txtCenterWidth;
		u16 txtSideStyle;
		u16 txtCenterStyle;
		vec3 cameraOscSpeed;
		vec3 cameraOscAmp;
		vec3 coverOscASpeed;
		vec3 coverOscAAmp;
		vec3 coverOscPSpeed;
		vec3 coverOscPAmp;
		CColor begColor;
		CColor endColor;
		CColor mouseOffColor;
		CColor shadowColorCenter;
		CColor shadowColorEnd;
		CColor shadowColorBeg;
		CColor shadowColorOff;
		vec3 topSpacer;
		vec3 bottomSpacer;
		vec3 topAngle;
		vec3 bottomAngle;
		vec3 topDeltaAngle;
		vec3 bottomDeltaAngle;
	};
	enum TexState { STATE_Loading, STATE_Ready, STATE_NoCover };
	struct CItem
	{
		std::string			id_;
		unsigned long		game_idx_;
		unsigned long		game_part_;
		std::string			title_;
		std::string			title_compare_cache_; // only cache


		std::string			picPath_;
		std::string			boxPicPath_;
		std::string			discPicPath_;
		int					playcount_;
		int					type_;
		STexture			texture_;
		volatile bool		boxTexture_;
		volatile enum TexState state_;
		
		//
		CItem(const char *itemId, unsigned long idx, unsigned long part, const char *itemTitle, const char *itemPic, const char *itemBoxPic, int playcount, int type);
		
		bool operator<(const CItem &i) const;
	};
	
	struct CCover
	{
		u32		index_;
		vec3	scale_;
		vec3	targetScale_;
		vec3	angle;
		vec3	targetAngle;
		vec3	pos;
		vec3	targetPos;
		CColor	color_;
		CColor	targetColor_;
		float	txtAngle_;
		float	txtTargetAngle_;
		vec3	txtPos;
		vec3	txtTargetPos;
		u8		txtColor_;
		u8		txtTargetColor_;
		CText	title;
		CColor	shadowColor_;
		CColor	targetShadowColor_;
		// 
		CCover(void);
	};
	enum CLRet { CL_OK, CL_ERROR, CL_NOMEM };
private:
	Mtx m_projMtx;
	Mtx m_viewMtx;
	vec3 m_cameraPos;
	vec3 m_cameraAim;
	vec3 m_targetCameraPos;
	vec3 m_targetCameraAim;
	std::vector<CItem> m_items;
	std::vector<CCover> m_covers;
	int m_delay;
	int m_minDelay;
	int m_jump;
	mutex_t m_mutex;
	volatile bool m_loadingPic;
	volatile bool m_waitingToClear;
	volatile bool m_moved;
	volatile int m_hqCover;
	bool m_selected;
	int m_tickCount;
	STexture m_loadingTexture;
	STexture m_noCoverTexture;
	STexture m_dvdSkin;
	STexture m_dvdSkin_Red;
	STexture m_dvdSkin_Black;
	// Settings
	std::string m_pngLoadCover;
	std::string m_pngLoadCoverFlat;
	std::string m_pngNoCover;
	std::string m_pngNoCoverFlat;
	u32 m_numBufCovers;
	SFont m_font;
	CColor m_fontColor;
	bool m_box;
	u32 m_range;
	u32 m_rows;
	u32 m_columns;
	SLayout m_loNormal;
	SLayout m_loSelected;
	int m_mouse;
	bool m_compressTextures;
	bool m_compressCache;
	std::string m_cachePath;
	bool m_deletePicsAfterCaching;
	bool m_mirrorBlur;
	float m_mirrorAlpha;
	float m_txtMirrorAlpha;
	float m_shadowScale;
	float m_shadowX;
	float m_shadowY;
	STexture m_effectTex;
	u32 m_blurRadius;
	float m_blurFactor;
	vec3 m_flipCoverPos;
	vec3 m_flipCoverAngle;
	vec3 m_flipCoverScale;
	bool m_snd2;
	SSoundEffect m_sound1;
	SSoundEffect m_sound2;
	SSoundEffect m_hoverSound;
	SSoundEffect m_selectSound;
	SSoundEffect m_cancelSound;
	u8 m_soundVolume;
	float m_lodBias;
	u8 m_aniso;
	bool m_edgeLOD;
private:
	void _draw(DrawMode dm = CFDR_NORMAL, bool mirror = false, bool blend = true);
	u32 _currentPos(void) const;
	void _effectBg(const STexture &tex);
	void _effectBlur(CVideo &vid, bool vertical);
	bool _effectVisible(void);
	void _drawMirrorZ(void);
	void _drawTitle(int i, bool mirror, bool rectangle);
	void _drawCover(int i, bool mirror, CCoverFlow::DrawMode dm);
	void _drawCoverFlat(int i, bool mirror, CCoverFlow::DrawMode dm);
	void _drawCoverBox(int i, bool mirror, CCoverFlow::DrawMode dm);
	void _updateTarget(int i, bool instant = false);
	void _updateAllTargets(bool instant = false);
	void _loadCover(int i, int item);
	void _loadCoverTexture(int i);
	void _coverTick(int i);
	void _unselect(void);
	vec3 _cameraMoves(void);
	vec3 _coverMovesA(void);
	vec3 _coverMovesP(void);
	STexture &_coverTexture(int i);
	void _left(int repeatDelay, u32 step);
	void _right(int repeatDelay, u32 step);
	void _jump(void);
	void _completeJump(void);
	void _setJump(int j);
	void _loadAllCovers(int i);
	static bool _calcTexLQLOD(STexture &tex);
	void _dropHQLOD(int i);
	bool _loadCoverTexPNG(u32 i, bool box, bool hq);
	CLRet _loadCoverTex(u32 i, bool box, bool hq);
	bool _invisibleCover(u32 x, u32 y);
	void _instantTarget(int i);
	void _transposeCover(std::vector<CCover> &dst, u32 rows, u32 columns, int pos);
	void _playSound(void);
	static bool _sortByPlayCount(CItem item1, CItem item2);
private:
	static int _picLoader(CCoverFlow *cf);
	static float _step(float cur, float tgt, float spd);
private:
	CCoverFlow(const CCoverFlow &);
	CCoverFlow &operator=(const CCoverFlow &);
};

#endif // !defined(__COVERFLOW_HPP)
