//
// Created by volt on 12/21/2024.
//

#include "World.hpp"
#include "entity.hpp"
#include "SysCounter.hpp"

#include <vector>
#include <list>
#include <optional>
#include <unordered_map>
#include <bit>
#include <entt/entity/entity.hpp>

namespace lpg {

    namespace detail {
        static constexpr inline int EntityPageSize = 256;
        static constexpr inline int LogEntityPageSize = 8;
        static_assert(EntityPageSize == (1 << LogEntityPageSize));

        enum class EntityPageType {
            Regular,
            ManagedComponent,
            StaticComponent
        };

        struct EntityPage {
            EntityPageType type;

            int32_t entityTypeId;
            int32_t pageId;
            int32_t parentPage;
            std::vector<int32_t> componentPageOffsets;
            int32_t stride;
            int32_t currentSize;

            std::array<uint64_t, std::max(1, EntityPageSize/64)> occupancy;
            std::vector<std::byte> storage;

            [[nodiscard]] std::optional<int> findFreeId() const {
                for (int i=0; i<occupancy.size(); i++) {
                    if (int bits = std::countr_zero(occupancy[i]); bits < 64) {
                        int result = bits + i*64;
                        if (result >= EntityPageSize) {
                            return std::nullopt;
                        }
                        return result;
                    }
                }
                return std::nullopt;
            }

            struct ReserveEntityResult {
                void* entity;
                int32_t id;
            };

            /*
             * Reserves the leftmost free spot, if one exists, for storing an entity.
             * It is the caller's responsibility to construct the entity object at the returned address.
             */
            std::optional<ReserveEntityResult> reserveEntity() {
                auto idOpt = findFreeId();
                if (not idOpt) {
                    return std::nullopt;
                }
                int id = *idOpt;
                int targetSize = (id + 1) * EntityPageSize;

                if (storage.size() < targetSize) {
                    storage.resize(targetSize);
                }

                occupancy[id / 64] |= (1 << (id % 64));
                return ReserveEntityResult {
                    .entity = entityPtr(id),
                    .id = id
                };
            }

            /*
             * Marks the storage at the specified offset as ready to be reused for another entity.
             * It is the caller's responsibility to destroy the entity object before calling this function.
             */
            void releaseEntity(int offset) {
                occupancy[offset / 64] &= ~(1 << (offset % 64));
            }

            bool isEntityPresent(int offset) {
                if (offset >= EntityPageSize) {
                    return false;
                }
                return occupancy[offset/64] & (1 << (offset % 64));
            }

            void* entityPtr(int offset) {
                return storage.data() + offset*stride;
            }
        };


        struct WorldImpl {


            std::vector<EntityPage> entityPages_;
            std::vector<int> entityVersions;
            std::vector<std::vector<int>> entityPagesByType_;



        };

    }




} // lpg