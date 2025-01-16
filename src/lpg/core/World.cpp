//
// Created by volt on 12/21/2024.
//

#include "World.hpp"
#include "entity.hpp"
#include "SysCounter.hpp"
#include "message.hpp"

#include <vector>
#include <list>
#include <optional>
#include <unordered_map>
#include <bit>
#include <set>

//TODO better separation of concerns and general cleanup across this entire file

namespace lpg {

    namespace detail {

        static constexpr inline unsigned EntityPageSize = 256;
        static constexpr inline unsigned LogEntityPageSize = 8;
        static_assert(EntityPageSize == (1 << LogEntityPageSize));

        inline auto DecomposeEntityDescriptor(EntityDescriptor ed) {
            struct Result {
                uint32_t page;
                uint32_t offset;
            };
            return Result {
                .page = ed / EntityPageSize,
                .offset = ed % EntityPageSize,
            };
        }

        struct EntityPage {

            int32_t entityTypeId;
            int32_t pageId;
            int32_t parentPage;
            std::vector<int32_t> componentPageOffsets;
            int32_t stride;
            int32_t currentSize;

            int32_t numOccupied = 0;

            std::array<uint64_t, (EntityPageSize+63)/64> occupancy;

            std::vector<std::byte> storage;

            [[nodiscard]] std::optional<int> findFreeOffset() const {
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
                return PageReserveEntityResult {
                    .entity = entityPtr(off),
                    .offset = off
                };
            }

            bool isEmpty() const {
                for (int i=0; i<occupancy.size(); i++) {
                    if (occupancy[i] != 0) {
                        return false;
                    }
                }
                return true;
            }

            bool isFull() const {
                for (int i=0; i<occupancy.size(); i++) {
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
                return occupancy[offset/64] & (1 << (offset % 64));
            }

            void* entityPtr(int offset) {
                return storage.data() + offset*stride;
            }

            [[nodiscard]] int numActiveEntities() const {
                int result = 0;
                for (int i=0; i<occupancy.size(); i++) {
                    result += std::popcount(occupancy[i]);
                }
                return result;
            }

            [[nodiscard]] std::vector<std::pair<int, int>> getActiveRanges() const {
                std::vector<std::pair<int, int>> result;
                result.reserve(8);
                int curBegin = 0, curEnd = 0;
                for (int i=0; i<occupancy.size(); i++) {
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

                        begin += 64*i;
                        end += 64*i;

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


        struct WorldImpl {

            std::vector<EntityInterface> entityInterfaces_;
            std::unordered_map<std::string, int> entityTypeMap_;

            std::vector<EntityPage> entityPages_;
            std::vector<std::vector<int>> entityPagesByType_;
            std::vector<std::set<int>> freePagesByType_;

            std::vector<EntityDescriptor> arrIdToDescriptor_;
            std::vector<EntityID> arrDescriptorToId_;
            std::vector<EntityVersionNumber> arrIdToVersion_;

            std::unordered_map<std::string, int32_t> hmEntNameToId_;

            ~WorldImpl() {

                //Destroy all entities stored by each EntityPage.
                //Consider during cleanup: in principle, EntityPage should be responsible for this.
                for (auto& page: entityPages_) {
                    auto& interface = entityInterfaces_.at(page.entityTypeId);
                    for (auto [beg, end]: page.getActiveRanges()) {
                        for (int i=beg; i<end; i++) {
                            interface.destroy(page.entityPtr(i));
                        }
                    }
                }
            }


            int registerEntityType(const std::string& name, const EntityInterface& entityInterface) {
                if (entityTypeMap_.contains(name)) {
                    throw std::runtime_error("Entity type already registered: " + name);
                }
                auto newEntTypeId = static_cast<int>(entityInterfaces_.size());
                entityInterfaces_.push_back(entityInterface);
                entityTypeMap_[name] = newEntTypeId;
                return newEntTypeId;
            }

            void assertPagesCompatible(const EntityPage& page1, const EntityPage& page2) {
                if (page1.entityTypeId != page2.entityTypeId) {
                    throw std::runtime_error("Pages are not compatible");
                }
            }

            void relocateEntity(int32_t targetDescriptor, int32_t sourceDescriptor) {
                auto [dstPageId, dstOff] = DecomposeEntityDescriptor(targetDescriptor);
                auto [srcPageId, srcOff] = DecomposeEntityDescriptor(sourceDescriptor);

                auto& dstPage = entityPages_.at(dstPageId);
                auto& srcPage = entityPages_.at(srcPageId);

                assertPagesCompatible(srcPage, dstPage);

                if (dstPage.isEntityPresent(dstOff)) {
                    throw std::runtime_error("invalid relocation");
                }

                const auto& interface = entityInterfaces_.at(srcPage.entityTypeId);

                void* dst = dstPage.entityPtr(dstOff);
                void* src = srcPage.entityPtr(srcOff);

                interface.move(dst, src);
                interface.destroy(src);
            }

            void swapEntities(int32_t targetDescriptor, int32_t sourceDescriptor) {
                auto [dstPageId, dstOff] = DecomposeEntityDescriptor(targetDescriptor);
                auto [srcPageId, srcOff] = DecomposeEntityDescriptor(sourceDescriptor);

                auto& dstPage = entityPages_.at(dstPageId);
                auto& srcPage = entityPages_.at(srcPageId);

                assertPagesCompatible(srcPage, dstPage);

                if (not srcPage.isEntityPresent(srcOff) || not dstPage.isEntityPresent(dstOff)) {
                    throw std::runtime_error("cannot swap with empty");
                }

                const auto& interface = entityInterfaces_.at(srcPage.entityTypeId);

                void* dst = dstPage.entityPtr(dstOff);
                void* src = srcPage.entityPtr(srcOff);

                interface.swap(dst, src);
            }

            int32_t createNewPage(int entityTypeId) {
                const auto& interface = entityInterfaces_.at(entityTypeId);
                int32_t newPageId = entityPages_.size();

                if (entityPagesByType_.size() < entityTypeId+1) {
                    entityPagesByType_.resize(entityTypeId+1);
                }
                entityPagesByType_[entityTypeId].push_back(newPageId);

                if (freePagesByType_.size() < entityTypeId+1) {
                    freePagesByType_.resize(entityTypeId+1);
                }
                freePagesByType_[entityTypeId].insert(newPageId);
                //TODO create pages for managed components

                entityPages_.push_back(EntityPage {
                    .occupancy = {},
                    .stride = interface.entitySize,
                    .storage = {},
                    .currentSize = 0,
                    .numOccupied = 0,
                    .pageId = newPageId,
                    .parentPage = -1,
                    .componentPageOffsets = {},
                    .entityTypeId = entityTypeId
                });
            }

            int32_t getFreePage(int entityTypeId) {
                auto& pageSet = freePagesByType_.at(entityTypeId);
                if (not pageSet.empty()) {
                    auto it = pageSet.begin();
                    auto result = *it;
                    pageSet.erase(it);
                    return result;
                } else {
                    return createNewPage(entityTypeId);
                }
            }

            ReserveEntityResult reserveEntity(int entityTypeId) {
                auto& page = entityPages_.at(getFreePage(entityTypeId));
                auto reserveResult = page.reserveEntity().value();
                if (page.isFull()) {
                    freePagesByType_.at(entityTypeId).erase(page.pageId);
                }
                return ReserveEntityResult {
                    .entity = page.entityPtr(reserveResult.offset),
                    .descriptor = static_cast<int32_t>(page.pageId * EntityPageSize + reserveResult.offset)
                };
            }

            bool despawnEntity(int entityDescriptor) {
                auto [pageNum, offset] = DecomposeEntityDescriptor(entityDescriptor);
                auto& page = entityPages_.at(pageNum);
                auto& entInterface = entityInterfaces_.at(pageNum);
                if (page.isEntityPresent(offset)) {
                    void* p = page.entityPtr(offset);
                    //TODO send PreKillMessage here
                    entInterface.destroy(p);
                    page.releaseEntity(offset);
                    if (page.numActiveEntities() == EntityPageSize - 1) {
                        freePagesByType_.at(page.entityTypeId).insert(page.pageId);
                    }
                    return true;
                }
                return false;
            }


        };

    }




} // lpg