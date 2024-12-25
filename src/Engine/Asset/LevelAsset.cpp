#include "LevelAsset.h"
#include "Serialization/Serialization.h"

using namespace Seele;

LevelAsset::LevelAsset() {}

LevelAsset::LevelAsset(std::string_view folderPath, std::string_view name) : Asset(folderPath, name) {}

LevelAsset::~LevelAsset() {}

void LevelAsset::save(ArchiveBuffer& buffer) const { }

void LevelAsset::load(ArchiveBuffer& buffer) { }
