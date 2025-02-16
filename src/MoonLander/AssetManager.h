#ifndef MAGNUM_MOONLANDER_ASSETMANAGER_H
#define MAGNUM_MOONLANDER_ASSETMANAGER_H

#include <unordered_map>

#include <Corrade/Utility/Resource.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Containers/Optional.h>

#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>

#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/AbstractImporter.h>

#include <Magnum/ImageView.h>

namespace Magnum::Game {
    class AssetManager {
    private:
        PluginManager::Manager<Trade::AbstractImporter> _manager;
        Containers::Pointer<Trade::AbstractImporter> _importer;
        std::unordered_map<std::string, GL::Texture2D> _assets;
        Utility::Resource _resource;
    public:
        AssetManager(): _resource{"sprites"} {
            /* Load image importer plugin */
            _importer = _manager.loadAndInstantiate("AnyImageImporter");

            /* If importer was not loaded, exit with error */
            if(!_importer) std::exit(1);
        };

        void addTexture(const std::string& key, const std::string& filename) {
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

        GL::Texture2D* getTexture(const std::string& key) {
            return &_assets[key];
        }

        Optional<Trade::ImageData2D> loadImage(const std::string& filename) {
            if(!_importer->openData(_resource.getRaw(filename)))
                Fatal{} << "Can't open image file with AnyImageImporter";

            Optional<Trade::ImageData2D> image = _importer->image2D(0);
            CORRADE_INTERNAL_ASSERT(image);

            return image;
        }

    };
}


#endif //MAGNUM_MOONLANDER_ASSETMANAGER_H
