#ifndef __TEXT_HPP
#define __TEXT_HPP

#include <vector>
#include <string>

#include "FreeTypeGX.h"
#include "video.h"

#include "smartptr.h"

struct SFont
{
	SmartBuf data;
	size_t dataSize;
	SmartPtr<FreeTypeGX> font;
	u32 lineSpacing;
public:
	bool fromBuffer(const u8 *buffer, u32 bufferSize, u32 size, u32 lspacing);
	bool fromFile(const char *filename, u32 size, u32 lspacing);
	bool newSize(u32 size, u32 lspacing);
	SFont(void) : dataSize(0), lineSpacing(0) { }
};

class CText
{
public:
	void setText(SFont font, const std::string &t);
	void setColor(const CColor &c);
	void setFrame(float width, u16 style, bool ignoreNewlines = false, bool instant = false);
	void tick(void);
	void draw(void);
private:
	struct SWord
	{
		std::string text;
		std::wstring text_render_cache_;
		vec3 pos;
		vec3 targetPos;
	};
private:
	typedef std::vector<SWord> CLine;
	
	std::vector<CLine> m_lines;
	SFont m_font;
	CColor m_color;
};

// Nothing to do with CText. Q&D helpers for string formating.
const char *fmt(const char *format, ...);
std::string sfmt(const char *format, ...);
std::string wfmt(const std::string &format, ...);
bool checkFmt(const std::string &ref, const std::string &format);
std::string vectorToString(const std::vector<std::string> &vect, char sep);
std::vector<std::string> stringToVector(const std::string &text, char sep);
std::vector<std::string> stringToVector(const std::string &text, char sep);

#endif // !defined(__TEXT_HPP)
