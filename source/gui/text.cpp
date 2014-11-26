
#include "text.h"
#include <cstdio>


static const char *g_whitespaces = " \f\n\r\t\v";

// Simplified use of sprintf
const char *fmt(const char *format, ...)
{
	enum {
		MAX_MSG_SIZE	= 512,
		MAX_USES		= 8
	};

	static char buffer[MAX_USES][MAX_MSG_SIZE];
	static int currentStr = 0;
	va_list va;

	currentStr = (currentStr + 1) % MAX_USES;
	va_start(va, format);
	vsnprintf(buffer[currentStr], MAX_MSG_SIZE, format, va);
	buffer[currentStr][MAX_MSG_SIZE - 1] = '\0';
	va_end(va);

	return buffer[currentStr];
}

// 
std::string sfmt(const char *format, ...)
{
	va_list va;
	u32 length;
	char *tmp;
	std::string s;

	va_start(va, format);
	length = vsnprintf(0, 0, format, va) + 1;
	va_end(va);
	tmp = new char[length + 1];
	va_start(va, format);
	vsnprintf(tmp, length, format, va);
	va_end(va);
	s = tmp;
	delete[] tmp;
	return s;
}

static inline bool fmtCount(const std::string &format, int &i, int &s)
{
	int state = 0;

	i = 0;
	s = 0;
	for (u32 k = 0; k < format.size(); ++k)
	{
		if (state == 0)
		{
			if (format[k] == L'%')
				state = 1;
		}
		else if (state == 1)
		{
			switch (format[k])
			{
				case L'%':
					state = 0;
					break;
				case L'i':
				case L'd':
					state = 0;
					++i;
					break;
				case L's':
					state = 0;
					++s;
					break;
				default:
					return false;
			}
		}
	}
	return true;
}

// Only handles the cases i need for translations : plain %i and %s
bool checkFmt(const std::string &ref, const std::string &format)
{
	int s;
	int i;
	int refs;
	int refi;
	if (!fmtCount(ref, refi, refs))
		return false;
	if (!fmtCount(format, i, s))
		return false;
	return i == refi && s == refs;
}

std::string wfmt(const std::string &format, ...)
{
	// Don't care about performance
	va_list va;
	std::string f(format);
	u32 length;
	char *tmp;

	va_start(va, format);
	length = vsnprintf(0, 0, f.c_str(), va) + 1;
	va_end(va);
	tmp = new char[length + 1];
	va_start(va, format);
	vsnprintf(tmp, length, f.c_str(), va);
	va_end(va);
	std::string ws;
	ws = tmp;
	delete[] tmp;
	return ws;
}

std::string vectorToString(const std::vector<std::string> &vect, char sep)
{
	std::string s;

	for (u32 i = 0; i < vect.size(); ++i)
	{
		if (i > 0)
			s.push_back(sep);
		s.append(vect[i]);
	}
	return s;
}

std::vector<std::string> stringToVector(const std::string &text, char sep)
{
	std::vector<std::string> v;
	if (text.empty())
		return v;
	u32 count = 1;
	for (u32 i = 0; i < text.size(); ++i)
		if (text[i] == sep)
			++count;
	v.reserve(count);
	std::string::size_type off = 0;
	std::string::size_type i = 0;
	do
	{
		i = text.find_first_of(sep, off);
		if (i != std::string::npos)
		{
			std::string ws(text.substr(off, i - off));
			v.push_back(ws);
			off = i + 1;
		}
		else
			v.push_back(text.substr(off));
	} while (i != std::string::npos);
	return v;
}


bool SFont::fromBuffer(const u8 *buffer, u32 bufferSize, u32 size, u32 lspacing)
{
	size = std::min(std::max(6u, size), 1000u);
	lspacing = std::min(std::max(6u, lspacing), 1000u);
	lineSpacing = lspacing;
	font.release();
	data.release();
	dataSize = 0;
	font = SmartPtr<FreeTypeGX>(new FreeTypeGX);
	if (!font)
		return false;
	font->loadFont(buffer, bufferSize, size, false);
	return true;
}

bool SFont::newSize(u32 size, u32 lspacing)
{
	if (!data)
		return false;
	size = std::min(std::max(6u, size), 1000u);
	lspacing = std::min(std::max(6u, lspacing), 1000u);
	lineSpacing = lspacing;
	font.release();
	font = SmartPtr<FreeTypeGX>(new FreeTypeGX);
	if (!font)
		return false;
	font->loadFont(data.get(), dataSize, size, false);
	return true;
}

bool SFont::fromFile(const char *filename, u32 size, u32 lspacing)
{
	FILE *file;
	u32 fileSize;

	size = std::min(std::max(6u, size), 1000u);
	lspacing = std::min(std::max(6u, lspacing), 1000u);
	font.release();
	data.release();
	dataSize = 0;
	lineSpacing = lspacing;
	file = fopen(filename, "rb");
	
	if (file == NULL)
		return false;
		
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	if (fileSize == 0)
	{
		fclose(file);
		return false;
	}
	
	data = smartMem2Alloc(fileSize);	// Use MEM2 because of big chinese fonts
	if (!!data)
		fread(data.get(), 1, fileSize, file);
	
	fclose(file);
	file = NULL;
	
	if (!data)
		return false;
	dataSize = fileSize;
	font = SmartPtr<FreeTypeGX>(new FreeTypeGX);
	if (!font)
	{
		data.release();
		return false;
	}
	font->loadFont(data.get(), dataSize, size, false);
	return true;
}

void CText::setText(SFont font, const std::string &t)
{
	CText::SWord w;
	std::vector<std::string> lines;

	m_lines.clear();
	m_font = font;
	if (!m_font.font)
		return;
	// Don't care about performance
	lines = stringToVector(t, L'\n');
	m_lines.reserve(lines.size());
	// 
	for (u32 k = 0; k < lines.size(); ++k)
	{
		std::string &l = lines[k];
		m_lines.push_back(CText::CLine());
		m_lines.back().reserve(32);
		std::string::size_type i = l.find_first_not_of(g_whitespaces);
		std::string::size_type j;
		while (i != std::string::npos)
		{
			j = l.find_first_of(g_whitespaces, i);
			if (j != std::string::npos && j > i)
			{
				w.text.assign(l, i, j - i);
				m_lines.back().push_back(w);
				i = l.find_first_not_of(g_whitespaces, j);
			}
			else if (j == std::string::npos)
			{
				w.text.assign(l, i, l.size() - i);
				m_lines.back().push_back(w);
				i = std::string::npos;
			}
		}
	}
}

void CText::setFrame(float width, u16 style, bool ignoreNewlines, bool instant)
{
	float posX;
	float posY;
	float wordWidth;
	float space;
	float shift;
	u32 lineBeg;

	if (!m_font.font)
		return;
	space = m_font.font->getWidth(L" ");

	posX = 0.f;
	posY = 0.f;
	lineBeg = 0;
	for (u32 k = 0; k < m_lines.size(); ++k)
	{
		CText::CLine &words = m_lines[k];
		if (words.empty())
			posY += (float)m_font.lineSpacing;
		
		for (u32 i = 0; i < words.size(); ++i)
		{
			std::wstring test(words[i].text.begin(), words[i].text.end());
			wordWidth = m_font.font->getWidth(test.c_str());

			if (posX == 0.f || posX + (float)wordWidth <= width)
			{
				words[i].targetPos = vec3(posX, posY, 0.f);
				posX += wordWidth + space;
			}
			else
			{
				posY += (float)m_font.lineSpacing;
				words[i].targetPos = vec3(0.f, posY, 0.f);
				if ((style & (FTGX_JUSTIFY_CENTER | FTGX_JUSTIFY_RIGHT)) != 0)
				{
					posX -= space;
					shift = (style & FTGX_JUSTIFY_CENTER) != 0 ? -posX * 0.5f : -posX;
					for (u32 j = lineBeg; j < i; ++j)
						words[j].targetPos.x += shift;
				}
				posX = wordWidth + space;
				lineBeg = i;
			}
		}
		// Quick patch for newline support
		if (!ignoreNewlines && k + 1 < m_lines.size())			posX = 9999999.f;
	}
	if ((style & (FTGX_JUSTIFY_CENTER | FTGX_JUSTIFY_RIGHT)) != 0)
	{
		posX -= space;
		shift = (style & FTGX_JUSTIFY_CENTER) != 0 ? -posX * 0.5f : -posX;
		for (u32 k = 0; k < m_lines.size(); ++k)
			for (u32 j = lineBeg; j < m_lines[k].size(); ++j)
				m_lines[k][j].targetPos.x += shift;
	}
	if ((style & (FTGX_ALIGN_MIDDLE | FTGX_ALIGN_BOTTOM)) != 0)
	{
		posY += (float)m_font.lineSpacing;
		shift = (style & FTGX_ALIGN_MIDDLE) != 0 ? -posY * 0.5f : -posY;
		for (u32 k = 0; k < m_lines.size(); ++k)
			for (u32 j = 0; j < m_lines[k].size(); ++j)
				m_lines[k][j].targetPos.y += shift;
	}
	if (instant)
		for (u32 k = 0; k < m_lines.size(); ++k)
			for (u32 i = 0; i < m_lines[k].size(); ++i)
				m_lines[k][i].pos = m_lines[k][i].targetPos;
}

void CText::setColor(const CColor &c)
{
	m_color = c;
}

void CText::tick(void)
{
	for (u32 k = 0; k < m_lines.size(); ++k)
	{
		for (u32 i = 0; i < m_lines[k].size(); ++i)
		{
			m_lines[k][i].pos += (m_lines[k][i].targetPos - m_lines[k][i].pos) * 0.05f;
			//m_lines[k][i].pos += (m_lines[k][i].targetPos - m_lines[k][i].pos) * 0.0001f;
		}
	}
}

void CText::draw(void)
{
	if (!m_font.font)
		return;

	std::wstring test;
	test.reserve(1024);

	for (u32 k = 0; k < m_lines.size(); ++k)
	{
		for (u32 i = 0; i < m_lines[k].size(); ++i)
		{
			m_font.font->setX(m_lines[k][i].pos.x);
			m_font.font->setY(m_lines[k][i].pos.y);

			//m_font.font->drawText(0, m_font.lineSpacing, m_lines[k][i].text.c_str(), m_color);

			m_font.font->drawText(0, m_font.lineSpacing, m_lines[k][i].text, m_color);

			//test();
			//test.assign(m_lines[k][i].text.begin(), m_lines[k][i].text.end());

			//m_font.font->drawText(0, m_font.lineSpacing, test.c_str(), m_color);
		}
	}
}
