
#include <fstream>

#include "config.h"
#include "text.h"

using namespace std;

static const char *g_whitespaces = " \f\n\r\t\v";
static const int g_floatPrecision = 10;

const string Config::emptyString;

Config::Config(void) :
	m_loaded(false), m_domains(), m_filename(), m_iter()
{
}

void trimEnd(std::string &line)
{
	std::string::size_type i = line.find_last_not_of(g_whitespaces);
	if (i == std::string::npos)
		line.clear();
	else
		line.resize(i + 1);
	
}

void trim(std::string &line)
{
	std::string::size_type i = line.find_last_not_of(g_whitespaces);
	if (i == string::npos)
	{
		line.clear();
	}
	else
		line.resize(i + 1);

	i = line.find_first_not_of(g_whitespaces);
	if (i > 0)
		line.erase(0, i);
}

void upperCase(std::string &text)
{
	char c;

	for (std::string::size_type i = 0; i < text.size(); ++i)
	{
		c = text[i];
		if (c >= 'a' && c <= 'z')
			text[i] = c & 0xDF;
	}
}

void lowerCase(std::string &text)
{
	char c;

	for (std::string::size_type i = 0; i < text.size(); ++i)
	{
		c = text[i];
		if (c >= 'A' && c <= 'Z')
			text[i] = c | 0x20;
	}
}

static string unescNewlines(const string &text)
{
	string s;
	bool escaping = false;

	s.reserve(text.size());
	for (string::size_type i = 0; i < text.size(); ++i)
	{
		if (escaping)
		{
			switch (text[i])
			{
				case 'n':
					s.push_back('\n');
					break;
				default:
					s.push_back(text[i]);
			}
			escaping = false;
		}
		else if (text[i] == '\\')
			escaping = true;
		else
			s.push_back(text[i]);
	}
	return s;
}

static string escNewlines(const string &text)
{
	string s;

	s.reserve(text.size());
	for (string::size_type i = 0; i < text.size(); ++i)
	{
		switch (text[i])
		{
			case '\n':
				s.push_back('\\');
				s.push_back('n');
				break;
			case '\\':
				s.push_back('\\');
				s.push_back('\\');
				break;
			default:
				s.push_back(text[i]);
		}
	}
	return s;
}

bool Config::hasDomain(const string &domain) const
{
	return m_domains.find(domain) != m_domains.end();
}

void Config::copyDomain(const string &dst, const string &src)
{
	m_domains[dst] = m_domains[src];
}

const string &Config::firstDomain(void)
{
	m_iter = m_domains.begin();
	if (m_iter == m_domains.end())
		return Config::emptyString;
	return m_iter->first;
}

const string &Config::nextDomain(const string &start) const
{
	Config::DomainMap::const_iterator i;
	Config::DomainMap::const_iterator j;
	if (m_domains.empty())
		return Config::emptyString;
	i = m_domains.find(start);
	if (i == m_domains.end())
		return m_domains.begin()->first;
	j = i;
	++j;
	return j != m_domains.end() ? j->first : i->first;
}

const string &Config::prevDomain(const string &start) const
{
	Config::DomainMap::const_iterator i;
	if (m_domains.empty())
		return Config::emptyString;
	i = m_domains.find(start);
	if (i == m_domains.end() || i == m_domains.begin())
		return m_domains.begin()->first;
	--i;
	return i->first;
}

bool Config::load(const char *filename)
{
	ifstream file(filename, ios::in | ios::binary);
	string line;
	u32 n;
	string domain("");

	m_loaded = false;
	m_filename = filename;
	n = 0;
	if (!file.is_open())
		return m_loaded;
	
	m_domains.clear();

	while (file.good())
	{
		line.clear();
		getline(file, line, '\n');
		++n;
		if (!file.bad() && !file.fail())
		{
			trimEnd(line);

			if (line.empty() || line[0] == '#')
				continue;

			if (line[0] == '[')
			{
				string::size_type i = line.find_first_of(']');
				if (i != string::npos && i > 1)
				{
					domain = line.substr(1, i - 1);
					upperCase(domain);

					if (m_domains.count(domain) > 0)
						domain.clear();
				}
			}
			else
				if (!domain.empty())
				{
					string::size_type i = line.find_first_of('=');
					if (i != string::npos && i > 0)
					{
						std::string key = line.substr(0, i);
						std::string value = line.substr(i + 1);

						trim(key);
						lowerCase(key);

						trim(value);

						m_domains[domain][key] = unescNewlines(value);
					}
				}
		}
	}
	m_loaded = true;
	return m_loaded;
}

void Config::save(void)
{
	ofstream file(m_filename.c_str(), ios::out | ios::binary);
	for (Config::DomainMap::iterator k = m_domains.begin(); k != m_domains.end(); ++k)
	{
		Config::KeyMap *m = &k->second;
		file << '\n' << '[' << k->first << ']' << '\n';
		for (Config::KeyMap::iterator l = m->begin(); l != m->end(); ++l)
			file << l->first << '=' << escNewlines(l->second) << '\n';
	}
}

bool Config::has(const std::string &domain, const std::string &key) const
{
	if (domain.empty() || key.empty())
		return false;
	
	std::string temp_domain = domain;
	std::string temp_key = key;
	upperCase(temp_domain);
	lowerCase(temp_key);

	DomainMap::const_iterator i = m_domains.find(temp_domain);

	if (i == m_domains.end())
		return false;

	return i->second.count(temp_key) > 0;
}

void Config::__set(const std::string &domain, const std::string & key, const std::string & value)
{
	std::string temp_domain = domain;
	std::string temp_key = key;
	upperCase(temp_domain);
	lowerCase(temp_key);

	m_domains[temp_domain][temp_key] = value;
}

void Config::setStr(const string &domain, const string &key, const std::string &val)
{
	if (domain.empty() || key.empty())
		return;

	__set(domain, key, val);
}

/*
void Config::setBool(const string &domain, const string &key, bool val)
{
	if (domain.empty() || key.empty())
		return;

	__set(domain, key, val ? "yes" : "no");
}

void Config::setOptBool(const string &domain, const string &key, int val)
{
	if (domain.empty() || key.empty())
		return;
	
	const char* value = nullptr;
	switch (val)
	{
		case 0:
			value = "no";
			break;
		case 1:
			value = "yes";
			break;
		default:
			value = "default";
	}

	__set(domain, key, value);
}
*/

void Config::setInt(const string &domain, const string &key, int val)
{
	if (domain.empty() || key.empty())
		return;

	__set(domain, key, sfmt("%i", val));
}

void Config::setFloat(const string &domain, const string &key, float val)
{
	if (domain.empty() || key.empty())
		return;

	__set(domain, key, sfmt("%.*g", g_floatPrecision, val));
}

void Config::setVector3D(const std::string &domain, const std::string &key, const vec3 &val)
{
	if (domain.empty() || key.empty())
		return;

	__set(domain, key, sfmt("%.*g, %.*g, %.*g", g_floatPrecision, val.x, g_floatPrecision, val.y, g_floatPrecision, val.z));
}

void Config::setColor(const std::string &domain, const std::string &key, const CColor &val)
{
	if (domain.empty() || key.empty())
		return;

	__set(domain, key, sfmt("#%.2X%.2X%.2X%.2X", val.r, val.g, val.b, val.a));
}

std::string &Config::__get(const std::string &domain, const std::string & key)
{
	std::string temp_domain = domain;
	std::string temp_key = key;

	upperCase(temp_domain);
	lowerCase(temp_key);

	return m_domains[temp_domain][temp_key];
}

std::string Config::getStr(const string &domain, const string &key, const std::string &defVal)
{
	if (domain.empty() || key.empty())
		return defVal;
	
	string &data = __get(domain, key);
	if (data.empty())
	{
		data = defVal;
		return defVal;
	}
	return data;
}

/*
bool Config::getBool(const string &domain, const string &key, bool defVal)
{
	if (domain.empty() || key.empty())
		return defVal;
	
	std::string &data = __get(domain, key);
	if (data.empty())
	{
		data = defVal ? "yes" : "no";
		return defVal;
	}

	trim(data);
	lowerCase(data);

	return (data == "yes" || data == "true" || data == "y" || data == "1");
}

bool Config::testOptBool(const string &domain, const string &key, bool defVal)
{
	if (domain.empty() || key.empty())
		return defVal;

	std::string temp_domain = domain;
	std::string temp_key = key;
	upperCase(temp_domain);
	lowerCase(temp_key);

	KeyMap &km = m_domains[temp_domain];
	KeyMap::iterator i = km.find(temp_key);
	if (i == km.end())
		return defVal;

	std::string result = i->second;
	trim(result);
	lowerCase(result);


	if (result == "yes")
		return true;
	else if (result == "no")
		return false;

	return defVal;
}

int Config::getInt(const string &domain, const string &key, int defVal)
{
	if (domain.empty() || key.empty())
		return defVal;

	std::string temp_domain = domain;
	std::string temp_key = key;
	upperCase(temp_domain);
	lowerCase(temp_key);

	std::string &data = m_domains[temp_domain][temp_key];
	if (data.empty())
	{
		switch (defVal)
		{
			case 0:
				data = "no";
				break;
			case 1:
				data = "yes";
				break;
			default:
				data = "default";
		}
		return defVal;
	}

	trim(data);
	lowerCase(data);

	if (data == "yes")
		return 1;
	else if (data == "no")
		return 0;
	return 2;
}
*/

int Config::getInt(const string &domain, const string &key, int defVal)
{
	if (domain.empty() || key.empty())
		return defVal;
	
	string &data = __get(domain, key);
	if (data.empty())
	{
		data = sfmt("%i", defVal);
		return defVal;
	}
	return strtol(data.c_str(), 0, 10);
}

float Config::getFloat(const string &domain, const string &key, float defVal)
{
	if (domain.empty() || key.empty())
		return defVal;
	string &data = __get(domain, key);
	if (data.empty())
	{
		data = sfmt("%.*g", g_floatPrecision, defVal);
		return defVal;
	}
	return strtod(data.c_str(), 0);
}

vec3 Config::getVector3D(const std::string &domain, const std::string &key, const vec3 &defVal)
{
	if (domain.empty() || key.empty())
		return defVal;
	string &data = __get(domain, key);
	string::size_type i;
	string::size_type j = string::npos;
	i = data.find_first_of(',');
	if (i != string::npos)
		j = data.find_first_of(',', i + 1);
	if (j == string::npos)
	{
		data = sfmt("%.*g, %.*g, %.*g", g_floatPrecision, defVal.x, g_floatPrecision, defVal.y, g_floatPrecision, defVal.z);
		return defVal;
	}
	return vec3(strtod(data.substr(0, i).c_str(), 0), strtod(data.substr(i + 1, j - i - 1).c_str(), 0), strtod(data.substr(j + 1).c_str(), 0));
}

CColor Config::getColor(const std::string &domain, const std::string &key, const CColor &defVal)
{
	if (domain.empty() || key.empty())
		return defVal;

	std::string temp_key = key;
	std::string temp_domain = domain;
	upperCase(temp_domain);
	lowerCase(temp_key);

	std::string &data = m_domains[temp_domain][temp_key];
	std::string text = data;
	trim(text);
	upperCase(text);

	u32 i = (u32)text.find_first_of('#');
	if (i != string::npos)
	{
		text.erase(0, i + 1);
		i = (u32)text.find_first_not_of("0123456789ABCDEF");
		if ((i != string::npos && i >= 6) || (i == string::npos && text.size() >= 6))
		{
			u32 n = ((i != string::npos && i >= 8) || (i == string::npos && text.size() >= 8)) ? 8 : 6;
			for (i = 0; i < n; ++i)
				if (text[i] <= '9')
					text[i] -= '0';
				else
					text[i] -= 'A' - 10;

			CColor c(text[0] * 0x10 + text[1], text[2] * 0x10 + text[3], text[4] * 0x10 + text[5], 1.f);
			
			if (n == 8)
				c.a = text[6] * 0x10 + text[7];
			
			return c;
		}
	}
	data = sfmt("#%.2X%.2X%.2X%.2X", defVal.r, defVal.g, defVal.b, defVal.a);
	return defVal;
}
