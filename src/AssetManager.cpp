// SPDX-FileCopyrightText: 2022-2026 Igal Alkon
// SPDX-License-Identifier: BSD-2-Clause

#include "AssetManager.h"

#include <Corrade/Utility/Debug.h>

namespace MoonLander {

AssetManager::AssetManager() : _resource{"sprites"} {
    _importer = _manager.loadAndInstantiate("AnyImageImporter");
    if (!_importer) std::exit(1);
}

void AssetManager::addTexture(const std::string &key, const std::string &filename) {
    Optional<Trade::ImageData2D> image = loadImage(filename);
    const Vector2i imageSize = image->size();

    GL::Texture2D texture;
    texture.setWrapping(GL::SamplerWrapping::ClampToEdge)
            .setMagnificationFilter(GL::SamplerFilter::Linear)
            .setMinificationFilter(GL::SamplerFilter::Linear)
            .setStorage(1, GL::textureFormat(image->format()), imageSize)
            .setSubImage(0, {}, *image)
            .setMagnificationFilter(SamplerFilter::Nearest);

    _assets.try_emplace(key, std::move(texture));
}

GL::Texture2D *AssetManager::getTexture(const std::string &key) {
    return &_assets[key];
}

Optional<Trade::ImageData2D> AssetManager::loadImage(const std::string &filename) {
    if (!_importer->openData(_resource.getRaw(filename)))
        Fatal{} << "Can't open image file with AnyImageImporter";

    Optional<Trade::ImageData2D> image = _importer->image2D(0);
    CORRADE_INTERNAL_ASSERT(image);

    return image;
}

} // namespace MoonLander
