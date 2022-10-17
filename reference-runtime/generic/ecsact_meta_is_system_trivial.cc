#include "ecsact/runtime/meta.h"

#include <vector>
#include "ecsact/runtime/meta.hh"

bool ecsact_meta_is_system_trivial
	( ecsact_system_like_id sys_id
	)
{
	auto caps = ecsact::meta::system_capabilities(sys_id);
	for(auto&& [comp_id, cap] : caps) {
		const bool is_tag_comp = ecsact::meta::count_fields(comp_id) == 0;
		if(is_tag_comp) {
			continue;
		}
		
		if((cap & ECSACT_SYS_CAP_WRITEONLY) == ECSACT_SYS_CAP_WRITEONLY) {
			return false;
		} else if((cap & ECSACT_SYS_CAP_ADDS) == ECSACT_SYS_CAP_ADDS) {
			return false;
		}
		
		auto assoc_fields = ecsact::meta::system_association_fields(
			sys_id,
			comp_id
		);

		for(auto assoc_field_id : assoc_fields) {

			auto assoc_caps = ecsact::meta::system_association_capabilities(
				sys_id,
				comp_id,
				assoc_field_id
			);
		}
	}

	// TODO

	return true;
}
