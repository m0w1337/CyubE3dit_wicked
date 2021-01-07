#pragma once


class cyVersion {
public:
	static const int major;
	static const int minor;
	static const int revision;
	static const std::string version_string;
	// major features
	static int GetMajor() {
		return major;
	}
	static int GetMinor() {
		return minor;
	}
	static int GetRevision() {
		return revision;
	}
	static const char* GetVersionString() {
		return version_string.c_str();
	}
};