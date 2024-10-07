/*****************************************************************
 * @file   ObjectAllocator.cpp
 * @brief  The implementation file for the ObjectAllocator class.
 * @author david.hedner@digipen.edu
 * @date   January 2024
 * 
 * @copyright © 2024 DigiPen (USA) Corporation.
 *****************************************************************/
#include "ObjectAllocator.h"
#include <cmath>
#include <malloc.h>
#include <cstdlib>
#include <cstring>

static size_t ComputeAlignmentSize(size_t objectSize, size_t alignmentSize)
{
	if (alignmentSize == 0)
	{
		return objectSize;
	}

	return static_cast<size_t>(ceil(static_cast<double>(objectSize) / static_cast<double>(alignmentSize))) * alignmentSize;
}

static size_t MaximumValue(size_t value1, size_t value2)
{
	return value1 > value2 ? value1 : value2;
}

ObjectAllocator::ObjectAllocator(size_t ObjectSize, const OAConfig& config) : Config_(config), ObjectSize_(ObjectSize), AllocatedBlockCount_(0)
{
	Stats_.ObjectSize_ = ObjectSize;

	size_t requestedHeaderSize = sizeof(ListNode);
	size_t requestedDataSize = MaximumValue(ObjectSize_ + Config_.PadBytes_ * 2 + GetBlockHeaderSize(), sizeof(void*));
	PageHeaderSize_ = ComputeAlignmentSize(requestedHeaderSize, Config_.Alignment_);
	ActualDataSize_ = ComputeAlignmentSize(requestedDataSize, Config_.Alignment_);
	Config_.LeftAlignSize_ = static_cast<unsigned int>(PageHeaderSize_ - requestedHeaderSize);
	size_t totalDataSize = ActualDataSize_ * (Config_.ObjectsPerPage_ - 1);
	Config_.InterAlignSize_ = static_cast<unsigned int>(ActualDataSize_ - requestedDataSize);
	size_t size = PageHeaderSize_ + totalDataSize + requestedDataSize;

	Stats_.PageSize_ = size;

	AllocateNewPage();
}

ObjectAllocator::~ObjectAllocator()
{
	for (unsigned int i = 0; i < Stats_.PagesInUse_; i++)
	{
		void* address = PageList_.PopBack();
		char* cursor = reinterpret_cast<char*>(address);

		if (Config_.HBlockInfo_.type_ == OAConfig::HBLOCK_TYPE::hbExternal)
		{
			cursor += PageHeaderSize_;

			for (unsigned int i = 0; i < Config_.ObjectsPerPage_; i++)
			{
				MemBlockInfo** header = reinterpret_cast<MemBlockInfo**>(cursor);
				delete[](*header)->label;
				delete* header;

				cursor += ActualDataSize_;
			}
		}
		delete[] reinterpret_cast<char*>(address);
	}
}

void* ObjectAllocator::Allocate(const char* label)
{
	if (Config_.UseCPPMemManager_)
	{
		Stats_.Allocations_++;
		Stats_.FreeObjects_--;
		Stats_.ObjectsInUse_++;

		if (Stats_.MostObjects_ < Stats_.ObjectsInUse_)
		{
			Stats_.MostObjects_ = Stats_.ObjectsInUse_;
		}

		return new char[Stats_.ObjectSize_];
	}

	if (Stats_.FreeObjects_ == 0)
	{
		if (Config_.MaxPages_ <= Stats_.PagesInUse_)
		{
			throw OAException(
				OAException::E_NO_PAGES, "Allocate: Reached maximum allowed pages");
		}

		AllocateNewPage();
	}

	Stats_.Allocations_++;
	Stats_.FreeObjects_--;
	Stats_.ObjectsInUse_++;

	if (Stats_.MostObjects_ < Stats_.ObjectsInUse_)
	{
		Stats_.MostObjects_ = Stats_.ObjectsInUse_;
	}

	void* data = FreeList_.PopBack();

	memset(data, ALLOCATED_PATTERN, ObjectSize_);

	// Get the header of the data to return
	char* header = reinterpret_cast<char*>(data);
	header -= Config_.PadBytes_ + GetBlockHeaderSize();
	switch (Config_.HBlockInfo_.type_) {
	case OAConfig::HBLOCK_TYPE::hbBasic:
	{
		BasicBlockHeader* dataHeader = reinterpret_cast<BasicBlockHeader*>(header);
		dataHeader->allocationNumber_ = ++AllocatedBlockCount_;
		dataHeader->SetUsed(true);
		break;
	}
	case OAConfig::HBLOCK_TYPE::hbExtended:
	{
		ExtendedBlockHeader* dataHeader = reinterpret_cast<ExtendedBlockHeader*>(header);
		dataHeader->allocationNumber_ = ++AllocatedBlockCount_;
		dataHeader->reuseCount_++;
		dataHeader->SetUsed(true);
		break;
	}
	case OAConfig::HBLOCK_TYPE::hbExternal:
	{
		MemBlockInfo** dataHeader = reinterpret_cast<MemBlockInfo**>(header);
		(*dataHeader)->alloc_num = ++AllocatedBlockCount_;
		(*dataHeader)->in_use = true;
		(*dataHeader)->label = nullptr;

		if (label != nullptr)
		{
			size_t labelLength = strlen(label) + 1;
			(*dataHeader)->label = new char[labelLength];
			memcpy((*dataHeader)->label, label, labelLength);
		}

		break;
	}
	case OAConfig::HBLOCK_TYPE::hbNone:
	{
		break;
	}
	default:
		break;
	}

	return data;
}

void ObjectAllocator::Free(void* Object)
{
	Stats_.Deallocations_++;

	// Use the C++ memory manager if it is enabled
	if (Config_.UseCPPMemManager_)
	{
		delete[] reinterpret_cast<char*>(Object);

		return;
	}

	ListNode* currentPage = PageList_.GetTailNode();
	size_t objectAddress = reinterpret_cast<size_t>(Object);
	while (currentPage != nullptr)
	{
		size_t currentPageAddress = reinterpret_cast<size_t>(currentPage);
		if (objectAddress > currentPageAddress &&
			objectAddress < currentPageAddress + Stats_.PageSize_ &&
			(objectAddress - currentPageAddress - PageHeaderSize_ - Config_.PadBytes_ - GetBlockHeaderSize()) % ActualDataSize_ != 0)
		{
			throw OAException(
				OAException::E_BAD_BOUNDARY, "Free: Bad boundary");
		}

		currentPage = currentPage->next;
	}


	bool wasInUse = false;

	unsigned char* header = reinterpret_cast<unsigned char*>(Object);
	header -= Config_.PadBytes_ + GetBlockHeaderSize();
	if (!IsValidBlock(header))
	{
		throw OAException(
			OAException::E_CORRUPTED_BLOCK, "Free: Corrupted block");
	}

	// Get the header of the data to return
	switch (Config_.HBlockInfo_.type_) {
	case OAConfig::HBLOCK_TYPE::hbBasic:
	{
		BasicBlockHeader* dataHeader = reinterpret_cast<BasicBlockHeader*>(header);
		wasInUse = dataHeader->IsUsed();
		dataHeader->SetUsed(false);
		dataHeader->allocationNumber_ = 0;
		break;
	}
	case OAConfig::HBLOCK_TYPE::hbExtended:
	{
		ExtendedBlockHeader* dataHeader = reinterpret_cast<ExtendedBlockHeader*>(header);
		wasInUse = dataHeader->IsUsed();
		dataHeader->SetUsed(false);
		dataHeader->allocationNumber_ = 0;
		dataHeader->reuseCount_ = 0;
		break;
	}
	case OAConfig::HBLOCK_TYPE::hbExternal:
	{
		MemBlockInfo** dataHeader = reinterpret_cast<MemBlockInfo**>(header);
		wasInUse = (*dataHeader)->in_use;
		(*dataHeader)->in_use = false;
		(*dataHeader)->alloc_num = 0;
		delete[](*dataHeader)->label;
		(*dataHeader)->label = nullptr;
		break;
	}
	case OAConfig::HBLOCK_TYPE::hbNone:
	{
		// Check if the object is in the free list
		if (ObjectSize_ <= sizeof(void*))
		{
			wasInUse = true;
			ListNode* freeEntry = FreeList_.GetTailNode();
			while (freeEntry != nullptr)
			{
				if (freeEntry == Object)
				{
					wasInUse = false;
					break;
				}

				freeEntry = freeEntry->next;
			}
		}
		else
		{
			unsigned char* firstDataByte = header + sizeof(void*);
			wasInUse = *firstDataByte != FREED_PATTERN && *firstDataByte != UNALLOCATED_PATTERN;
		}

		break;
	}
	default:
		break;
	}

	if (!wasInUse)
	{
		throw OAException(
			OAException::E_MULTIPLE_FREE, "Free: Block was already freed");
	}

	memset(Object, FREED_PATTERN, ObjectSize_);

	Stats_.ObjectsInUse_--;
	Stats_.FreeObjects_++;
	FreeList_.PushBack(Object);
}

unsigned ObjectAllocator::DumpMemoryInUse(DUMPCALLBACK fn) const
{
	if (Config_.HBlockInfo_.type_ == OAConfig::HBLOCK_TYPE::hbNone)
	{
		return 0;
	}
	unsigned int callbackCount = 0;
	ListNode* currentPage = PageList_.GetTailNode();
	while (currentPage != nullptr)
	{
		char* cursor = reinterpret_cast<char*>(currentPage);
		cursor += PageHeaderSize_;

		for (unsigned int i = 0; i < Config_.ObjectsPerPage_; i++)
		{
			bool trigger = false;
			switch (Config_.HBlockInfo_.type_) {
			case OAConfig::HBLOCK_TYPE::hbBasic:
			{
				BasicBlockHeader* dataHeader = reinterpret_cast<BasicBlockHeader*>(cursor);
				trigger = dataHeader->IsUsed();
				break;
			}
			case OAConfig::HBLOCK_TYPE::hbExtended:
			{
				ExtendedBlockHeader* dataHeader = reinterpret_cast<ExtendedBlockHeader*>(cursor);
				trigger = dataHeader->IsUsed();
				break;
			}
			case OAConfig::HBLOCK_TYPE::hbExternal:
			{
				MemBlockInfo** dataHeader = reinterpret_cast<MemBlockInfo**>(cursor);
				trigger = (*dataHeader)->in_use;
				break;
			}
			case OAConfig::HBLOCK_TYPE::hbNone:
			{
				break;
			}
			default:
				break;
			}

			if (trigger)
			{
				fn(cursor + GetBlockHeaderSize(), ObjectSize_);
				callbackCount++;
			}

			cursor += ActualDataSize_;
		}

		currentPage = currentPage->next;
	}

	return callbackCount;
}

unsigned ObjectAllocator::ValidatePages(VALIDATECALLBACK fn) const
{
	if (Config_.PadBytes_ == 0)
	{
		return 0;
	}

	unsigned int callbackCount = 0;
	ListNode* currentPage = PageList_.GetTailNode();
	while (currentPage != nullptr)
	{
		// Reading the pad pattern considers unsigned char and char to be different, so it must be casted
		unsigned char* cursor = reinterpret_cast<unsigned char*>(currentPage);
		cursor += PageHeaderSize_;

		for (unsigned int i = 0; i < Config_.ObjectsPerPage_; i++)
		{
			if (!IsValidBlock(cursor))
			{
				fn(cursor + GetBlockHeaderSize() + Config_.PadBytes_, ObjectSize_);
				callbackCount++;
			}

			cursor += ActualDataSize_;
		}

		currentPage = currentPage->next;
	}

	return callbackCount;
}

unsigned ObjectAllocator::FreeEmptyPages()
{
	return 0;
}

bool ObjectAllocator::ImplementedExtraCredit()
{
	return false;
}

void ObjectAllocator::SetDebugState(bool State)
{
	Config_.UseCPPMemManager_ = State;
}

const void* ObjectAllocator::GetFreeList() const
{
	return FreeList_.GetTail();
}

const void* ObjectAllocator::GetPageList() const
{
	return PageList_.GetTail();;
}

OAConfig ObjectAllocator::GetConfig() const
{
	return Config_;
}

OAStats ObjectAllocator::GetStats() const
{
	return Stats_;
}

size_t ObjectAllocator::GetBlockHeaderSize() const
{
	return Config_.HBlockInfo_.size_;
}

char* ObjectAllocator::AllocateNewPage()
{
	if (Config_.UseCPPMemManager_)
	{
		return nullptr;
	}

	Stats_.FreeObjects_ += Config_.ObjectsPerPage_;
	Stats_.PagesInUse_++;

	// Align address for the page to the alignment boundary using _aligned_malloc
	char* page = static_cast<char*>(_aligned_malloc(Stats_.PageSize_, MaximumValue(Config_.Alignment_, 1)));

	if (!page)
	{
		throw OAException(
			OAException::E_NO_MEMORY, "AllocateNewPage: No memory for page allocation");
	}

	// Set the first 8 bytes of the page to null
	char* current = page;
	reinterpret_cast<ListNode*>(current)->next = nullptr;

	// Add to page list at initial address
	PageList_.PushBack(current);

	current += sizeof(ListNode);

	// Set the align pattern in the page header
	memset(current, ALIGN_PATTERN, Config_.LeftAlignSize_);
	current += Config_.LeftAlignSize_;

	// Fill the data blocks
	for (unsigned int i = 0; i < Config_.ObjectsPerPage_; i++)
	{

		switch (Config_.HBlockInfo_.type_) {
		case OAConfig::HBLOCK_TYPE::hbBasic:
		{
			BasicBlockHeader* header = reinterpret_cast<BasicBlockHeader*>(current);
			header->flags_ = 0;
			header->allocationNumber_ = 0;
			current += GetBlockHeaderSize();
			break;
		}
		case OAConfig::HBLOCK_TYPE::hbExtended:
		{
			ExtendedBlockHeader* header = reinterpret_cast<ExtendedBlockHeader*>(current);
			header->flags_ = 0;
			header->allocationNumber_ = 0;
			header->reuseCount_ = 0;
			current += GetBlockHeaderSize() - Config_.HBlockInfo_.additional_;
			memset(current, 0, Config_.HBlockInfo_.additional_);
			current += Config_.HBlockInfo_.additional_;
			break;
		}
		case OAConfig::HBLOCK_TYPE::hbExternal:
		{
			MemBlockInfo* header = new MemBlockInfo();
			header->alloc_num = 0;
			memcpy(current, &header, sizeof(MemBlockInfo*));
			current += GetBlockHeaderSize();
			break;
		}
		case OAConfig::HBLOCK_TYPE::hbNone:
		{
			break;
		}
		default:
			break;
		}

		// Initialize (left) padding
		memset(current, PAD_PATTERN, Config_.PadBytes_);
		current += Config_.PadBytes_;

		// Initialize data
		memset(current, UNALLOCATED_PATTERN, ObjectSize_);

		// Add data to list before initializing it
		FreeList_.PushBack(current);

		current += ObjectSize_;

		// Initialize (right) padding
		memset(current, PAD_PATTERN, Config_.PadBytes_);
		current += Config_.PadBytes_;

		if (i < Config_.ObjectsPerPage_ - 1)
		{
			// Set the align pattern in the data header
			memset(current, ALIGN_PATTERN, Config_.InterAlignSize_);
			current += Config_.InterAlignSize_;
		}
	}

	return page;
}

bool ObjectAllocator::IsValidBlock(unsigned char* cursor) const
{
	cursor += GetBlockHeaderSize();

	for (unsigned int i = 0; i < Config_.PadBytes_; i++)
	{
		if (*cursor != PAD_PATTERN)
		{
			return false;
		}

		cursor++;
	}
	cursor += ObjectSize_;

	for (unsigned int i = 0; i < Config_.PadBytes_; i++)
	{
		if (*cursor != PAD_PATTERN)
		{
			return false;
		}

		cursor++;
	}

	return true;
}

void ObjectAllocator::EmbeddedList::Reinitialize()
{
	_tail = nullptr;
}

void ObjectAllocator::EmbeddedList::PushBack(void* address)
{
	ListNode* newNode = reinterpret_cast<ListNode*>(address);
	if (_tail == nullptr)
	{
		_tail = newNode;
		_tail->next = nullptr;
		return;
	}

	newNode->next = _tail;
	_tail = newNode;
}

void* ObjectAllocator::EmbeddedList::PopBack()
{
	void* temp = _tail;
	_tail = _tail->next;
	return temp;
}

void* ObjectAllocator::EmbeddedList::GetTail() const
{
	return _tail;
}

ObjectAllocator::ListNode* ObjectAllocator::EmbeddedList::GetTailNode() const
{
	return _tail;
}

void ObjectAllocator::BasicBlockHeader::SetUsed(bool used)
{
	if (used)
	{
		flags_ |= 0x01;
	}
	else
	{
		flags_ &= ~0x01;
	}
}

bool ObjectAllocator::BasicBlockHeader::IsUsed() const
{
	return (flags_ & 0x01) != 0;
}

void ObjectAllocator::ExtendedBlockHeader::SetUsed(bool used)
{
	if (used)
	{
		flags_ |= 0x01;
	}
	else
	{
		flags_ &= ~0x01;
	}
}

bool ObjectAllocator::ExtendedBlockHeader::IsUsed() const
{
	return (flags_ & 0x01) != 0;
}
