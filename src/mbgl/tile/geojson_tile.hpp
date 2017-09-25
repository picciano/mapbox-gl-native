#pragma once

#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/style/sources/custom_vector_source.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
namespace mbgl {

class TileParameters;

class GeoJSONTile : public GeometryTile {
public:
    GeoJSONTile(const OverscaledTileID&,
                std::string sourceID,
                const TileParameters&,
                mapbox::geometry::feature_collection<int16_t>);

    void updateData(mapbox::geometry::feature_collection<int16_t>);

    void setNecessity(Necessity) final;
    
    void querySourceFeatures(
        std::vector<Feature>& result,
        const SourceQueryOptions&) override;
};

class CustomTile: public GeometryTile {
public:
    CustomTile(const OverscaledTileID&,
               std::string sourceID,
               const TileParameters&,
               const style::GeoJSONOptions);

    void setTileData(const CanonicalTileID& tileID, const style::FetchTileResult& result);

    void setNecessity(Necessity) final;
    
    void querySourceFeatures(
        std::vector<Feature>& result,
        const SourceQueryOptions&) override;
    
    ActorRef<style::FetchTileCallback> dataActor();
private:
    const style::GeoJSONOptions options;
    Actor<style::FetchTileCallback> updateDataActor;

};

} // namespace mbgl
