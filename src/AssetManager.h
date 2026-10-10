// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MOONLANDER_ASSETMANAGER_H
#define MOONLANDER_ASSETMANAGER_H

#include <unordered_map>
#include <string>

#include <Corrade/Utility/Resource.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/Pointer.h>

#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>

#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/AbstractImporter.h>

#include <Magnum/ImageView.h>

#include "MoonLander.h"

namespace MoonLander {
    using namespace Magnum;
    using namespace Corrade;
    using Containers::Optional;
    using Containers::Pointer;

    class AssetManager {
    private:
        PluginManager::Manager<Trade::AbstractImporter> _manager;
        Containers::Pointer<Trade::AbstractImporter> _importer;
        std::unordered_map<std::string, GL::Texture2D> _assets;
        Utility::Resource _resource;

    public:
        AssetManager();

        void addTexture(const std::string &key, const std::string &filename);
        GL::Texture2D *getTexture(const std::string &key);
        Optional<Trade::ImageData2D> loadImage(const std::string &filename);
    };
}

#endif //MOONLANDER_ASSETMANAGER_H
