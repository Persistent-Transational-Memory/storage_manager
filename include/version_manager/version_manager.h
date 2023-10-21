/*
 * @author: BL-GS 
 * @date:   2023/2/26
 */

#pragma once
#ifndef PTM_VERSION_MANAGER_H
#define PTM_VERSION_MANAGER_H

#include <version_manager/abstract_version_manager.h>

#include <version_manager/simple_version_manager/dram_version_manager.h>
#include <version_manager/simple_version_manager/pmem_version_manager.h>
#include <version_manager/simple_version_manager/pmdk_version_manager.h>

namespace versionm {

	enum class VersionManagerKind {
		DRAM,
		PMEM,
		PMEM_NUMA,
		PMDK
	};

	template<VersionManagerKind Type, class VersionHeader>
	struct VersionManagerManager {
		using VersionManager = void;
	};

	template<class VersionHeader>
	struct VersionManagerManager<VersionManagerKind::DRAM, VersionHeader> {
		using VersionManager = DRAMVersionManager<VersionHeader>;

		static_assert(VersionManagerConcept<VersionManager>);
	};

	template<class VersionHeader>
	struct VersionManagerManager<VersionManagerKind::PMEM, VersionHeader> {
		using VersionManager = PMEMVersionManager<VersionHeader>;

		static_assert(VersionManagerConcept<VersionManager>);
	};

	template<class VersionHeader>
	struct VersionManagerManager<VersionManagerKind::PMEM_NUMA, VersionHeader> {
		using VersionManager = PMEMNUMAVersionManager<VersionHeader>;

		static_assert(VersionManagerConcept<VersionManager>);
	};

	template<class VersionHeader>
	struct VersionManagerManager<VersionManagerKind::PMDK, VersionHeader> {
		using VersionManager = PMDKVersionManager<VersionHeader>;

		static_assert(VersionManagerConcept<VersionManager>);
	};
}

#endif //PTM_VERSION_MANAGER_H
