#ifndef __CONFIG_HPP
#define __CONFIG_HPP

#include <map>
#include <string>

#include "video.h"
#include "smartptr.h"


class CColor;
class vec3;

class Config
{ 
public:
	Config(void);

	void clear(void) { m_domains.clear(); }
	bool load(const char *filename = 0);
	void save(void);
	bool loaded(void) const { return m_loaded; }
	bool has(const std::string &domain, const std::string &key) const;

	// Set
	void setString(const std::string &domain, const std::string &key, const std::string &val);
	void setBool(const std::string &domain, const std::string &key, bool val);
	void setOptBool(const std::string &domain, const std::string &key, int val);
	void setInt(const std::string &domain, const std::string &key, int val);
	void setFloat(const std::string &domain, const std::string &key, float val);
	void setVector3D(const std::string &domain, const std::string &key, const vec3 &val);
	void setColor(const std::string &domain, const std::string &key, const CColor &val);
	// Get
	std::string getString(const std::string &domain, const std::string &key, const std::string &defVal = "");
	bool getBool(const std::string &domain, const std::string &key, bool defVal = false);
	int getOptBool(const std::string &domain, const std::string &key, int defVal = 2);
	bool testOptBool(const std::string &domain, const std::string &key, bool defVal);
	int getInt(const std::string &domain, const std::string &key, int defVal = 0);
	float getFloat(const std::string &domain, const std::string &key, float defVal = 0.f);
	vec3 getVector3D(const std::string &domain, const std::string &key, const vec3 &defVal = vec3());
	CColor getColor(const std::string &domain, const std::string &key, const CColor &defVal = CColor());
	// 
	const std::string &firstDomain(void);
	const std::string &nextDomain(const std::string &start) const;
	const std::string &prevDomain(const std::string &start) const;
	bool hasDomain(const std::string &domain) const;
	void copyDomain(const std::string &dst, const std::string &src);
private:

	/* set parameter to domain map including all the wierd convertions */
	void __set(const std::string &domain, const std::string & key, const std::string & value);
	/* get parameter from domain map including all the wierd convertions */
	std::string &__get(const std::string &domain, const std::string & key);

	typedef std::map<std::string, std::string> KeyMap;
	typedef std::map<std::string, KeyMap> DomainMap;
private:
	bool m_loaded;
	DomainMap m_domains;
	std::string m_filename;
	DomainMap::iterator m_iter;
	static const std::string emptyString;
private:
	Config(const Config &) = delete;
	Config &operator=(const Config &) = delete;
};

#endif // !defined(__CONFIG_HPP)
