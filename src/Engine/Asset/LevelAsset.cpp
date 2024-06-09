#include "LevelAsset.h"

using namespace Seele;

LevelAsset::LevelAsset() {}

LevelAsset::LevelAsset(std::string_view folderPath, std::string_view name) : Asset(folderPath, name) {}

LevelAsset::~LevelAsset() {}

void LevelAsset::save(ArchiveBuffer&) const {}

void LevelAsset::load(ArchiveBuffer&) {}
