#include "stdafx.h"
#include "cyVersion.h"
#include <string>


// main version
const int cyVersion::major = 0;
// minor features, major updates, breaking compatibility changes
const int cyVersion::minor = 1;
// minor bug fixes, alterations, refactors, updates
const int cyVersion::revision = 0;

const std::string cyVersion::version_string = std::to_string(cyVersion::major) + "." + std::to_string(cyVersion::minor) + "." + std::to_string(cyVersion::revision);


