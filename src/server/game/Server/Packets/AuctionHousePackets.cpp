/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "AuctionHousePackets.h"
#include "AuctionHouseMgr.h"
#include "ObjectGuid.h"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::AuctionHouse::AuctionItem const& auctionItem)
{
    data << auctionItem.Item;
    data << int32(auctionItem.Count);
    data << int32(auctionItem.Charges);
    data << auctionItem.Flags;
    data << int32(auctionItem.AuctionItemID);
    data << auctionItem.Owner;
    data << uint64(auctionItem.MinBid);
    data << uint64(auctionItem.MinIncrement);
    data << uint64(auctionItem.BuyoutPrice);
    data << int32(auctionItem.DurationLeft);
    data << uint8(auctionItem.DeleteReason);
    data.WriteBits(auctionItem.Enchantments.size(), 4);
    data.WriteBits(auctionItem.Gems.size(), 2);
    data.WriteBit(auctionItem.CensorServerSideInfo);
    data.WriteBit(auctionItem.CensorBidInfo);
    data.FlushBits();

    for (auto const& gem : auctionItem.Gems)
        data << gem;

    for (auto const& enchant : auctionItem.Enchantments)
        data << enchant;

    if (!auctionItem.CensorServerSideInfo)
    {
        data << auctionItem.ItemGuid;
        data << auctionItem.OwnerAccountID;
        data << auctionItem.EndTime;
    }

    if (!auctionItem.CensorBidInfo)
    {
        data << auctionItem.Bidder;
        data << auctionItem.BidAmount;
    }

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::AuctionHouse::AuctionOwnerNotification const& ownerNotification)
{
    data << int32(ownerNotification.AuctionItemID);
    data << uint64(ownerNotification.BidAmount);
    data << ownerNotification.Item;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::AuctionHouse::AuctionBidderNotification const& bidderNotification)
{
    data << int32(bidderNotification.AuctionItemID);
    data << bidderNotification.Bidder;
    data << bidderNotification.Item;

    return data;
}

void WorldPackets::AuctionHouse::AuctionOwnerNotification::Initialize(AuctionEntry const* auction, ::Item const* item)
{
    AuctionItemID = auction->Id;
    Item.Initialize(item);
    BidAmount = auction->bid;
}

void WorldPackets::AuctionHouse::AuctionBidderNotification::Initialize(AuctionEntry const* auction, ::Item const* item)
{
    AuctionItemID = auction->Id;
    Item.Initialize(item);
    Bidder = auction->Bidder;
}

void WorldPackets::AuctionHouse::AuctionHelloRequest::Read()
{
    _worldPacket >> Guid;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionHelloResponse::Write()
{
    _worldPacket << Guid;
    _worldPacket.WriteBit(OpenForBusiness);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::AuctionHouse::AuctionCommandResult::InitializeAuction(AuctionEntry* auction)
{
    if (!auction)
        return;

    AuctionItemID = auction->Id;
    Money = auction->bid == auction->buyout ? 0 : auction->bid;
    MinIncrement = auction->bid == auction->buyout ? 0 : auction->GetAuctionOutBid();
    Guid = auction->Bidder;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionCommandResult::Write()
{
    _worldPacket << uint32(AuctionItemID);
    _worldPacket << int32(Command);
    _worldPacket << int32(ErrorCode);
    _worldPacket << int32(BagResult);
    _worldPacket << Guid;
    _worldPacket << uint64(MinIncrement);
    _worldPacket << uint64(Money);

    return &_worldPacket;
}

void WorldPackets::AuctionHouse::AuctionSellItem::Read()
{
    _worldPacket >> Auctioneer;
    _worldPacket >> MinBid;
    _worldPacket >> BuyoutPrice;
    _worldPacket >> RunTime;

    Items.resize(_worldPacket.ReadBits(5));
    _worldPacket.ResetBitPos();

    for (auto& item : Items)
    {
        _worldPacket >> item.Guid;
        _worldPacket >> item.UseCount;
    }
}

void WorldPackets::AuctionHouse::AuctionPlaceBid::Read()
{
    _worldPacket >> Auctioneer;
    _worldPacket >> AuctionItemID;
    _worldPacket >> BidAmount;
}

void WorldPackets::AuctionHouse::AuctionListBidderItems::Read()
{
    _worldPacket >> Auctioneer;
    _worldPacket >> Offset;
    uint8 auctionItemIDsCount = _worldPacket.ReadBits(7);
    _worldPacket.ResetBitPos();

    for (uint8 i = 0; i < auctionItemIDsCount; i++)
    {
        uint32 AuctionItemID = 0;
        _worldPacket >> AuctionItemID;
        AuctionItemIDs.emplace_back(AuctionItemID);
    }
}

void WorldPackets::AuctionHouse::AuctionRemoveItem::Read()
{
    _worldPacket >> Auctioneer;
    _worldPacket >> AuctionItemID;
}

void WorldPackets::AuctionHouse::AuctionReplicateItems::Read()
{
    _worldPacket >> Auctioneer;
    _worldPacket >> ChangeNumberGlobal;
    _worldPacket >> ChangeNumberCursor;
    _worldPacket >> ChangeNumberTombstone;
    _worldPacket >> Count;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionListItemsResult::Write()
{
    _worldPacket << static_cast<int32>(Items.size());
    _worldPacket << int32(TotalCount);
    _worldPacket << int32(DesiredDelay);
    _worldPacket.WriteBit(OnlyUsable);
    _worldPacket.FlushBits();

    for (AuctionItem const& item : Items)
        _worldPacket << item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionListOwnerItemsResult::Write()
{
    _worldPacket << static_cast<int32>(Items.size());
    _worldPacket << uint32(TotalCount);
    _worldPacket << uint32(DesiredDelay);

    for (auto const& item : Items)
        _worldPacket << item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionListBidderItemsResult::Write()
{
    _worldPacket << static_cast<int32>(Items.size());
    _worldPacket << uint32(TotalCount);
    _worldPacket << uint32(DesiredDelay);

    for (auto const& item : Items)
        _worldPacket << item;

    return &_worldPacket;
}

void WorldPackets::AuctionHouse::AuctionListItems::Read()
{
    _worldPacket >> Offset;
    _worldPacket >> Auctioneer;
    _worldPacket >> MinLevel;
    _worldPacket >> MaxLevel;
    _worldPacket >> Quality;
    _worldPacket >> SortCount;
    KnownPets.resize(_worldPacket.read<uint32>());
    _worldPacket >> MaxPetLevel;
    for (auto &v : KnownPets)
        _worldPacket >> v;

    Name = _worldPacket.ReadString(_worldPacket.ReadBits(8));
    ClassFilters.resize(_worldPacket.ReadBits(3));
    OnlyUsable = _worldPacket.ReadBit();
    ExactMatch = _worldPacket.ReadBit();

    for (ClassFilter& classFilter : ClassFilters)
    {
        _worldPacket >> classFilter.ItemClass;
        classFilter.SubClassFilters.resize(_worldPacket.ReadBits(5));
        for (ClassFilter::SubClassFilter& subclassFilter : classFilter.SubClassFilters)
        {
            _worldPacket >> subclassFilter.ItemSubclass;
            _worldPacket >> subclassFilter.InvTypeMask;
        }
    }

    /*auto DataSize =  */_worldPacket.read<uint32>(); // DataSize = (SortCount * 2)
    for (int32 i = 0; i < SortCount; i++)
    {
        Sort sort;
        _worldPacket >> sort.Type;
        _worldPacket >> sort.Direction;
        DataSort.push_back(sort);
    }
}

void WorldPackets::AuctionHouse::AuctionListOwnerItems::Read()
{
    _worldPacket >> Auctioneer;
    _worldPacket >> Offset;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionListPendingSalesResult::Write()
{
    _worldPacket << static_cast<int32>(Mails.size());
    _worldPacket << int32(TotalNumRecords);

    for (auto const& mail : Mails)
        _worldPacket << mail;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionClosedNotification::Write()
{
    _worldPacket << Info;
    _worldPacket << float(ProceedsMailDelay);
    _worldPacket.WriteBit(Sold);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionOwnerBidNotification::Write()
{
    _worldPacket << Info;
    _worldPacket << uint64(MinIncrement);
    _worldPacket << Bidder;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionWonNotification::Write()
{
    _worldPacket << Info;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionOutBidNotification::Write()
{
    _worldPacket << Info;
    _worldPacket << uint64(BidAmount);
    _worldPacket << uint64(MinIncrement);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::AuctionHouse::AuctionReplicateResponse::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << uint32(DesiredDelay);
    _worldPacket << uint32(ChangeNumberGlobal);
    _worldPacket << uint32(ChangeNumberCursor);
    _worldPacket << uint32(ChangeNumberTombstone);
    _worldPacket << static_cast<int32>(Items.size());

    for (auto const& item : Items)
        _worldPacket << item;

    return &_worldPacket;
}
