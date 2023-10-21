/*
 * @author: BL-GS 
 * @date:   2023/1/24
 */

#pragma once
#ifndef PTM_STORAGE_MANAGER_SIMPLE_STORAGE_MANAGER_PMDK_PMEM_DATA_DRAM_LOG_MANGER_H
#define PTM_STORAGE_MANAGER_SIMPLE_STORAGE_MANAGER_PMDK_PMEM_DATA_DRAM_LOG_MANGER_H

#include <data_manager/data_manager.h>
#include <log_manager/log_manager.h>
#include <version_manager/version_manager.h>

#include <storage_manager/simple_storage_manager/simple_storage_manager.h>

namespace storage {

	template<typename DataKey,
			typename TupleHeader,
			typename IndexTuple,
			IndexType DataIndexTp,
			typename VersionHeader>
	requires DataKeyTypeConcept<DataKey>
	class PDDLPMDKManager:
			public SimpleMVStorageManagerTemplate<
					typename datam::DataManagerManager<datam::DataManagerKind::PMDK, DataKey, TupleHeader, IndexTuple, DataIndexTp>::DataManager ,
					logm::LogManagerManager<logm::LogManagerKind::DRAM_TL>::LogManager,
					typename versionm::VersionManagerManager<versionm::VersionManagerKind::PMDK, VersionHeader>::VersionManager
			> {

	public:
		using Self             = PDDLPMDKManager<DataKey, TupleHeader, IndexTuple, DataIndexTp, VersionHeader>;
		using DataManager      = datam::DataManagerManager<datam::DataManagerKind::PMDK, DataKey, TupleHeader, IndexTuple, DataIndexTp>::DataManager;
		using LogManager       = logm::LogManagerManager<logm::LogManagerKind::DRAM_TL>::LogManager;
		using VersionManager   = versionm::VersionManagerManager<versionm::VersionManagerKind::PMEM_NUMA, VersionHeader>::VersionManager;
		using BaseManager      = SimpleMVStorageManagerTemplate<DataManager, LogManager, VersionManager>;

		/*
		 * Data manager parameters
		 */
		using DataKeyType      = DataManager::DataKeyType;
		using DataTupleHeaderType = DataManager::DataTupleHeaderType;
		using IndexTupleType   = DataManager::IndexTupleType;

		/*
		 * Align Assumed
		 */
		static constexpr size_t DATA_ALLOC_ALIGN_SIZE    = DataManager::DATA_ALLOC_ALIGN_SIZE;
		static constexpr size_t VHEADER_ALLOC_ALIGN_SIZE = DataManager::VHEADER_ALLOC_ALIGN_SIZE;
		static constexpr size_t LOG_ALLOC_ALIGN_SIZE     = LogManager::LOG_ALLOC_ALIGN_SIZE;
	};

	template<typename DataKey,
			typename TupleHeader,
			typename IndexTuple,
			IndexType DataIndexTp>
	requires DataKeyTypeConcept<DataKey>
	class PDDLPMDKManager<DataKey, TupleHeader, IndexTuple, DataIndexTp, void>:
			public SimpleStorageManagerTemplate<
					typename datam::DataManagerManager<datam::DataManagerKind::PMDK, DataKey, TupleHeader, IndexTuple, DataIndexTp>::DataManager ,
					typename logm::LogManagerManager<logm::LogManagerKind::DRAM_TL>::LogManager
			> {

	public:
		using Self             = PDDLPMDKManager<DataKey, TupleHeader, IndexTuple, DataIndexTp, void>;
		using DataManager      = datam::DataManagerManager<datam::DataManagerKind::PMDK, DataKey, TupleHeader, IndexTuple, DataIndexTp>::DataManager;
		using LogManager       = logm::LogManagerManager<logm::LogManagerKind::DRAM_TL>::LogManager;
		using BaseManager      = SimpleStorageManagerTemplate<DataManager, LogManager>;

		/*
		 * Data manager parameters
		 */
		using DataKeyType      = DataManager::DataKeyType;
		using DataTupleHeaderType = DataManager::DataTupleHeaderType;
		using IndexTupleType   = DataManager::IndexTupleType;

		/*
		 * Align Assumed
		 */
		static constexpr size_t DATA_ALLOC_ALIGN_SIZE    = DataManager::DATA_ALLOC_ALIGN_SIZE;
		static constexpr size_t VHEADER_ALLOC_ALIGN_SIZE = DataManager::VHEADER_ALLOC_ALIGN_SIZE;
		static constexpr size_t LOG_ALLOC_ALIGN_SIZE     = LogManager::LOG_ALLOC_ALIGN_SIZE;
	};

}

#endif //PTM_STORAGE_MANAGER_SIMPLE_STORAGE_MANAGER_PMDK_PMEM_DATA_DRAM_LOG_MANGER_H
