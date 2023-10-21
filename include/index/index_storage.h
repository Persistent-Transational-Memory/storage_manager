/*
 * @author: BL-GS 
 * @date:   2023/4/23
 */

#pragma once
#ifndef PTM_INDEX_STORAGE_H
#define PTM_INDEX_STORAGE_H

#include <memory_resource>
#include "mem_allocator/mem_allocator.h"

namespace ix {

	constexpr char INDEX_DRAM_DIR_NAME[] = "/dev/shm";
	constexpr char INDEX_PMEM_DIR_NAME[] = "/mnt/pmem0";
	constexpr char INDEX_FILE_NAME[] = "index";

	enum class IndexStorageKind {
		DRAM,
		PMEM,
		Hybrid
	};

	template<IndexStorageKind kind = IndexStorageKind::DRAM>
	class IndexStorage {
	private:
		allocator::DRAMPoolAllocator allocator_;

	public:
		IndexStorage(uint32_t tuple_size, uint32_t expected_amount):
			allocator_(INDEX_DRAM_DIR_NAME, INDEX_FILE_NAME, tuple_size, expected_amount * 2) {}

	public:
		void *allocate(size_t size) {
			return allocator_.allocate(size);
		}

		void deallocate(void *ptr) {
			allocator_.deallocate(ptr, 0);
		}

		void back_up() { }
	};

	template<>
	class IndexStorage<IndexStorageKind::PMEM> {
	private:
		allocator::PMDKAllocator allocator_;

	public:
		IndexStorage(uint32_t tuple_size, uint32_t expected_amount):
				allocator_(INDEX_PMEM_DIR_NAME, INDEX_FILE_NAME, tuple_size, expected_amount * 2) {}

	public:
		void *allocate(size_t size) {
			return allocator_.allocate(size);
		}

		void deallocate(void *ptr) {
			allocator_.deallocate(ptr, 0);
		}

		void back_up() { }
	};

	template<>
	class IndexStorage<IndexStorageKind::Hybrid> {
	private:
		allocator::DRAMPoolAllocator dram_storage_;
		FileDescriptor pmem_descriptor_;

	public:
		IndexStorage(uint32_t tuple_size, uint32_t expected_amount):
				dram_storage_(INDEX_DRAM_DIR_NAME, INDEX_FILE_NAME, tuple_size, expected_amount * 2),
				pmem_descriptor_(INDEX_PMEM_DIR_NAME, INDEX_FILE_NAME, tuple_size * expected_amount * 2) {}

	public:
		void *allocate(size_t size) {
			return dram_storage_.allocate(size);
		}

		void deallocate(void *ptr) {
			dram_storage_.deallocate(ptr, 0);
		}

		void back_up() {
			FileDescriptor &dram_descriptor = dram_storage_.get_descriptor();
			void *dram_start_ptr = dram_descriptor.aligned_start_ptr;
			uint64_t dram_size   = dram_descriptor.aligned_total_size;
			void *pmem_start_ptr = pmem_descriptor_.aligned_start_ptr;



			std::memcpy(std::assume_aligned<FileDescriptor::ALIGN_SIZE>(pmem_start_ptr),
			            std::assume_aligned<FileDescriptor::ALIGN_SIZE>(dram_start_ptr),
						dram_size);
		}
	};

}

#endif //PTM_INDEX_STORAGE_H
