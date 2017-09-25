#include <mbgl/style/sources/custom_vector_source.hpp>
#include <mbgl/style/sources/custom_vector_source_impl.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/tile/tile_id_hash.hpp>

namespace mbgl {
namespace style {

class CustomTileLoader::Impl {
public:
    Impl(FetchTileFunction&& fetchTileFn) {
        fetchTileFunction = std::move(fetchTileFn);
    }

    void fetchTile(const CanonicalTileID& tileID, ActorRef<FetchTileCallback> callbackRef) {
        printf("CustomTileLoader::Requesting tile: %d/%d/%d\n", tileID.z, tileID.x, tileID.y);
        fetchTileFunction(tileID);
        auto insertResult = tileCallbackMap.insert({tileID, callbackRef});
        if (insertResult.second == false) {
            insertResult.first->second = callbackRef;
        }
    }

    void setTileData(const CanonicalTileID& tileID, const mapbox::geojson::geojson& data) {
        auto iter = tileCallbackMap.find(tileID);
        if (iter == tileCallbackMap.end()) return;
        printf("CustomTileLoader::Invoke SetTile tile: %d/%d/%d\n", tileID.z, tileID.x, tileID.y);
        iter->second.invoke(&FetchTileCallback::operator(), tileID, data);
    }

private:
    FetchTileFunction fetchTileFunction;
    std::unordered_map<CanonicalTileID, ActorRef<FetchTileCallback>> tileCallbackMap;
};


CustomTileLoader::CustomTileLoader(FetchTileFunction&& fetchTileFn)
    : impl(new CustomTileLoader::Impl(std::move(fetchTileFn))) {

}

CustomTileLoader::~CustomTileLoader() {
    delete impl;
    impl = nullptr;
}
void CustomTileLoader::fetchTile(const CanonicalTileID& tileID, ActorRef<FetchTileCallback> callbackRef) {
    impl->fetchTile(tileID, callbackRef);
}

void CustomTileLoader::setTileData(const CanonicalTileID& tileID, const mapbox::geojson::geojson& data) {
    impl->setTileData(tileID, data);
}

CustomVectorSource::CustomVectorSource(std::string id,
                                       const GeoJSONOptions options,
                                       FetchTileFunction fetchTileFn)
    : Source(makeMutable<CustomVectorSource::Impl>(std::move(id), options)),
    mailbox(std::make_shared<Mailbox>(*Scheduler::GetCurrent())),
    loader(std::move(fetchTileFn)) {
}

const CustomVectorSource::Impl& CustomVectorSource::impl() const {
    return static_cast<const CustomVectorSource::Impl&>(*baseImpl);
}
void CustomVectorSource::loadDescription(FileSource&) {
    baseImpl = makeMutable<CustomVectorSource::Impl>(impl(), ActorRef<CustomTileLoader>(loader, mailbox));
    loaded = true;
}

void CustomVectorSource::setTileData(const CanonicalTileID& tileID,
                                     const mapbox::geojson::geojson& data) {
    loader.setTileData(tileID, data);
}

} // namespace style
} // namespace mbgl
