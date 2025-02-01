//
// Created by volt on 12/21/2024.
//

#include "World.hpp"
#include "SysCounter.hpp"
#include "entity.hpp"
#include "message.hpp"
#include "util.hpp"

#include <bit>
#include <list>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

// TODO better separation of concerns and general cleanup across this entire file

namespace lpg {
    namespace detail {
        static constexpr inline unsigned EntityPageSize = 256;
        static constexpr inline unsigned LogEntityPageSize = 8;
        static_assert(EntityPageSize == (1 << LogEntityPageSize));

        struct EntityPage {
            int32_t entityTypeId;
            int32_t pageId;
            int32_t parentPage;

            std::vector<int32_t> managedComponentPageOffsets;
            int32_t stride;
            int32_t currentSize;

            int32_t numOccupied = 0;

            std::array<uint64_t, (EntityPageSize + 63) / 64> occupancy;

            std::vector<std::byte> storage;

            [[nodiscard]] std::optional<int> findFreeOffset() const {
                for (int i = 0; i < occupancy.size(); i++) {
                    if (int bits = std::countr_zero(occupancy[i]); bits < 64) {
                        int result = bits + i * 64;
                        if (result >= EntityPageSize) {
                            return std::nullopt;
                        }
                        return result;
                    }
                }
                return std::nullopt;
            }

            struct PageReserveEntityResult {
                void* entity;
                int32_t offset;
            };

            /*
             * Reserves the leftmost free spot, if one exists, for storing an entity.
             * It is the caller's responsibility to construct the entity object at the returned address.
             */
            std::optional<PageReserveEntityResult> reserveEntity() {
                auto offOpt = findFreeOffset();
                if (not offOpt) {
                    return std::nullopt;
                }
                int off = *offOpt;
                int targetSize = (off + 1) * stride;

                if (storage.size() < targetSize) {
                    storage.resize(targetSize);
                }

                occupancy[off / 64] |= (1 << (off % 64));
                return PageReserveEntityResult{
                    .entity = entityPtr(off),
                    .offset = off
                };
            }

            bool isEmpty() const {
                for (int i = 0; i < occupancy.size(); i++) {
                    if (occupancy[i] != 0) {
                        return false;
                    }
                }
                return true;
            }

            bool isFull() const {
                for (int i = 0; i < occupancy.size(); i++) {
                    if (~occupancy[i] != 0) {
                        return false;
                    }
                }
                return true;
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
                return occupancy[offset / 64] & (1 << (offset % 64));
            }

            void* entityPtr(int offset) {
                return storage.data() + offset * stride;
            }
            void* componentPtr(int offset, int componentOfffset) {
                return storage.data() + componentOfffset + offset * stride;
            }

            [[nodiscard]] int numActiveEntities() const {
                int result = 0;
                for (int i = 0; i < occupancy.size(); i++) {
                    result += std::popcount(occupancy[i]);
                }
                return result;
            }

            [[nodiscard]] std::vector<std::pair<int, int> > getActiveRanges() const {
                std::vector<std::pair<int, int> > result;
                result.reserve(8);
                int curBegin = 0, curEnd = 0;
                for (int i = 0; i < occupancy.size(); i++) {
                    uint64_t tmp = occupancy[i];
                    int pos = 0;
                    while (tmp != 0) {
                        int t0 = std::countr_zero(tmp);
                        tmp >>= t0 & 63;
                        int t1 = std::countr_one(tmp);
                        tmp >>= t1 & 63;
                        if (t1 == 64) {
                            tmp = 0;
                        }

                        int begin = pos + t0;
                        int end = begin + t1;
                        pos += t0 + t1;

                        begin += 64 * i;
                        end += 64 * i;

                        if (curEnd == begin) {
                            curEnd = end;
                        } else if (curBegin != curEnd) {
                            result.emplace_back(curBegin, curEnd);
                            curBegin = begin;
                            curEnd = end;
                        }
                    }
                }
                if (curBegin != curEnd) {
                    result.emplace_back(curBegin, curEnd);
                }

                return result;
            }
        };



        struct WorldData {
            std::vector<EntityInterface> entityInterfaces_;
            std::unordered_map<std::string, int> entityTypeMap_;

            struct ComponentInfo {
                int pageId;
                int componentOffset;
            };

            std::vector<EntityPage> entityPages_;
            std::vector<std::vector<int>> entityPagesByType_;
            std::vector<std::vector<ComponentInfo>> entityPagesByComponentType_;

            std::vector<std::set<int>> freePagesByType_;

            std::vector<EntityDescriptor> arrIdToDescriptor_;
            std::vector<EntityID> arrDescriptorToId_;
            std::vector<EntityVersionNumber> arrIdToVersion_;

            std::unordered_map<std::string, int32_t> hmEntNameToId_;

            

            bool initFinalized = false;
        };
    } // namespace detail

    inline auto DecomposeEntityDescriptor(EntityDescriptor ed) {
        struct Result {
            uint32_t page;
            uint32_t offset;
        };
        return Result {
            .page = ed / detail::EntityPageSize,
            .offset = ed % detail::EntityPageSize,
        };
    }

    World::World() {
        this->worldData_ = detail::WorldData{};
    }

    World::~World() {
        auto& data = std::any_cast<detail::WorldData &>(worldData_);

        // Destroy all entities stored by each EntityPage.
        // Consider during refactor: in principle, EntityPage should be responsible for this
        for (auto& page: data.entityPages_) {
            auto& interface = data.entityInterfaces_.at(page.entityTypeId);
            for (auto [beg, end]: page.getActiveRanges()) {
                for (int i = beg; i < end; i++) {
                    interface.destroy(page.entityPtr(i));
                }
            }
        }
    }


    int World::registerEntityTypeImpl(const std::string& name, const EntityInterface& entityInterface) {
        auto& data = std::any_cast<detail::WorldData &>(data);

        if (data.entityTypeMap_.contains(name)) {
            throw std::runtime_error("Entity type already registered: " + name);
        }
        auto newEntTypeId = static_cast<int>(data.entityInterfaces_.size());
        data.entityInterfaces_.push_back(entityInterface);
        data.entityTypeMap_[name] = newEntTypeId;
        return newEntTypeId;
    }

    static void assertPagesCompatible(const detail::EntityPage& page1, const detail::EntityPage& page2) {
        if (page1.entityTypeId != page2.entityTypeId) {
            throw std::runtime_error("Pages are not compatible");
        }
    }

    void World::relocateEntity(int32_t targetDescriptor, int32_t sourceDescriptor) {
        auto& data = std::any_cast<detail::WorldData &>(worldData_);

        auto [dstPageId, dstOff] = DecomposeEntityDescriptor(targetDescriptor);
        auto [srcPageId, srcOff] = DecomposeEntityDescriptor(sourceDescriptor);

        auto& dstPage = data.entityPages_.at(dstPageId);
        auto& srcPage = data.entityPages_.at(srcPageId);

        assertPagesCompatible(srcPage, dstPage);

        if (dstPage.isEntityPresent(dstOff)) {
            throw std::runtime_error("invalid relocation");
        }

        const auto& interface = data.entityInterfaces_.at(srcPage.entityTypeId);

        void* dst = dstPage.entityPtr(dstOff);
        void* src = srcPage.entityPtr(srcOff);

        interface.move(dst, src);
        interface.destroy(src);
    }

    void World::swapEntities(int32_t targetDescriptor, int32_t sourceDescriptor) {
        auto& data = std::any_cast<detail::WorldData &>(worldData_);

        auto [dstPageId, dstOff] = DecomposeEntityDescriptor(targetDescriptor);
        auto [srcPageId, srcOff] = DecomposeEntityDescriptor(sourceDescriptor);

        auto& dstPage = data.entityPages_.at(dstPageId);
        auto& srcPage = data.entityPages_.at(srcPageId);

        assertPagesCompatible(srcPage, dstPage);

        if (not srcPage.isEntityPresent(srcOff) || not dstPage.isEntityPresent(dstOff)) {
            throw std::runtime_error("cannot swap with empty");
        }

        const auto& interface = data.entityInterfaces_.at(srcPage.entityTypeId);

        void* dst = dstPage.entityPtr(dstOff);
        void* src = srcPage.entityPtr(srcOff);

        interface.swap(dst, src);
    }

    int32_t World::createNewPage(int entityTypeId) {
        auto& data = std::any_cast<detail::WorldData &>(worldData_);

        const auto& interface = data.entityInterfaces_.at(entityTypeId);
        int32_t newPageId = data.entityPages_.size();



        vec::ResizeFor(data.entityPagesByType_, entityTypeId);
        vec::ResizeFor(data.freePagesByType_, entityTypeId);
        vec::ResizeFor(data.entityPagesByComponentType_, entityTypeId);

        data.entityPagesByType_[entityTypeId].push_back(newPageId);
        data.freePagesByType_[entityTypeId].insert(newPageId);
        for (const auto& compInfo: interface.embeddedComponents) {
            data.entityPagesByComponentType_[compInfo.entityTypeId].push_back({
                .pageId = newPageId,
                .componentOffset = compInfo.offset
            });
        }
        // TODO register managed components
        // TODO create pages for managed components

        data.entityPages_.push_back(
            detail::EntityPage{
                .occupancy = {},
                .stride = interface.entitySize,
                .storage = {},
                .currentSize = 0,
                .numOccupied = 0,
                .pageId = newPageId,
                .parentPage = -1,
                .managedComponentPageOffsets = {},
                .entityTypeId = entityTypeId
            }
        );
    }

    int32_t World::getFreePage(int entityTypeId) {
        auto& data = std::any_cast<detail::WorldData &>(worldData_);

        auto& pageSet = data.freePagesByType_.at(entityTypeId);
        if (not pageSet.empty()) {
            auto it = pageSet.begin();
            auto result = *it;
            pageSet.erase(it);
            return result;
        } else {
            return createNewPage(entityTypeId);
        }
    }

    detail::ReserveEntityResult World::reserveEntity(int entityTypeId) {
        auto& data = std::any_cast<detail::WorldData &>(worldData_);

        auto& page = data.entityPages_.at(getFreePage(entityTypeId));
        auto reserveResult = page.reserveEntity().value();
        if (page.isFull()) {
            data.freePagesByType_.at(entityTypeId).erase(page.pageId);
        }
        return detail::ReserveEntityResult{
            .entity = page.entityPtr(reserveResult.offset),
            .descriptor = static_cast<int32_t>(page.pageId * detail::EntityPageSize + reserveResult.offset)
        };
    }

    bool World::despawnEntity(EntityDescriptor entityDescriptor) {
        auto& data = std::any_cast<detail::WorldData &>(worldData_);

        auto [pageNum, offset] = DecomposeEntityDescriptor(entityDescriptor);
        auto& page = data.entityPages_.at(pageNum);
        auto& entInterface = data.entityInterfaces_.at(pageNum);
        if (page.isEntityPresent(offset)) {
            void* p = page.entityPtr(offset);

            auto preKillMessageTypeId = detail::GetMessageTypeId<PreKillMessage>();
            PreKillMessage preKillMessage {.descriptor = entityDescriptor};
            if (entInterface.sendMessage.size() > preKillMessageTypeId && entInterface.sendMessage[preKillMessageTypeId]) {
                entInterface.sendMessage[preKillMessageTypeId](&preKillMessage, p);
            }

            entInterface.destroy(p);
            page.releaseEntity(offset);
            if (page.numActiveEntities() == detail::EntityPageSize - 1) {
                data.freePagesByType_.at(page.entityTypeId).insert(page.pageId);
            }
            return true;
        }
        return false;
    }


    void World::finalizeInit() {
        auto& data = std::any_cast<detail::WorldData&>(worldData_);

        data.initFinalized = true;
    }

    void World::forEachEntityImpl(int entityTypeId, void* userdata, void (*onEntity)(void* userdata, void* entBegin, void* entEnd)) {
        auto& data = std::any_cast<detail::WorldData&>(worldData_);
        int entitySize = data.entityInterfaces_.at(entityTypeId).entitySize;

        for (auto pageId: data.entityPagesByType_.at(entityTypeId)) {
            auto& page = data.entityPages_.at(pageId);
            for (auto [beg, end]: page.getActiveRanges()) {
                onEntity(userdata, page.entityPtr(beg), page.entityPtr(end));
            }
        }

        for (const auto& cInfo: data.entityPagesByComponentType_.at(entityTypeId)) {
            auto& page = data.entityPages_.at(cInfo.pageId);
            for (auto [beg, end]: page.getActiveRanges()) {
                //TODO optimize this (use strided spans)
                for (int i = beg; i != end; ++i) {
                    void* pBeg = page.componentPtr(beg, cInfo.componentOffset);
                    void* pEnd = pBeg + entitySize;
                    onEntity(userdata, pBeg, pEnd);
                }
            }
        }

    }

    void World::registerMessageTypeImpl(const std::string& name, int messageTypeId) {
        auto& data = std::any_cast<detail::WorldData&>(worldData_);


        if (data.initFinalized) {
            throw std::runtime_error("Init has been finalized earlier. No new types may be registered at this point.");
        }
    }

    EntityVersionNumber World::getCurVersionNumOf(EntityID entId) {
        auto& data = std::any_cast<detail::WorldData&>(worldData_);
        return data.arrIdToVersion_.at(entId);
    }

    void World::saveEntityInterface(const EntityInterface& entityInterface, int32_t entTypeId) {
        auto& data = std::any_cast<detail::WorldData &>(worldData_);

        if (data.initFinalized) {
            throw std::runtime_error("Init has been finalized earlier. No new types may be registered at this point.");
        }

        if (data.entityInterfaces_.size() < entTypeId + 1) {
            data.entityInterfaces_.resize(entTypeId + 1);
        }

        data.entityInterfaces_[entTypeId] = entityInterface;
    }
} // namespace lpg
