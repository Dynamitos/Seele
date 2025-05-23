#include "EnvironmentMapAsset.h"

using namespace Seele;

EnvironmentMapAsset::EnvironmentMapAsset() {}

EnvironmentMapAsset::EnvironmentMapAsset(std::string_view folderPath, std::string_view name) : Asset(folderPath, name) {}

EnvironmentMapAsset::~EnvironmentMapAsset() {}

void EnvironmentMapAsset::save(ArchiveBuffer&) const {}

void EnvironmentMapAsset::load(ArchiveBuffer&) {}
