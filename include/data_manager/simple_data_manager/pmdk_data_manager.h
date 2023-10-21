/*
 * @author: BL-GS 
 * @date:   2023/2/26
 */

#pragma once
#ifndef PTM_DATA_MANAGER_SIMPLE_DATA_MANAGER_PMDK_DATA_MANAGER_H
#define PTM_DATA_MANAGER_SIMPLE_DATA_MANAGER_PMDK_DATA_MANAGER_H

#include <mem_allocator/mem_allocator.h>

#include <data_manager/abstract_data_manager.h>
#include <data_manager/simple_data_manager/simple_data_manager.h>

namespace datam {

	template<class Key, class TupleHeader, class IndexTuple, IndexType IndexTp>
	struct PMDKDataManagerBasic {

		using DataAllocator = allocator::PMDKAllocator;

		using DataIndex = ix::IndexManager<IndexTp, Key, IndexTuple>::Index;
	};

	template<class Key, class TupleHeader, class IndexTuple, IndexType IndexTp>
	class PMDKDataManager : public SimpleDataManagerTemplate<
			Key, TupleHeader,
			IndexTuple,
			typename PMDKDataManagerBasic<Key, TupleHeader, IndexTuple, IndexTp>::DataIndex,
			typename PMDKDataManagerBasic<Key, TupleHeader, IndexTuple, IndexTp>::DataAllocator
	>  {
	public:
		using BaseType = SimpleDataManagerTemplate<
				Key, TupleHeader,
				IndexTuple,
				typename PMDKDataManagerBasic<Key, TupleHeader, IndexTuple, IndexTp>::DataIndex,
				typename PMDKDataManagerBasic<Key, TupleHeader, IndexTuple, IndexTp>::DataAllocator
		>;

		using DataKeyType         = Key;
		using DataTupleHeaderType = TupleHeader;
		using IndexTupleType      = IndexTuple;

		static constexpr StorageOrder DATA_STORAGE_ORDER = StorageOrder::Random;

		static constexpr StorageOrder get_data_storage_order() { return DATA_STORAGE_ORDER; };

		static constexpr StorageMemType DATA_STORAGE_MEM_TYPE = StorageMemType::PMEM;

		static constexpr StorageMemType get_data_storage_memtype() { return DATA_STORAGE_MEM_TYPE; };

		using DataAllocator = allocator::PMDKAllocator;
		static_assert(allocator::MemAllocatorConcept<DataAllocator>);

		static constexpr StorageControlHeader get_data_storage_control_header() { return DataAllocator::get_header(); }

		/*
		 * Index Configuration
		 */

		using DataIndexNodeType = ix::IndexManager<IndexTp, DataKeyType, IndexTupleType>::NodeType;

		using DataIndex         = ix::IndexManager<IndexTp, DataKeyType, IndexTupleType>::Index;

		static_assert(ix::IndexConcept<DataIndex>);
	};

}

#endif //PTM_DATA_MANAGER_SIMPLE_DATA_MANAGER_PMDK_DATA_MANAGER_H
