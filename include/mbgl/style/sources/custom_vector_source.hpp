#pragma once

#include <mbgl/style/source.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/geojson.hpp>
#include <mbgl/util/variant.hpp>
#include <mbgl/actor/actor_ref.hpp>
#include <mbgl/util/noncopyable.hpp>

namespace mbgl {
namespace style {

struct Error { std::string message; };

using FetchTileResult = variant<
    mapbox::geojson::geojson,
    Error>;

using FetchTileCallback = std::function<void(const CanonicalTileID& tileID, const FetchTileResult&)>;
using FetchTileFunction = std::function<void(const CanonicalTileID&)>; //, FetchTileCallback)>;


class CustomTileLoader : private util::noncopyable {
public:
    CustomTileLoader(FetchTileFunction&& fetchTileFn);
    ~CustomTileLoader();
    void fetchTile(const CanonicalTileID& tileID, ActorRef<FetchTileCallback> callbackRef);
    void setTileData(const CanonicalTileID& tileID, const mapbox::geojson::geojson& data);

private:
    class Impl;
    Impl* impl = nullptr;
};

class CustomVectorSource : public Source {
public:
    CustomVectorSource(std::string id,
                       GeoJSONOptions options,
                       FetchTileFunction fetchTile);

    void loadDescription(FileSource&) final;
    void setTileData(const CanonicalTileID&, const mapbox::geojson::geojson& geojson);

    // Private implementation
    class Impl;
    const Impl& impl() const;
private:
    std::shared_ptr<Mailbox> mailbox;
    CustomTileLoader loader;

};

template <>
inline bool Source::is<CustomVectorSource>() const {
    return getType() == SourceType::CustomVector;
}

} // namespace style
} // namespace mbgl
