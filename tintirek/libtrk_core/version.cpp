/*
 *	version.cpp
 * 
 *	Library version number and utilities
 */

#include <string.h>
#include <sstream>

#include "trk_version.h"

const char* TrkVersion::getFullVersionInfo() const
{
	std::stringstream os;
	os << major_ << "." << minor_ << "." << patch_ << tag_;
	return strdup(os.str().c_str());
}

bool TrkVersion::TrkVerCompatible(const TrkVersion* my_version, const TrkVersion* lib_version)
{
	// @TODO: Maybe we can do full version match checking system?

	return my_version->getMajor() == lib_version->getMajor()
		&& my_version->getMinor() <= lib_version->getMinor();
}

bool TrkVersion::TrkVerEqual(const TrkVersion* my_version, const TrkVersion* other_version)
{
	return my_version->getMajor() == other_version->getMajor()
		&& my_version->getMinor() == other_version->getMinor()
		&& my_version->getPatch() == other_version->getPatch()
		&& 0 == strcmp(my_version->getTag(), other_version->getTag());
}

std::ostream& operator << (std::ostream& os, const TrkVersion& obj)
{
	os << obj.getMajor() << "." << obj.getMinor() << "." << obj.getPatch() << obj.getTag();
	return os;
}

bool TrkVersionHelper::CheckVersionList(const TrkVersion* MyVersion, const TrkVersion** CheckList, bool UseEqual, const char*& ErrorString)
{
	for (int i = 0; (CheckList[i]) != nullptr; i++)
	{
		const TrkVersion* current_check = CheckList[i];
		if (UseEqual ? 
			!TrkVersion::TrkVerEqual(MyVersion, current_check) :
			!TrkVersion::TrkVerCompatible(MyVersion, current_check))
		{
			std::stringstream os;
			os << "Version mistach in "
				<< current_check->getLabel() 
				<< " \"" << current_check->getFullVersionInfo() 
				<< "\", must be" 
				<< (UseEqual ? "" : " or greater than")
				<< " \"" << MyVersion->getFullVersionInfo()
				<< "\"";

			ErrorString = strdup(os.str().c_str());

			return false;
		}
	}

	return true;
}