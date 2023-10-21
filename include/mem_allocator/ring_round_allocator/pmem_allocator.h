/*
 * @author: BL-GS 
 * @date:   2023/3/1
 */

#pragma once
#ifndef PTM_MEM_ALLOCATOR_RING_ROUND_ALLOCATOR_PMEM_ALLOCATOR_H
#define PTM_MEM_ALLOCATOR_RING_ROUND_ALLOCATOR_PMEM_ALLOCATOR_H

#include <cstdint>
#include <iostream>
#include <fstream>
#include <forward_list>
#include <filesystem>
#include <cassert>
#include <queue>
#include <span>

#include <sys/unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <logger/logger.h>
#include <thread/thread.h>
#include <mem_allocator/abstract_mem_allocator.h>

namespace allocator {

	class RindRoundPMEMAllocator {
	public:
		static constexpr size_t ALLOC_ALIGN_SIZE = CACHE_LINE_SIZE;

		inline static const char *PMEM_ROOT_DIR_NAME = GLOBAL_DATA_MEM_DIR_PATH[0];

	private:

		FileDescriptor pmem_descriptor_;

		std::atomic<uint8_t *> cur_bound_;

	public:
		RindRoundPMEMAllocator(size_t tuple_size, size_t expected_amount):
			pmem_descriptor_(PMEM_ROOT_DIR_NAME,
							 allocate_file_name(),
							 std::max(align_size(tuple_size) * expected_amount * 2, 1024UL * 1024 * 1024)),
			 cur_bound_(pmem_descriptor_.aligned_start_ptr) {

			auto &global_logger = util::get_global_logger();
			global_logger.info("PMEM File Path: ", pmem_descriptor_.file_path);
			global_logger.info("PMEM Map Size: ", pmem_descriptor_.total_size);
		}

		~RindRoundPMEMAllocator() = default;

		constexpr static MemAllocatorControlHeader get_header() {
			return {
					.allocate_order = MemAllocatorOrder::Sequential,
					.mem_type = MemMedia::PMEM
			};
		}

	public:
		void *allocate(size_t size) {
			size = align_size(size);

			while (true) {
				uint8_t *ptr      = cur_bound_.load(std::memory_order::acquire);
				uint8_t *old_ptr  = ptr;
				uint8_t *next_ptr = ptr + size;

				if (next_ptr >= pmem_descriptor_.start_ptr + pmem_descriptor_.total_size) [[unlikely]] {
					// Relocate the space to the start
					ptr = pmem_descriptor_.aligned_start_ptr;
					next_ptr = ptr + size;
				}

				// Try to allocate a new space
				if (cur_bound_.compare_exchange_strong(old_ptr, next_ptr)) {
					return ptr;
				}
			}
		}

		void deallocate(void *ptr, [[maybe_unused]] size_t size) {}

		std::span<uint8_t> get_space_range() const {
			return { pmem_descriptor_.aligned_start_ptr, pmem_descriptor_.aligned_total_size };
		}

	private:

		static size_t align_size(size_t size) {
			constexpr size_t ALIGNED_SIZE = CACHE_LINE_SIZE;
			return (size + ALIGNED_SIZE - 1) & (~(ALIGNED_SIZE - 1));
		}

	};
}

#endif //PTM_MEM_ALLOCATOR_RING_ROUND_ALLOCATOR_PMEM_ALLOCATOR_H
