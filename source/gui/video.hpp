
#ifndef __VIDEO_HPP
#define __VIDEO_HPP

#include <gccore.h>

#include "smartptr.hpp"
#include "vector.hpp"
#include "texture.hpp"

class CTexCoord
{
public:
	float x;
	float y;
public:
	CTexCoord(void) { x = 0.f; y = 0.f; }
	CTexCoord(float px, float py) { x = px; y = py; }
};

class CColor : public GXColor
{
public:
	CColor(void) { r = 0; g = 0; b = 0; a = 0xFF; }
	CColor(u8 pr, u8 pg, u8 pb) { r = pr; g = pg; b = pb; a = 0xFF; }
	CColor(u8 pr, u8 pg, u8 pb, u8 pa) { r = pr; g = pg; b = pb; a = pa; }
	CColor(u32 rgba8)
	{
		a = (rgba8 & 0xFF000000) >> 24;
		r = (rgba8 & 0x00FF0000) >> 16;
		g = (rgba8 & 0x0000FF00) >> 8;
		b = rgba8 & 0x000000FF;
	}
	bool operator==(const CColor &c) const { return c.r == r && c.g == g && c.b == b && c.a == a; }
	bool operator!=(const CColor &c) const { return c.r != r || c.g != g || c.b != b || c.a != a; }
	void blend(const CColor &src);
	static CColor interpolate(const CColor &c1, const CColor &c2, u8 n);
};

class CVideo
{
public:
	CVideo(void);
	~CVideo(void);
	void init(void);
	void prepare(void);
	void setAA(u8 aa, bool alpha = false, int width = 0, int height = 0);
	void prepareAAPass(int aaStep);
	void render(void);
	void renderAAPass(int aaStep);
	void drawAAScene(bool fs = true);
	void renderToTexture(STexture &tex, bool clear);
	void cleanup(void);
	void setup2DProjection(bool setViewPort = true, bool noScale = false);
	u32 width(void) const { return m_rmode->fbWidth; }
	u32 height(void) const { return m_rmode->efbHeight; }
	bool wide(void) const { return m_wide; }
	void set2DViewport(u32 w, u32 h, int x, int y);
	void prepareStencil(void);
	void renderStencil(void);
	int stencilVal(int x, int y);
	void waitMessage(const STexture &tex);
	void shiftViewPort(float x, float y);
private:
	GXRModeObj *m_rmode;
	void *m_frameBuf[2];
	int m_curFB;
	void *m_fifo;
	float m_yScale;
	u32 m_xfbHeight;
	bool m_wide;
	u32 m_width2D;
	u32 m_height2D;
	int m_x2D;
	int m_y2D;
	u8 m_aa;
	bool m_aaAlpha;
	int m_aaWidth;
	int m_aaHeight;
	SmartBuf m_stencil;
	SmartBuf m_aaBuffer[8];
	u32 m_aaBufferSize[8];
	float m_vpX;
	float m_vpY;
	float m_vpW;
	float m_vpH;
	// 
	static const int _stencilWidth;
	static const int _stencilHeight;
	static const float _jitter2[2][2];
	static const float _jitter3[3][2];
	static const float _jitter4[4][2];
	static const float _jitter5[5][2];
	static const float _jitter6[6][2];
	static const float _jitter8[8][2];
private:
	void _drawAASceneWithAlpha(float w, float h);
	void _setViewPort(float x, float y, float w, float h);
private:
	CVideo(const CVideo &);
};

#endif //!defined(__VIDEO_HPP)
