/*
 * @author: BL-GS 
 * @date:   2023/2/26
 */

#pragma once
#ifndef DATA_DATA_MANAGER_H
#define DATA_DATA_MANAGER_H

#include <data_manager/abstract_data_manager.h>

#include <data_manager/simple_data_manager/dram_data_manager.h>
#include <data_manager/simple_data_manager/pmdk_data_manager.h>
#include <data_manager/simple_data_manager/pmem_data_manager.h>

namespace datam {

	enum class DataManagerKind {
		DRAM,
		PMEM,
		PMEM_NUMA,
		PMDK,
	};

	template<DataManagerKind Type, class Key, class TupleHeader, class IndexTuple, IndexType IndexTp>
	struct DataManagerManager {
		using DataManager = void;
	};

	template<class Key, class TupleHeader, class IndexTuple, IndexType IndexTp>
	struct DataManagerManager<DataManagerKind::DRAM, Key, TupleHeader, IndexTuple, IndexTp> {
		using DataManager = DRAMDataManager<Key, TupleHeader, IndexTuple, IndexTp>;

		static_assert(DataManagerConcept<DataManager>);
	};

	template<class Key, class TupleHeader, class IndexTuple, IndexType IndexTp>
	struct DataManagerManager<DataManagerKind::PMEM, Key, TupleHeader, IndexTuple, IndexTp> {
		using DataManager = PMEMDataManager<Key, TupleHeader, IndexTuple, IndexTp>;

		static_assert(DataManagerConcept<DataManager>);
	};

	template<class Key, class TupleHeader, class IndexTuple, IndexType IndexTp>
	struct DataManagerManager<DataManagerKind::PMEM_NUMA, Key, TupleHeader, IndexTuple, IndexTp> {
		using DataManager = PMEMNUMADataManager<Key, TupleHeader, IndexTuple, IndexTp>;

		static_assert(DataManagerConcept<DataManager>);
	};

	template<class Key, class TupleHeader, class IndexTuple, IndexType IndexTp>
	struct DataManagerManager<DataManagerKind::PMDK, Key, TupleHeader, IndexTuple, IndexTp> {
		using DataManager = PMDKDataManager<Key, TupleHeader, IndexTuple, IndexTp>;

		static_assert(DataManagerConcept<DataManager>);
	};

}

#endif //DATA_DATA_MANAGER_H
