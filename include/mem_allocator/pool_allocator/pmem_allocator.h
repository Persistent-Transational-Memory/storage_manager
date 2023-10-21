/*
 * @author: BL-GS 
 * @date:   2023/1/9
 */

#pragma once
#ifndef PTM_MEM_ALLOCATOR_POOL_ALLOCATOR_ALLOC_SIMPLE_PMEM_ALLOCATOR_H
#define PTM_MEM_ALLOCATOR_POOL_ALLOCATOR_ALLOC_SIMPLE_PMEM_ALLOCATOR_H

#include <cstdint>
#include <iostream>
#include <fstream>
#include <stack>
#include <filesystem>
#include <cassert>

#include <tbb/concurrent_queue.h>

#include <util/utility_macro.h>
#include <thread/thread.h>
#include <logger/logger.h>
#include <mem_allocator/abstract_mem_allocator.h>

namespace allocator {
	class SimplePmemAllocator {
	public:
		static constexpr size_t ALLOC_ALIGN_SIZE = CACHE_LINE_SIZE;

		inline static const char *PMEM_ROOT_DIR_NAME = GLOBAL_DATA_MEM_DIR_PATH[0];

		static constexpr size_t BLOCK_ALIGN_SIZE = CACHE_LINE_SIZE;

		static constexpr size_t THREAD_LOCAL_BUFFER_NUM = 32;

	private:
		tbb::concurrent_queue<void *> blocks_array;

		size_t alloc_size_;

		FileDescriptor pmem_descriptor_;

		std::stack<void *> thread_local_storage_[thread::get_max_tid()];

	public:
		SimplePmemAllocator(size_t tuple_size, size_t expected_amount):
			alloc_size_(align_block_size(tuple_size)),
			pmem_descriptor_(PMEM_ROOT_DIR_NAME,
			                 allocate_file_name(),
							 std::max(alloc_size_ * expected_amount * 2, 128UL * 1024 * 1024)) {

			for (uint8_t *cur_ptr = pmem_descriptor_.aligned_start_ptr;
						cur_ptr < pmem_descriptor_.start_ptr + pmem_descriptor_.total_size; cur_ptr += alloc_size_) {
				blocks_array.push(cur_ptr);
			}

			auto &global_logger = util::get_global_logger();
			global_logger.info("PMEM File Path: ", pmem_descriptor_.file_path);
			global_logger.info("PMEM Map Size: ", pmem_descriptor_.total_size);
			global_logger.info("Aligned Size of Data Block: ", alloc_size_);
			global_logger.info("Amount of Data Blocks: ", blocks_array.unsafe_size());
			global_logger.info("Address range: ",
			                   static_cast<void *>(pmem_descriptor_.aligned_start_ptr), '-',
							   static_cast<void *>(pmem_descriptor_.aligned_start_ptr + pmem_descriptor_.aligned_total_size));
		}

		~SimplePmemAllocator() = default;

		constexpr static MemAllocatorControlHeader get_header() {
			return {
					.allocate_order = MemAllocatorOrder::Random,
					.mem_type       = MemMedia::PMEM
			};
		}

	public:
		void *allocate(size_t size) {
			uint32_t tid = thread::get_tid();
			void *res = nullptr;
			if (thread_local_storage_[tid].empty()) {
				while (!blocks_array.try_pop(res)) {
					if (blocks_array.unsafe_size() == 0) [[unlikely]] {
						util::logger_error(__FILE__, __LINE__, ": Running out of memory");
						exit(-1);
					}
				}
				DEBUG_ASSERT(res != nullptr);
			}
			else {
				res = thread_local_storage_[tid].top();
				thread_local_storage_[tid].pop();
				DEBUG_ASSERT(res != nullptr);
			}

			return res;
		}
		
		void deallocate(void *ptr, [[maybe_unused]] size_t size) {
			if (thread::is_registered()) [[likely]] {
				uint32_t tid = thread::get_tid();
				if (thread_local_storage_[tid].size() >= THREAD_LOCAL_BUFFER_NUM) {
					blocks_array.push(ptr);
				}
				else {
					thread_local_storage_[tid].push(ptr);
				}
			}
			else {
				blocks_array.push(ptr);
			}
		}

	private:
		static size_t align_block_size(size_t origin_size) {
			return (origin_size + (BLOCK_ALIGN_SIZE - 1)) & (~(BLOCK_ALIGN_SIZE - 1));
		}
	};
}

#endif //PTM_MEM_ALLOCATOR_POOL_ALLOCATOR_ALLOC_SIMPLE_PMEM_ALLOCATOR_H
