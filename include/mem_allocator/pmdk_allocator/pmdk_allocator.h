/*
 * @author: BL-GS 
 * @date:   2023/2/22
 */

#pragma once
#ifndef MEM_ALLOC_PMDK_ALLOCATOR_H
#define MEM_ALLOC_PMDK_ALLOCATOR_H

#include <cstdint>
#include <cassert>
#include <filesystem>

#include <libpmemobj.h>

#include <logger/logger.h>
#include <mem_allocator/abstract_mem_allocator.h>

extern "C" {
	TOID_DECLARE(uint8_t, 0);
}

namespace allocator {

	class PMDKAllocator {
	public:
		static constexpr size_t ALLOC_ALIGN_SIZE = 0;

		inline static const char *PMEM_LAYOUT = "PMDKAllocatorLayout";

		inline static const char *PMEM_ROOT_DIR_NAME = GLOBAL_DATA_MEM_DIR_PATH[0];

	private:
		pmemobjpool *pool_;

		std::string alloc_file;

	public:
		PMDKAllocator(size_t tuple_size, size_t expected_amount):
			pool_(nullptr),
			alloc_file(std::string(PMEM_ROOT_DIR_NAME) + allocate_file_name()) {

			uint32_t total_size = std::max(tuple_size * expected_amount * 8, 1024UL * 1024 * 1024);

			if (std::filesystem::exists(alloc_file)) {
				pool_ = pmemobj_open(alloc_file.c_str(), PMEM_LAYOUT);
			}
			else {
				pool_ = pmemobj_create(alloc_file.c_str(), PMEM_LAYOUT, total_size, 0666);
			}
			if (pool_ == nullptr) {
				util::get_global_logger().error("PMDK fail creating pool.");
				assert(false);
			}
		}

		PMDKAllocator(std::string_view dir_name, std::string_view file_name, size_t tuple_size, size_t expected_amount):
				pool_(nullptr),
				alloc_file(std::string(dir_name) + file_name.data()) {

			uint32_t total_size = std::max(tuple_size * expected_amount * 8, 1024UL * 1024 * 1024);

			if (std::filesystem::exists(alloc_file)) {
				pool_ = pmemobj_open(alloc_file.c_str(), PMEM_LAYOUT);
			}
			else {
				pool_ = pmemobj_create(alloc_file.c_str(), PMEM_LAYOUT, total_size, 0666);
			}
			if (pool_ == nullptr) {
				util::get_global_logger().error("PMDK fail creating pool.");
				assert(false);
			}
		}

		~PMDKAllocator() {
			pmemobj_close(pool_);
		}

		constexpr static MemAllocatorControlHeader get_header() {
			return {
					.allocate_order = MemAllocatorOrder::Random,
					.mem_type = MemMedia::PMEM
			};
		}

	public:
		void *allocate(size_t size) {
			TOID(uint8_t) target_oid;
			pmemobj_alloc(pool_, (PMEMoid *)&target_oid, size, 0, NULL, NULL);
			if (TOID_IS_NULL(target_oid)) { assert(false); }
			return pmemobj_direct(target_oid.oid);
		}

		void deallocate(void *ptr, [[maybe_unused]]size_t size) {
			auto oid = pmemobj_oid(ptr);
			pmemobj_free(&oid);
		}
	};

}
#endif //MEM_ALLOC_PMDK_ALLOCATOR_H
