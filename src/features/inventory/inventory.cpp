#include <algorithm>
#include <utility>

#include "inventory.h"
#include "itemgenerator.h"

#include "gameitems/lookup.h"
#include "inventory/item.h"
#include "inventory/structs.h"

#include "../../utils/memory.h"

#include "../../../lib/sdk/EconItemView.h"

using Inventory::InvalidDynamicDataIdx;
using Inventory::BASE_ITEMID;

static std::vector<inventory::Skin> dynamicSkinData;
static std::vector<inventory::Glove> dynamicGloveData;
static std::vector<inventory::Agent> dynamicAgentData;
static std::vector<inventory::Music> dynamicMusicData;
static std::vector<inventory::SouvenirPackage> dynamicSouvenirPackageData;
static std::vector<inventory::ServiceMedal> dynamicServiceMedalData;
static std::vector<inventory::TournamentCoin> dynamicTournamentCoinData;
static std::vector<inventory::Graffiti> dynamicGraffitiData;

class InventoryImpl {
public:
    struct ToEquip {
        ToEquip(Team team, int slot, std::size_t index) : team{ team }, slot{ slot }, index{ index } {}

        Team team;
        int slot;
        std::size_t index;
    };

    static std::vector<inventory::Item>& get() noexcept
    {
        return instance().inventory;
    }

    static void addItem(const game_items::Item& gameItem, std::size_t dynamicDataIdx, bool asUnacknowledged) noexcept
    {
        instance().toAdd.emplace_back(gameItem, dynamicDataIdx, asUnacknowledged);
    }

    static std::uint64_t addItemNow(const game_items::Item& gameItem, std::size_t dynamicDataIdx, bool asUnacknowledged) noexcept
    {
        return instance()._addItem(gameItem, dynamicDataIdx, asUnacknowledged);
    }

    static void deleteItemNow(std::uint64_t itemID) noexcept
    {
        instance()._deleteItem(itemID);
    }

    static void runFrame() noexcept
    {
        instance()._runFrame();
    }

    static inventory::Item* getItem(std::uint64_t itemID) noexcept
    {
        return instance()._getItem(itemID);
    }

    static std::uint64_t recreateItem(std::uint64_t itemID) noexcept
    {
        return instance()._recreateItem(itemID);
    }

    static void clear() noexcept
    {
        instance()._clear();
    }

    static void equipItem(Team team, int slot, std::size_t index) noexcept
    {
        instance().toEquip.emplace_back(team, slot, index);
    }

    static std::size_t getItemIndex(std::uint64_t itemID) noexcept
    {
        return instance()._getItemIndex(itemID);
    }
private:
    inventory::Item* _getItem(std::uint64_t itemID) noexcept
    {
        if (itemID >= BASE_ITEMID && static_cast<std::size_t>(itemID - BASE_ITEMID) < inventory.size())
            return &inventory[static_cast<std::size_t>(itemID - BASE_ITEMID)];
        return nullptr;
    }

    std::size_t _getItemIndex(std::uint64_t itemID) noexcept
    {
        assert(_getItem(itemID) != nullptr);
        return static_cast<std::size_t>(itemID - BASE_ITEMID - std::count_if(inventory.begin(), inventory.begin() + static_cast<std::size_t>(itemID - BASE_ITEMID), [](const auto& item) { return item.isDeleted(); }));
    }

    static void initSkinEconItem(const inventory::Item& inventoryItem, EconItem& econItem) noexcept
    {
        assert(inventoryItem.isSkin());

        EconItemAttributeSetter attributeSetter{ *memory->itemSystem()->getItemSchema() };

        const auto paintKit = StaticData::lookup().getStorage().getPaintKit(inventoryItem.get()).id;
        attributeSetter.setPaintKit(econItem, static_cast<float>(paintKit));

        const auto& dynamicData = dynamicSkinData[inventoryItem.getDynamicDataIndex()];
        const auto isMP5LabRats = Helpers::isMP5LabRats(inventoryItem.get().getWeaponID(), paintKit);
        if (dynamicData.isSouvenir() || isMP5LabRats) {
            econItem.quality = 12;
        } else {
            if (dynamicData.statTrak > -1) {
                attributeSetter.setStatTrak(econItem, dynamicData.statTrak);
                attributeSetter.setStatTrakType(econItem, 0);
                econItem.quality = 9;
            }
            if (Helpers::isKnife(econItem.weaponId))
                econItem.quality = 3;
        }

        if (isMP5LabRats) {
            attributeSetter.setSpecialEventID(econItem, 1);
        } else {
            if (dynamicData.tournamentID != 0)
                attributeSetter.setTournamentID(econItem, dynamicData.tournamentID);

            if (dynamicData.tournamentStage != TournamentStage{ 0 }) {
                attributeSetter.setTournamentStage(econItem, static_cast<int>(dynamicData.tournamentStage));
                attributeSetter.setTournamentTeam1(econItem, static_cast<int>(dynamicData.tournamentTeam1));
                attributeSetter.setTournamentTeam2(econItem, static_cast<int>(dynamicData.tournamentTeam2));
                if (dynamicData.proPlayer != static_cast<ProPlayer>(0))
                    attributeSetter.setTournamentPlayer(econItem, static_cast<int>(dynamicData.proPlayer));
            }
        }

        attributeSetter.setWear(econItem, dynamicData.wear);
        attributeSetter.setSeed(econItem, static_cast<float>(dynamicData.seed));
        memory->setCustomName(&econItem, dynamicData.nameTag.c_str());

        for (std::size_t j = 0; j < dynamicData.stickers.size(); ++j) {
            const auto& sticker = dynamicData.stickers[j];
            if (sticker.stickerID == 0)
                continue;

            attributeSetter.setStickerID(econItem, j, sticker.stickerID);
            attributeSetter.setStickerWear(econItem, j, sticker.wear);
        }
    }

    std::uint64_t _createSOCItem(const inventory::Item& inventoryItem, bool asUnacknowledged) const noexcept
    {
        const auto localInventory = memory->inventoryManager->getLocalInventory();
        if (!localInventory)
            return 0;

        const auto baseTypeCache = localInventory->getItemBaseTypeCache();
        if (!baseTypeCache)
            return 0;

        static const auto baseInvID = localInventory->getHighestIDs().second;

        const auto econItem = memory->createEconItemSharedObject();
        econItem->itemID = BASE_ITEMID + inventory.size() - 1;
        econItem->originalID = 0;
        econItem->accountID = localInventory->getAccountID();
        econItem->inventory = asUnacknowledged ? 0 : baseInvID + inventory.size();

        const auto& item = inventoryItem.get();
        econItem->rarity = static_cast<std::uint16_t>(item.getRarity());
        econItem->quality = 4;
        econItem->weaponId = item.getWeaponID();

        const auto& storage = StaticData::lookup().getStorage();

        EconItemAttributeSetter attributeSetter{ *memory->itemSystem()->getItemSchema() };

        if (item.isSticker()) {
            attributeSetter.setStickerID(*econItem, 0, storage.getStickerKit(item).id);
        } else if (item.isPatch()) {
            attributeSetter.setStickerID(*econItem, 0, storage.getPatch(item).id);
        } else if (item.isGraffiti()) {
            attributeSetter.setStickerID(*econItem, 0, storage.getGraffitiKit(item).id);
            const auto& dynamicData = dynamicGraffitiData[inventoryItem.getDynamicDataIndex()];
            if (dynamicData.usesLeft >= 0) {
                econItem->weaponId = WeaponId::Graffiti;
                attributeSetter.setSpraysRemaining(*econItem, dynamicData.usesLeft);
            }
        } else if (item.isMusic()) {
            attributeSetter.setMusicID(*econItem, storage.getMusicKit(item).id);
            const auto& dynamicData = dynamicMusicData[inventoryItem.getDynamicDataIndex()];
            if (dynamicData.statTrak > -1) {
                attributeSetter.setStatTrak(*econItem, dynamicData.statTrak);
                attributeSetter.setStatTrakType(*econItem, 1);
                econItem->quality = 9;
            }
        } else if (item.isSkin()) {
            initSkinEconItem(inventoryItem, *econItem);
        } else if (item.isGloves()) {
            econItem->quality = 3;
            attributeSetter.setPaintKit(*econItem, static_cast<float>(storage.getPaintKit(item).id));

            const auto& dynamicData = dynamicGloveData[inventoryItem.getDynamicDataIndex()];
            attributeSetter.setWear(*econItem, dynamicData.wear);
            attributeSetter.setSeed(*econItem, static_cast<float>(dynamicData.seed));
        } else if (item.isCollectible()) {
            if (storage.isCollectibleGenuine(item))
                econItem->quality = 1;
        } else if (item.isAgent()) {
            const auto& dynamicData = dynamicAgentData[inventoryItem.getDynamicDataIndex()];
            for (std::size_t j = 0; j < dynamicData.patches.size(); ++j) {
                const auto& patch = dynamicData.patches[j];
                if (patch.patchID == 0)
                    continue;

                attributeSetter.setStickerID(*econItem, j, patch.patchID);
            }
        } else if (item.isServiceMedal()) {
            if (const auto& dynamicData = dynamicServiceMedalData[inventoryItem.getDynamicDataIndex()]; dynamicData.issueDateTimestamp != 0)
                attributeSetter.setIssueDate(*econItem, dynamicData.issueDateTimestamp);
        } else if (item.isTournamentCoin()) {
            attributeSetter.setDropsAwarded(*econItem, dynamicTournamentCoinData[inventoryItem.getDynamicDataIndex()].dropsAwarded);
            attributeSetter.setDropsRedeemed(*econItem, 0);
        } else if (item.isCase() && StaticData::isSouvenirPackage(item)) {
            if (const auto& dynamicData = dynamicSouvenirPackageData[inventoryItem.getDynamicDataIndex()]; dynamicData.tournamentStage != TournamentStage{ 0 }) {
                attributeSetter.setTournamentStage(*econItem, static_cast<int>(dynamicData.tournamentStage));
                attributeSetter.setTournamentTeam1(*econItem, static_cast<int>(dynamicData.tournamentTeam1));
                attributeSetter.setTournamentTeam2(*econItem, static_cast<int>(dynamicData.tournamentTeam2));
                if (dynamicData.proPlayer != static_cast<ProPlayer>(0))
                    attributeSetter.setTournamentPlayer(*econItem, static_cast<int>(dynamicData.proPlayer));
            }
        }

        baseTypeCache->addObject(econItem);
        localInventory->soCreated(localInventory->getSOID(), (SharedObject*)econItem, 4);

        if (const auto inventoryComponent = *memory->uiComponentInventory) {
            memory->setItemSessionPropertyValue(inventoryComponent, econItem->itemID, "recent", "0");
            memory->setItemSessionPropertyValue(inventoryComponent, econItem->itemID, "updated", "0");
        }

        if (const auto view = memory->findOrCreateEconItemViewForItemID(econItem->itemID))
            view->clearInventoryImageRGBA();

        return econItem->itemID;
    }

    void _deleteItem(std::uint64_t itemID) noexcept
    {
        const auto item = _getItem(itemID);
        if (!item)
            return;

        const auto view = memory->findOrCreateEconItemViewForItemID(itemID);
        if (!view)
            return;

        const auto econItem = memory->getSOCData(view);
        if (!econItem)
            return;

        const auto localInventory = memory->inventoryManager->getLocalInventory();
        if (!localInventory)
            return;

        localInventory->soDestroyed(localInventory->getSOID(), (SharedObject*)econItem, 4);

        if (const auto baseTypeCache = localInventory->getItemBaseTypeCache())
            baseTypeCache->removeObject(econItem);

        econItem->destructor();
        item->markAsDeleted();
    }

    std::uint64_t _addItem(const game_items::Item& gameItem, std::size_t dynamicDataIdx, bool asUnacknowledged) noexcept
    {
        return _createSOCItem(inventory.emplace_back(gameItem, dynamicDataIdx != InvalidDynamicDataIdx ? dynamicDataIdx : ItemGenerator::createDefaultDynamicData(gameItem)), asUnacknowledged);
    }

    std::uint64_t _recreateItem(std::uint64_t itemID) noexcept
    {
        const auto item = _getItem(itemID);
        if (!item)
            return 0;

        auto itemCopy = *item;

        if (const auto localInventory = memory->inventoryManager->getLocalInventory()) {
            if (const auto view = memory->findOrCreateEconItemViewForItemID(itemID)) {
                if (const auto econItem = memory->getSOCData(view)) {
                    if (const auto def = memory->itemSystem()->getItemSchema()->getItemDefinitionInterface(econItem->weaponId)) {
                        if (const auto slotCT = def->getLoadoutSlot(Team::CT); localInventory->getItemInLoadout(Team::CT, slotCT) == view)
                            toEquip.emplace_back(Team::CT, slotCT, inventory.size());
                        if (const auto slotTT = def->getLoadoutSlot(Team::TT); localInventory->getItemInLoadout(Team::TT, slotTT) == view)
                            toEquip.emplace_back(Team::TT, slotTT, inventory.size());
                    }
                }
            }
        }

        _deleteItem(itemID);
        return _createSOCItem(inventory.emplace_back(std::move(itemCopy)), false);
    }

    void _addItems() noexcept
    {
        for (const auto [gameItem, dynamicDataIndex, asUnacknowledged] : toAdd)
            _addItem(gameItem, dynamicDataIndex, asUnacknowledged);
        toAdd.clear();
    }

    void _deleteItems() noexcept
    {
        for (std::size_t i = 0; i < inventory.size(); ++i) {
            if (inventory[i].shouldDelete()) {
                _deleteItem(BASE_ITEMID + i);
                inventory[i].markAsDeleted();
            }
        }
    }

    void _clear() noexcept
    {
        for (std::size_t i = 0; i < inventory.size(); ++i)
            _deleteItem(BASE_ITEMID + i);

        inventory.clear();
        dynamicSkinData.clear();
        dynamicGloveData.clear();
        dynamicAgentData.clear();
        dynamicMusicData.clear();
    }

    void _equipItems() noexcept
    {
        for (const auto& item : toEquip)
            memory->inventoryManager->equipItemInSlot(item.team, item.slot, item.index + BASE_ITEMID);
        toEquip.clear();
    }

    void _runFrame() noexcept
    {
        _deleteItems();
        _addItems();
        _equipItems();
    }

    static InventoryImpl& instance() noexcept
    {
        static InventoryImpl inventory;
        return inventory;
    }

    std::vector<std::tuple<std::reference_wrapper<const game_items::Item>, std::size_t, bool>> toAdd;
    std::vector<ToEquip> toEquip;
    std::vector<inventory::Item> inventory;
};

inventory::Skin& Inventory::dynamicSkinData(const inventory::Item& item) noexcept
{
    assert(item.isSkin());
    return ::dynamicSkinData[item.getDynamicDataIndex()];
}

inventory::Glove& Inventory::dynamicGloveData(const inventory::Item& item) noexcept
{
    assert(item.isGlove());
    return ::dynamicGloveData[item.getDynamicDataIndex()];
}

inventory::Agent& Inventory::dynamicAgentData(const inventory::Item& item) noexcept
{
    assert(item.isAgent());
    return ::dynamicAgentData[item.getDynamicDataIndex()];
}

inventory::Music& Inventory::dynamicMusicData(const inventory::Item& item) noexcept
{
    assert(item.isMusic());
    return ::dynamicMusicData[item.getDynamicDataIndex()];
}

inventory::SouvenirPackage& Inventory::dynamicSouvenirPackageData(const inventory::Item& item) noexcept
{
    assert(item.isCase() && StaticData::isSouvenirPackage(item.get()));
    return ::dynamicSouvenirPackageData[item.getDynamicDataIndex()];
}

inventory::ServiceMedal& Inventory::dynamicServiceMedalData(const inventory::Item& item) noexcept
{
    assert(item.isServiceMedal());
    return ::dynamicServiceMedalData[item.getDynamicDataIndex()];
}

inventory::TournamentCoin& Inventory::dynamicTournamentCoinData(const inventory::Item& item) noexcept
{
    assert(item.isTournamentCoin());
    return ::dynamicTournamentCoinData[item.getDynamicDataIndex()];
}

inventory::Graffiti& Inventory::dynamicGraffitiData(const inventory::Item& item) noexcept
{
    assert(item.isGraffiti());
    return ::dynamicGraffitiData[item.getDynamicDataIndex()];
}

std::size_t Inventory::emplaceDynamicData(inventory::Skin&& data) noexcept
{
    ::dynamicSkinData.push_back(std::move(data));
    return ::dynamicSkinData.size() - 1;
}

std::size_t Inventory::emplaceDynamicData(inventory::Glove&& data) noexcept
{
    ::dynamicGloveData.push_back(std::move(data));
    return ::dynamicGloveData.size() - 1;
}

std::size_t Inventory::emplaceDynamicData(inventory::Agent&& data) noexcept
{
    ::dynamicAgentData.push_back(std::move(data));
    return ::dynamicAgentData.size() - 1;
}

std::size_t Inventory::emplaceDynamicData(inventory::Music&& data) noexcept
{
    ::dynamicMusicData.push_back(std::move(data));
    return ::dynamicMusicData.size() - 1;
}

std::size_t Inventory::emplaceDynamicData(inventory::SouvenirPackage&& data) noexcept
{
    ::dynamicSouvenirPackageData.push_back(std::move(data));
    return ::dynamicSouvenirPackageData.size() - 1;
}

std::size_t Inventory::emplaceDynamicData(inventory::ServiceMedal&& data) noexcept
{
    ::dynamicServiceMedalData.push_back(std::move(data));
    return ::dynamicServiceMedalData.size() - 1;
}

std::size_t Inventory::emplaceDynamicData(inventory::TournamentCoin&& data) noexcept
{
    ::dynamicTournamentCoinData.push_back(std::move(data));
    return ::dynamicTournamentCoinData.size() - 1;
}

std::size_t Inventory::emplaceDynamicData(inventory::Graffiti&& data) noexcept
{
    ::dynamicGraffitiData.push_back(std::move(data));
    return ::dynamicGraffitiData.size() - 1;
}

std::vector<inventory::Item>& Inventory::get() noexcept
{
    return InventoryImpl::get();
}

void Inventory::addItemUnacknowledged(const game_items::Item& gameItem, std::size_t dynamicDataIdx) noexcept
{
    InventoryImpl::addItem(gameItem, dynamicDataIdx, true);
}

void Inventory::addItemAcknowledged(const game_items::Item& gameItem, std::size_t dynamicDataIdx) noexcept
{
    InventoryImpl::addItem(gameItem, dynamicDataIdx, false);
}

std::uint64_t Inventory::addItemNow(const game_items::Item& gameItem, std::size_t dynamicDataIdx, bool asUnacknowledged) noexcept
{
    return InventoryImpl::addItemNow(gameItem, dynamicDataIdx, asUnacknowledged);
}

void Inventory::deleteItemNow(std::uint64_t itemID) noexcept
{
    InventoryImpl::deleteItemNow(itemID);
}

void Inventory::runFrame() noexcept
{
    InventoryImpl::runFrame();
}

inventory::Item* Inventory::getItem(std::uint64_t itemID) noexcept
{
    return InventoryImpl::getItem(itemID);
}

std::uint64_t Inventory::recreateItem(std::uint64_t itemID) noexcept
{
    return InventoryImpl::recreateItem(itemID);
}

void Inventory::clear() noexcept
{
    InventoryImpl::clear();
}

void Inventory::equipItem(Team team, int slot, std::size_t index) noexcept
{
    InventoryImpl::equipItem(team, slot, index);
}

std::size_t Inventory::getItemIndex(std::uint64_t itemID) noexcept
{
    return InventoryImpl::getItemIndex(itemID);
}
