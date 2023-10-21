/*
 * @author: BL-GS 
 * @date:   2023/1/9
 */

#pragma once
#ifndef PTM_MEM_ALLOCATOR_POOL_ALLOCATOR_NUMALOCAL_PMEM_ALLOCATOR_H
#define PTM_MEM_ALLOCATOR_POOL_ALLOCATOR_NUMALOCAL_PMEM_ALLOCATOR_H

#include <cstdint>
#include <iostream>
#include <fstream>
#include <forward_list>
#include <filesystem>
#include <cassert>
#include <queue>

#include <sys/unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <tbb/concurrent_queue.h>

#include <thread/thread.h>
#include <mem_allocator/abstract_mem_allocator.h>

namespace allocator {
	class SimpleNUMALocalPmemAllocator {
	public:
		static constexpr size_t ALLOC_ALIGN_SIZE = CACHE_LINE_SIZE;

		inline static auto PMEM_ROOT_DIR_NAME = GLOBAL_DATA_MEM_DIR_PATH;

	private:
		uint32_t num_numa_node_;

		struct NUMAMem {
			FileDescriptor file_descriptor_;
			tbb::concurrent_queue<void *> blocks_array;

			NUMAMem(std::string_view dir_name, std::string_view path, size_t alloc_size):
					file_descriptor_(dir_name, path, alloc_size) {}
		};
		std::vector<NUMAMem> numa_mem_;

		size_t alloc_size_;

		size_t total_size_;

	public:
		SimpleNUMALocalPmemAllocator(size_t tuple_size, size_t expected_amount):
			num_numa_node_(0), alloc_size_(tuple_size),
			total_size_(std::max(tuple_size * expected_amount * 8, 1024UL * 1024 * 1024)) {

			num_numa_node_ = thread::get_num_nodes();
			numa_mem_.reserve(num_numa_node_);

			if (num_numa_node_ > 1) { total_size_ *= num_numa_node_; }

			for (uint32_t i = 0; i < num_numa_node_; ++i) {
				numa_mem_.emplace_back(PMEM_ROOT_DIR_NAME[i],
									   allocate_file_name(),
									   total_size_);

				uint32_t align       = CACHE_LINE_SIZE;
				uint32_t align_size  = (alloc_size_ + align - 1) / align * align;
				uint8_t *aligned_ptr = numa_mem_[i].file_descriptor_.aligned_start_ptr;

				uint64_t total_amount = total_size_ / align_size;
				uint8_t *start_ptr = aligned_ptr;
				uint8_t *end_ptr = start_ptr + total_amount * align_size;

				for (uint8_t *cur_ptr = start_ptr; cur_ptr < end_ptr; cur_ptr += align_size) {
					numa_mem_[i].blocks_array.push(cur_ptr);
				}

				auto &global_logger = util::get_global_logger();
				global_logger.info("Aligned Size of Data Block: ", alloc_size_);
				global_logger.info("Amount of Data Block: ", total_amount);
			}

		}

		~SimpleNUMALocalPmemAllocator() = default;

		constexpr static MemAllocatorControlHeader get_header() {
			return {
					.allocate_order = MemAllocatorOrder::Random,
					.mem_type = MemMedia::PMEM
			};
		}

	public:
		void *allocate(size_t size) {
			assert(size == alloc_size_);
			uint32_t numa_id = thread::get_cpu_numa_id();

			void *res;
			while (!numa_mem_[numa_id].blocks_array.try_pop(res));
			return res;
		}
		
		void deallocate(void *ptr, [[maybe_unused]] size_t size) {
			assert(size == alloc_size_);
			uint32_t numa_id = thread::get_cpu_numa_id();
			numa_mem_[numa_id].blocks_array.push(ptr);
		}
	};
}

#endif //PTM_MEM_ALLOCATOR_POOL_ALLOCATOR_NUMALOCAL_PMEM_ALLOCATOR_H
